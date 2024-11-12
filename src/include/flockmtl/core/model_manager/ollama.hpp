#ifndef _FLOCK_MTL_MODEL_MANAGER_OLLAMA_H
#define _FLOCK_MTL_MODEL_MANAGER_OLLAMA_H

#include "session.hpp"

#include <iostream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

namespace flockmtl {
namespace core {

class OllamaModelManager {
public:
    OllamaModelManager(bool throw_exception) : _session("Ollama", throw_exception), _throw_exception(throw_exception) {
    }
    OllamaModelManager(const OllamaModelManager &) = delete;
    OllamaModelManager &operator=(const OllamaModelManager &) = delete;
    OllamaModelManager(OllamaModelManager &&) = delete;
    OllamaModelManager &operator=(OllamaModelManager &&) = delete;

    std::string GetChatUrl (){
        static int check_done = -1;
        static const char *chat_url = nullptr;

        if (check_done == -1) {
            chat_url = std::getenv("OLLAMA_CHAT_URL");
            check_done = 1;
        }

        if (!chat_url) {
            throw std::runtime_error("OLLAMA_CHAT_URL environment variable is not set.");
        }

        return chat_url;
    }

    std::string GetEmbedUrl (){
        static int check_done = -1;
        static const char *embed_url = nullptr;

        if (check_done == -1) {
            embed_url = std::getenv("OLLAMA_EMBED_URL");
            check_done = 1;
        }

        if (!embed_url) {
            throw std::runtime_error("OLLAMA_EMBED_URL environment variable is not set.");
        }

        return embed_url;
    }

    nlohmann::json CallComplete(const nlohmann::json &json, const std::string &contentType = "application/json") {
        std::string url = GetChatUrl();
        _session.setUrl(url);
        return execute_post(json.dump(), contentType);
    }

    nlohmann::json CallEmbedding(const nlohmann::json &json, const std::string &contentType = "application/json") {
        std::string url = GetEmbedUrl();
        _session.setUrl(url);
        return execute_post(json.dump(), contentType);
    }

    bool validModel(const std::string &user_model_name) {
        auto response = _session.validOllamaModelsJson();
        auto json = nlohmann::json::parse(response.text);
        bool res = false;
        for (const auto &model : json["models"]) {
            if (model.contains("name")) {
                const auto &available_model = model["name"].get<std::string>();
                res |= available_model.find(user_model_name) != std::string::npos;
            }
        }
        return res;
    }

private:
    Session _session;
    bool _throw_exception;

    nlohmann::json execute_post(const std::string &data, const std::string &contentType) {
        setParameters(data, contentType);
        auto response = _session.postPrepareOllama(contentType);
        if (response.is_error) {
            trigger_error(response.error_message);
        }

        nlohmann::json json {};
        if (isJson(response.text)) {

            json = nlohmann::json::parse(response.text);
            checkResponse(json);
        } else {
            trigger_error("Response is not a valid JSON");
        }

        return json;
    }

    void trigger_error(const std::string &msg) {
        if (_throw_exception) {
            throw std::runtime_error(msg);
        } else {
            std::cerr << "[Ollama] error. Reason: " << msg << '\n';
        }
    }

    void checkResponse(const nlohmann::json &json) {
        if (json.count("error")) {
            auto reason = json["error"].dump();
            trigger_error(reason);
            std::cerr << ">> response error :\n" << json.dump(2) << "\n";
        }
    }

    bool isJson(const std::string &data) {
        bool rc = true;
        try {
            auto json = nlohmann::json::parse(data); // throws if no json
        } catch (std::exception &) {
            rc = false;
        }
        return (rc);
    }

    void setParameters(const std::string &data, const std::string &contentType = "") {
        if (contentType != "multipart/form-data") {
            _session.setBody(data);
        }
    }
};

} // namespace core
} // namespace flockmtl
#endif