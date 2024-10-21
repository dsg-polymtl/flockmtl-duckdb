#include <flockmtl/common.hpp>
#include <flockmtl/core/model_manager/model_manager.hpp>
#include <flockmtl/core/model_manager/openai.hpp>
#include <flockmtl_extension.hpp>

namespace flockmtl {
namespace core {

std::string ModelManager::OPENAI_API_KEY = "";

nlohmann::json ModelManager::CallComplete(const std::string &prompt, const std::string &model,
                                          const nlohmann::json &settings, const bool json_response) {
    // List of supported models
    static const std::unordered_set<std::string> supported_models = {"gpt-4o", "gpt-4o-mini"};

    // Check if the provided model is in the list of supported models
    if (supported_models.find(model) == supported_models.end()) {
        throw std::invalid_argument("Model '" + model +
                                    "' is not supported. Please choose one from the supported list: "
                                    "gpt-4o, gpt-4o-mini.");
    }

    openai::start(ModelManager::OPENAI_API_KEY);

    // check if settings is not empty and has max_tokens and temperature else make some default values
    auto max_tokens = 4000;
    auto temperature = 0.5;
    if (!settings.empty()) {
        for (auto &[key, value] : settings.items()) {
            if (key == "max_tokens") {
                max_tokens = std::stoi(static_cast<std::string>(value));
            } else if (key == "temperature") {
                temperature = std::stof(static_cast<std::string>(value));
            } else {
                throw std::invalid_argument("Invalid setting key: " + key);
            }
        }
    }

    // Create a JSON request payload with the provided parameters
    nlohmann::json request_payload = {{"model", model},
                                      {"messages", {{{"role", "user"}, {"content", prompt}}}},
                                      {"max_tokens", max_tokens},
                                      {"temperature", temperature}};

    // Conditionally add "response_format" if json_response is true
    if (json_response) {
        request_payload["response_format"] = {{"type", "json_object"}};
    }

    // Make a request to the OpenAI API
    auto completion = openai::chat().create(request_payload);

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

nlohmann::json ModelManager::CallEmbedding(const std::string &input, const std::string &model) {
    // List of supported models
    static const std::unordered_set<std::string> supported_models = {"text-embedding-3-small",
                                                                     "text-embedding-3-large"};

    // Check if the provided model is in the list of supported models
    if (supported_models.find(model) == supported_models.end()) {
        throw std::invalid_argument("Model '" + model +
                                    "' is not supported. Please choose one from the supported list: "
                                    "text-embedding-3-small, text-embedding-3-large.");
    }

    // Get API key from the environment variable
    const char *key = std::getenv("OPENAI_API_KEY");
    if (!key) {
        throw std::runtime_error("OPENAI_API_KEY environment variable is not set.");
    } else {
        openai::start(); // Assume it uses the environment variable if no API
                         // key is provided
    }

    // Create a JSON request payload with the provided parameters
    nlohmann::json request_payload = {
        {"model", model},
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

} // namespace core
} // namespace flockmtl