#include <flockmtl/common.hpp>
#include <flockmtl/core/model_manager/model_manager.hpp>
#include <flockmtl/core/model_manager/openai.hpp>
#include <flockmtl/core/model_manager/azure.hpp>
#include <flockmtl_extension.hpp>
#include <memory>
#include <string>
#include <stdexcept>


namespace flockmtl {
namespace core {

static const std::unordered_set<std::string> supported_models = {"gpt-4o", "gpt-4o-mini"};
static const std::unordered_set<std::string> supported_providers = {"openai", "azure"};
static const std::unordered_set<std::string> supported_embedding_models = {"text-embedding-3-small",
                                                                           "text-embedding-3-large"};

ModelDetails ModelManager::CreateModelDetails(Connection &con, const nlohmann::json &model_json) {
    ModelDetails model_details;

    model_details.provider_name = model_json.contains("provider") ? model_json.at("provider").get<std::string>() : "";
    model_details.model_name = model_json.contains("model_name") ? model_json.at("model_name").get<std::string>() : "";
    auto query_result = GetQueriedModel(con, model_details.model_name, model_details.provider_name);
    model_details.model = query_result.first;
    model_details.max_tokens = query_result.second;

    auto max_tokens = 4000;
    auto temperature = 0.5;

    for (auto &[key, value] : model_json.items()) {
        if (key == "max_tokens") {
            max_tokens = std::stoi(static_cast<std::string>(value));
        } else if (key == "temperature") {
            temperature = std::stof(static_cast<std::string>(value));
        } else if (key == "provider" || key == "model_name") {
            continue;
        } else {
            throw std::invalid_argument("Invalid setting key: " + key);
        }
    }

    model_details.max_tokens = max_tokens;
    model_details.temperature = temperature;

    return model_details;
}

std::pair<std::string, int32_t> ModelManager::GetQueriedModel(Connection &con, const std::string &model_name,
                                                              const std::string &provider_name) {
    std::string query = "SELECT model, max_tokens FROM flockmtl_config.FLOCKMTL_MODEL_USER_DEFINED_INTERNAL_TABLE "
                        "WHERE model_name = '" +
                        model_name + "'";
    if (!provider_name.empty()) {
        query += " AND provider_name = '" + provider_name + "'";
    }

    auto query_result = con.Query(query);

    if (query_result->RowCount() == 0) {
        query_result = con.Query(
            "SELECT model, max_tokens FROM flockmtl_config.FLOCKMTL_MODEL_DEFAULT_INTERNAL_TABLE WHERE model_name = '" +
            model_name + "'");

        if (query_result->RowCount() == 0) {
            throw std::runtime_error("Model not found");
        }
    }

    return {query_result->GetValue(0, 0).ToString(), query_result->GetValue(1, 0).GetValue<int32_t>()};
}

nlohmann::json ModelManager::OpenAICallComplete(const std::string &prompt, const ModelDetails &model_details,
                                                const bool json_response) {

    // Get API key from the environment variable
    auto key = openai::OpenAI::get_openai_api_key();
    openai::start(key);

    // Create a JSON request payload with the provided parameters
    nlohmann::json request_payload = {{"model", model_details.model},
                                      {"messages", {{{"role", "user"}, {"content", prompt}}}},
                                      {"max_tokens", model_details.max_tokens},
                                      {"temperature", model_details.temperature}};

    // Conditionally add "response_format" if json_response is true
    if (json_response) {
        request_payload["response_format"] = {{"type", "json_object"}};
    }

    // Make a request to the OpenAI API
    nlohmann::json completion;
    try {
        completion = openai::chat().create(request_payload);
    } catch (const std::exception &e) {
        throw std::runtime_error("Error in making request to OpenAI API: " + std::string(e.what()));
    }
    // Check if the conversation was too long for the context window
    if (completion["choices"][0]["finish_reason"] == "length") {
        // Handle the error when the context window is too long
        throw std::runtime_error(
            "The response exceeded the context window length you can increase your max_tokens parameter.");
        // Add error handling code here
    }

    // Check if the OpenAI safety system refused the request
    if (completion["choices"][0]["message"]["refusal"] != nullptr) {
        // Handle refusal error
        throw std::runtime_error("The request was refused due to OpenAI's safety system.{\"refusal\": \"" +
                                 completion["choices"][0]["message"]["refusal"].get<std::string>() + "\"}");
    }

    // Check if the model's output included restricted content
    if (completion["choices"][0]["finish_reason"] == "content_filter") {
        // Handle content filtering
        throw std::runtime_error("The content filter was triggered, resulting in incomplete JSON.");
        // Add error handling code here
    }

    std::string content_str = completion["choices"][0]["message"]["content"];

    if (json_response) {
        return nlohmann::json::parse(content_str);
    }

    return content_str;
}

nlohmann::json ModelManager::AzureCallComplete(const std::string &prompt, const ModelDetails &model_details,
                                               const bool json_response) {
    // Get API key from the environment variable
    auto api_key = AzureModelManager::get_azure_api_key();
    auto resource_name = AzureModelManager::get_azure_resource_name();
    auto api_version = AzureModelManager::get_azure_api_version();

    auto azure_model_manager_uptr =
        std::make_unique<AzureModelManager>(api_key, resource_name, model_details.model, api_version, false);

    // Create a JSON request payload with the provided parameters
    nlohmann::json request_payload = {{"model", model_details.model},
                                      {"messages", {{{"role", "user"}, {"content", prompt}}}},
                                      {"max_tokens", model_details.max_tokens},
                                      {"temperature", model_details.temperature}};

    // Conditionally add "response_format" if json_response is true
    if (json_response) {
        request_payload["response_format"] = {{"type", "json_object"}};
    }

    // Make a request to the Azure API
    auto completion = azure_model_manager_uptr->CallComplete(request_payload);

    // Check if the conversation was too long for the context window
    if (completion["choices"][0]["finish_reason"] == "length") {
        // Handle the error when the context window is too long
        throw std::runtime_error(
            "The response exceeded the context window length you can increase your max_tokens parameter.");
        // Add error handling code here
    }

    // Check if the safety system refused the request
    if (completion["choices"][0]["message"]["refusal"] != nullptr) {
        // Handle refusal error
        throw std::runtime_error("The request was refused due to Azure's safety system.{\"refusal\": \"" +
                                 completion["choices"][0]["message"]["refusal"].get<std::string>() + "\"}");
    }

    // Check if the model's output included restricted content
    if (completion["choices"][0]["finish_reason"] == "content_filter") {
        // Handle content filtering
        throw std::runtime_error("The content filter was triggered, resulting in incomplete JSON.");
        // Add error handling code here
    }

    std::string content_str = completion["choices"][0]["message"]["content"];

    if (json_response) {
        return nlohmann::json::parse(content_str);
    }

    return content_str;
}

nlohmann::json ModelManager::CallComplete(const std::string &prompt, const ModelDetails &model_details,
                                          const bool json_response) {

    // Check if the provided model is in the list of supported models
    if (supported_models.find(model_details.model) == supported_models.end()) {
        throw std::invalid_argument("Model '" + model_details.model +
                                    "' is not supported. Please choose one from the supported list: "
                                    "gpt-4o, gpt-4o-mini.");
    }

    // Check if the provider is in the list of supported provider
    if (!model_details.provider_name.empty() && model_details.provider_name != "default" &&
        supported_providers.find(model_details.provider_name) == supported_providers.end()) {

        throw std::invalid_argument("Provider '" + model_details.provider_name +
                                    "' is not supported. Please choose one from the supported list: "
                                    "openai/default, azure");
    }

    if (model_details.provider_name == "openai" || model_details.provider_name == "default" ||
        model_details.provider_name == "") {
        return OpenAICallComplete(prompt, model_details, json_response);
    } else {
        return AzureCallComplete(prompt, model_details, json_response);
    }
}

nlohmann::json ModelManager::OpenAICallEmbedding(const std::string &input, const ModelDetails &model_details) {
    // Get API key from the environment variable
    auto key = openai::OpenAI::get_openai_api_key();
    openai::start(key);

    // Create a JSON request payload with the provided parameters
    nlohmann::json request_payload = {
        {"model", model_details.model},
        {"input", input},
    };

    // Make a request to the OpenAI API
    auto completion = openai::embedding().create(request_payload);

    // Check if the conversation was too long for the context window
    if (completion["choices"][0]["finish_reason"] == "length") {
        // Handle the error when the context window is too long
        throw std::runtime_error(
            "The response exceeded the context window length you can increase your max_tokens parameter.");
        // Add error handling code here
    }

    auto embedding = completion["data"][0]["embedding"];

    return embedding;
}

nlohmann::json ModelManager::AzureCallEmbedding(const std::string &input, const ModelDetails &model_details) {
    // Get API key from the environment variable
    auto api_key = AzureModelManager::get_azure_api_key();
    auto resource_name = AzureModelManager::get_azure_resource_name();
    auto api_version = AzureModelManager::get_azure_api_version();

    auto azure_model_manager_uptr =
        std::make_unique<AzureModelManager>(api_key, resource_name, model_details.model, api_version, false);

    // Create a JSON request payload with the provided parameters
    nlohmann::json request_payload = {
        {"model", model_details.model},
        {"input", input},
    };

    // Make a request to the Azure API
    auto completion = azure_model_manager_uptr->CallEmbedding(request_payload);

    // Check if the conversation was too long for the context window
    if (completion["choices"][0]["finish_reason"] == "length") {
        // Handle the error when the context window is too long
        throw std::runtime_error(
            "The response exceeded the context window length you can increase your max_tokens parameter.");
        // Add error handling code here
    }

    auto embedding = completion["data"][0]["embedding"];

    return embedding;
}

nlohmann::json ModelManager::CallEmbedding(const std::string &input, const ModelDetails &model_details) {

    // Check if the provided model is in the list of supported models
    if (supported_embedding_models.find(model_details.model) == supported_embedding_models.end()) {
        throw std::invalid_argument("Model '" + model_details.model +
                                    "' is not supported. Please choose one from the supported list: "
                                    "text-embedding-3-small, text-embedding-3-large.");
    }

    // Check if the provider is in the list of supported provider
    if (!model_details.provider_name.empty() && model_details.provider_name != "default" &&
        supported_providers.find(model_details.provider_name) == supported_providers.end()) {

        throw std::invalid_argument("Provider '" + model_details.provider_name +
                                    "' is not supported. Please choose one from the supported list: "
                                    "openai/default, azure");
    }

    if (model_details.provider_name == "openai" || model_details.provider_name == "default" ||
        model_details.provider_name.empty()) {
        return OpenAICallEmbedding(input, model_details);
    } else {
        return AzureCallEmbedding(input, model_details);
    }
}

} // namespace core
} // namespace flockmtl