#include <flockmtl/common.hpp>
#include <flockmtl/core/model_manager/supported_providers.hpp>
#include <flockmtl/core/model_manager/supported_models.hpp>
#include <flockmtl/core/model_manager/supported_embedding_models.hpp>
#include <flockmtl/core/model_manager/model_manager.hpp>
#include <flockmtl/core/model_manager/openai.hpp>
#include <flockmtl/core/model_manager/azure.hpp>
#include <flockmtl/core/model_manager/ollama.hpp>
#include <flockmtl/core/config/config.hpp>
#include <flockmtl_extension.hpp>
#include <memory>
#include <string>
#include <stdexcept>

namespace flockmtl {
namespace core {

ModelDetails ModelManager::CreateModelDetails(Connection &con, const nlohmann::json &model_json) {
    ModelDetails model_details;

    model_details.provider_name = model_json.contains("provider") ? model_json.at("provider").get<std::string>() : "";
    model_details.model_name = model_json.contains("model_name") ? model_json.at("model_name").get<std::string>() : "";
    auto query_result = GetQueriedModel(con, model_details.model_name, model_details.provider_name);
    model_details.model = std::get<0>(query_result);
    model_details.context_window = std::get<1>(query_result);
    model_details.max_output_tokens = std::get<2>(query_result);
    model_details.temperature = 0.5;

    for (auto &[key, value] : model_json.items()) {
        if (key == "max_output_tokens") {
            model_details.max_output_tokens = std::stoi(static_cast<std::string>(value));
        } else if (key == "temperature") {
            model_details.temperature = std::stof(static_cast<std::string>(value));
        } else if (key == "provider" || key == "model_name") {
            continue;
        } else {
            throw std::invalid_argument("Invalid setting key: " + key);
        }
    }

    return model_details;
}

std::tuple<std::string, int32_t, int32_t> ModelManager::GetQueriedModel(Connection &con, const std::string &model_name,
                                                                        const std::string &provider_name) {

    auto provider_name_lower = provider_name;
    std::transform(provider_name_lower.begin(), provider_name_lower.end(), provider_name_lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (provider_name_lower == OLLAMA) {
        OllamaModelManager olam(false);
        if (!olam.validModel(model_name)) {
            throw std::runtime_error("Specified ollama model not deployed, please deploy before using it");
        }
        return {model_name, Config::default_context_window, Config::default_max_output_tokens};
    }

    std::string query = "SELECT model, context_window, max_output_tokens FROM "
                        "flockmtl_config.FLOCKMTL_MODEL_USER_DEFINED_INTERNAL_TABLE "
                        "WHERE model_name = '" +
                        model_name + "'";
    if (!provider_name.empty()) {
        query += " AND provider_name = '" + provider_name + "'";
    }

    auto query_result = con.Query(query);

    if (query_result->RowCount() == 0) {
        query_result = con.Query("SELECT model, context_window, max_output_tokens FROM "
                                 "flockmtl_config.FLOCKMTL_MODEL_DEFAULT_INTERNAL_TABLE WHERE model_name = '" +
                                 model_name + "'");

        if (query_result->RowCount() == 0) {
            throw std::runtime_error("Model not found");
        }
    }

    return {query_result->GetValue(0, 0).ToString(), query_result->GetValue(1, 0).GetValue<int32_t>(),
            query_result->GetValue(2, 0).GetValue<int32_t>()};
}

nlohmann::json ModelManager::OllamaCallComplete(const std::string &prompt, const ModelDetails &model_details,
                                                const bool json_response) {
    auto ollama_model_manager_uptr = std::make_unique<OllamaModelManager>(false);

    // Create a JSON request payload with the provided parameters
    nlohmann::json request_payload = {{"model", model_details.model},
                                      {"messages", {{{"role", "user"}, {"content", prompt}}}},
                                      {"stream", false},
                                      {"options",
                                       {
                                           {"temperature", model_details.temperature},
                                           {"num_ctx", model_details.max_output_tokens},
                                       }},
                                      {"keep_alive", -1}};

    // Conditionally add "response_format" if json_response is true
    if (json_response) {
        request_payload["format"] = "json";
    }

    nlohmann::json completion;
    try {
        completion = ollama_model_manager_uptr->CallComplete(request_payload);
    } catch (const std::exception &e) {
        throw std::runtime_error("Error in making request to Ollama API: " + std::string(e.what()));
    }

    // Check if the call was not succesfull
    if ((completion.contains("done_reason") && completion["done_reason"] != "stop") ||
        (completion.contains("done") && !completion["done"].is_null() && completion["done"].get<bool>() != true)) {
        // Handle refusal error
        throw std::runtime_error("The request was refused due to some internal error with Ollama API");
    }

    std::string content_str = completion["message"]["content"];

    if (json_response) {
        return nlohmann::json::parse(content_str);
    }

    return content_str;
}

nlohmann::json ModelManager::AwsBedrockCallComplete(const std::string &prompt, const ModelDetails &model_details,
                                                    const bool json_response) {
    return nlohmann::json();
}

nlohmann::json ModelManager::OpenAICallComplete(const std::string &prompt, const ModelDetails &model_details,
                                                const bool json_response) {

    // Get API key from the environment variable
    auto key = openai::OpenAI::get_openai_api_key();
    openai::start(key);

    // Create a JSON request payload with the provided parameters
    nlohmann::json request_payload = {{"model", model_details.model},
                                      {"messages", {{{"role", "user"}, {"content", prompt}}}},
                                      {"max_tokens", model_details.max_output_tokens},
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
            "The response exceeded the context window length you can increase your max_output_tokens parameter.");
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
                                      {"max_tokens", model_details.max_output_tokens},
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
            "The response exceeded the context window length you can increase your max_output_tokens parameter.");
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
    }

