#include "flockmtl/model_manager/providers/adapters/ollama.hpp"

namespace flockmtl {

nlohmann::json OllamaProvider::CallComplete(const std::string& prompt, const bool json_response) {
    auto ollama_model_manager_uptr = std::make_unique<OllamaModelManager>(false);

    // Create a JSON request payload with the provided parameters
    nlohmann::json request_payload = {{"model", model_details_.model},
                                      {"prompt", prompt},
                                      {"stream", false},
                                      {"options",
                                       {
                                           {"temperature", model_details_.temperature},
                                           {"num_ctx", model_details_.max_output_tokens},
                                       }},
                                      {"keep_alive", -1}};

    // Conditionally add "response_format" if json_response is true
    if (json_response) {
        request_payload["format"] = "json";
    }

    nlohmann::json completion;
    try {
        completion = ollama_model_manager_uptr->CallComplete(request_payload);
    } catch (const std::exception& e) {
        throw std::runtime_error(duckdb_fmt::format("Error in making request to Ollama API: {}", e.what()));
    }

    // Check if the call was not succesfull
    if ((completion.contains("done_reason") && completion["done_reason"] != "stop") ||
        (completion.contains("done") && !completion["done"].is_null() && completion["done"].get<bool>() != true)) {
        // Handle refusal error
        throw std::runtime_error("The request was refused due to some internal error with Ollama API");
    }

    std::string content_str = completion["response"];

    if (json_response) {
        return nlohmann::json::parse(content_str);
    }

    return content_str;
}

nlohmann::json OllamaProvider::CallEmbedding(const std::vector<std::string>& inputs) {
    auto ollama_model_manager_uptr = std::make_unique<OllamaModelManager>(false);

    // Create a JSON request payload with the provided parameters
    nlohmann::json request_payload = {
        {"model", model_details_.model},
        {"input", inputs},
    };

    nlohmann::json completion;
    try {
        completion = ollama_model_manager_uptr->CallEmbedding(request_payload);
    } catch (const std::exception& e) {
        throw std::runtime_error(duckdb_fmt::format("Error in making request to Ollama API: {}", e.what()));
    }

    auto embeddings = nlohmann::json::array();
    for (auto& item : completion["embeddings"]) {
        embeddings.push_back(item);
    }

    return embeddings;
}

} // namespace flockmtl