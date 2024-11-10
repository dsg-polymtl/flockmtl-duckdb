#ifndef _FLOCK_MTL_MODEL_MANAGER_AZURE_H
#define _FLOCK_MTL_MODEL_MANAGER_AZURE_H

#include <iostream>
#include <stdexcept>
#include <string>
#include "session.hpp"
#include <nlohmann/json.hpp>

namespace flockmtl {
namespace core {

class AzureModelManager {
public:
    AzureModelManager(std::string token, std::string resource_name, std::string deployment_model_name,
                      std::string api_version, bool throw_exception)
        : _token(token), _resource_name(resource_name), _deployment_model_name(deployment_model_name),
          _api_version(api_version), _session("Azure", throw_exception), _throw_exception(throw_exception) {
        _session.setToken(token, "");
    }

    AzureModelManager(const AzureModelManager &) = delete;
    AzureModelManager &operator=(const AzureModelManager &) = delete;
    AzureModelManager(AzureModelManager &&) = delete;
    AzureModelManager &operator=(AzureModelManager &&) = delete;

    nlohmann::json CallComplete(const nlohmann::json &json, const std::string &contentType = "application/json") {
        std::string url = "https://" + _resource_name + ".openai.azure.com/openai/deployments/" +
                          _deployment_model_name + "/chat/completions?api-version=" + _api_version;
        _session.setUrl(url);
        return execute_post(json.dump(), contentType);
    }

    nlohmann::json CallEmbedding(const nlohmann::json &json, const std::string &contentType = "application/json") {
        string url = "https://" + _resource_name + ".openai.azure.com/openai/deployments/" + _deployment_model_name +
                     "/embeddings?api-version=" + _api_version;
        _session.setUrl(url);
        return execute_post(json.dump(), contentType);
    }

    // I am adding it here since I want to keep provider specific calls
    // inside same file
    static const char *get_azure_api_key() {
        static int check_done = -1;
        static const char *api_key = nullptr;

        if (check_done == -1) {
            api_key = std::getenv("AZURE_API_KEY");
            check_done = 1;
        }

        if (!api_key) {
            throw std::runtime_error("AZURE_API_KEY environment variable is not set.");
        }

        return api_key;
    }

    static const char *get_azure_resource_name() {
        static int check_done = -1;
        static const char *rname = nullptr;

        if (check_done == -1) {
            rname = std::getenv("AZURE_RESOURCE_NAME");
            check_done = 1;
        }

        if (!rname) {
            throw std::runtime_error("AZURE_RESOURCE_NAME environment variable is not set.");
        }

        return rname;
    }

    static const char *get_azure_api_version() {
        static int check_done = -1;
        static const char *api_version = nullptr;

        if (check_done == -1) {
            api_version = std::getenv("AZURE_API_VERSION");
            check_done = 1;
        }

        if (!api_version) {
            throw std::runtime_error("AZURE_VERSION environment variable is not set.");
        }

        return api_version;
    }

private:
    std::string _token;
    std::string _resource_name;
    std::string _deployment_model_name;
    std::string _api_version;
    Session _session;
    bool _throw_exception;

    nlohmann::json execute_post(const std::string &data, const std::string &contentType) {
        setParameters(data, contentType);
        auto response = _session.postPrepare(contentType);
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
            std::cerr << "[Azure] error. Reason: " << msg << '\n';
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