    std::string content_str = completion["choices"][0]["message"]["content"];

    if (json_response) {
        return nlohmann::json::parse(content_str);
    }

    return content_str;
}

nlohmann::json ModelManager::CallComplete(const std::string &prompt, const ModelDetails &model_details,
                                          const bool json_response) {

    // Check if the provider is in the list of supported provider
    auto provider = GetProviderType(model_details.provider_name);
    if (provider == FLOCKMTL_UNSUPPORTED_PROVIDER) {

        throw std::runtime_error("Provider '" + model_details.provider_name +
                                 "' is not supported. Please choose one from the supported list: "
                                 "openai/default, azure, ollama, aws bedrock");
    }

    // Check if the provided model is in the list of supported models
    auto model = GetModelType(model_details.model, model_details.provider_name);
    if (model == FLOCKMTL_UNSUPPORTED_MODEL) {

        throw std::runtime_error("Model '" + model_details.model +
                                 "' is not supported. Please choose one from the supported list: "
                                 "gpt-4o, gpt-4o-mini, ollama-models (only with ollama as provider).");
    }

    return CallCompleteProvider(prompt, model_details, json_response).second;
}

nlohmann::json ModelManager::OllamaCallEmbedding(const std::vector<string> &inputs, const ModelDetails &model_details) {
    auto ollama_model_manager_uptr = std::make_unique<OllamaModelManager>(false);

    // Create a JSON request payload with the provided parameters
    nlohmann::json request_payload = {
        {"model", model_details.model},
        {"input", inputs},
    };

    nlohmann::json completion;
    try {
        completion = ollama_model_manager_uptr->CallEmbedding(request_payload);
    } catch (const std::exception &e) {
        throw std::runtime_error("Error in making request to Ollama API: " + std::string(e.what()));
    }

    auto embeddings = nlohmann::json::array();
    for (auto &item : completion["embeddings"]) {
        embeddings.push_back(item);
    }

    return embeddings;
}

nlohmann::json ModelManager::AwsBedrockCallEmbedding(const std::vector<string> &inputs,
                                                     const ModelDetails &model_details) {
    return nlohmann::json();
}

nlohmann::json ModelManager::OpenAICallEmbedding(const std::vector<string> &inputs, const ModelDetails &model_details) {
    // Get API key from the environment variable
    auto key = openai::OpenAI::get_openai_api_key();
    openai::start(key);

    // Create a JSON request payload with the provided parameters
    nlohmann::json request_payload = {
        {"model", model_details.model},
        {"input", inputs},
    };

    // Make a request to the OpenAI API
    auto completion = openai::embedding().create(request_payload);

    // Check if the conversation was too long for the context window
    if (completion["choices"][0]["finish_reason"] == "length") {
        // Handle the error when the context window is too long
        throw std::runtime_error(
            "The response exceeded the context window length you can increase your max_output_tokens parameter.");
    }

    auto embeddings = nlohmann::json::array();
    for (auto &item : completion["data"]) {
        embeddings.push_back(item["embedding"]);
    }

    return embeddings;
}

nlohmann::json ModelManager::AzureCallEmbedding(const std::vector<string> &inputs, const ModelDetails &model_details) {
    // Get API key from the environment variable
    auto api_key = AzureModelManager::get_azure_api_key();
    auto resource_name = AzureModelManager::get_azure_resource_name();
    auto api_version = AzureModelManager::get_azure_api_version();

    auto azure_model_manager_uptr =
        std::make_unique<AzureModelManager>(api_key, resource_name, model_details.model, api_version, false);

    // Create a JSON request payload with the provided parameters
    nlohmann::json request_payload = {
        {"model", model_details.model},
        {"input", inputs},
    };

    // Make a request to the Azure API
    auto completion = azure_model_manager_uptr->CallEmbedding(request_payload);

    // Check if the conversation was too long for the context window
    if (completion["choices"][0]["finish_reason"] == "length") {
        // Handle the error when the context window is too long
        throw std::runtime_error(
            "The response exceeded the context window length you can increase your max_output_tokens parameter.");
    }

    auto embeddings = nlohmann::json::array();
    for (auto &item : completion["data"]) {
        embeddings.push_back(item["embedding"]);
    }

    return embeddings;
}

nlohmann::json ModelManager::CallEmbedding(const std::vector<string> &inputs, const ModelDetails &model_details) {

    // Check if the provider is in the list of supported provider
    auto provider = GetProviderType(model_details.provider_name);
    if (provider == FLOCKMTL_UNSUPPORTED_PROVIDER) {

        throw std::runtime_error("Provider '" + model_details.provider_name +
                                 "' is not supported. Please choose one from the supported list: "
                                 "openai/default, azure, ollama, aws bedrock");
    }

    // Check if the provided model is in the list of supported models
    auto model = GetEmbeddingModelType(model_details.model, model_details.provider_name);
    if (model == FLOCKMTL_UNSUPPORTED_EMBEDDING_MODEL) {

        throw std::runtime_error(
            "Model '" + model_details.model +
            "' is not supported. Please choose one from the supported list: "
            "text-embedding-3-small, text-embedding-3-large, ollama-models (only with ollama as provider).");
    }

    auto result = CallEmbeddingProvider(inputs, model_details);
    return result.second;
}

std::pair<bool, nlohmann::json> ModelManager::CallCompleteProvider(const std::string &prompt,
                                                                   const ModelDetails &model_details,
                                                                   const bool json_response) {
    auto provider = GetProviderType(model_details.provider_name);
    switch (provider) {
    case FLOCKMTL_OPENAI:
        return {true, OpenAICallComplete(prompt, model_details, json_response)};
    case FLOCKMTL_AZURE:
        return {true, AzureCallComplete(prompt, model_details, json_response)};
    case FLOCKMTL_OLLAMA:
        return {true, OllamaCallComplete(prompt, model_details, json_response)};
    case FLOCKMTL_AWS_BEDROCK:
        return {true, AwsBedrockCallComplete(prompt, model_details, json_response)};
    default:
        return {false, {}};
    }
}

std::pair<bool, nlohmann::json> ModelManager::CallEmbeddingProvider(const std::vector<std::string> &inputs,
                                                                    const ModelDetails &model_details) {
    auto provider = GetProviderType(model_details.provider_name);
    switch (provider) {
    case FLOCKMTL_OPENAI:
        return {true, OpenAICallEmbedding(inputs, model_details)};
    case FLOCKMTL_AZURE:
        return {true, AzureCallEmbedding(inputs, model_details)};
    case FLOCKMTL_OLLAMA:
        return {true, OllamaCallEmbedding(inputs, model_details)};
    case FLOCKMTL_AWS_BEDROCK:
        return {true, AwsBedrockCallEmbedding(inputs, model_details)};
    default:
        return {false, {}};
    }
}

} // namespace core
} // namespace flockmtl