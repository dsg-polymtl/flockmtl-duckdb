#pragma once
#include "flockmtl/common.hpp"
#include "nlohmann/json.hpp"

#include <utility>

namespace flockmtl {
namespace core {

struct ModelDetails {
    std::string model;
    std::string model_name;
    std::string provider_name;
    int max_tokens;
    float temperature;
};

struct ModelManager {
public:
    static ModelDetails CreateModelDetails(Connection &con, const nlohmann::json &model_json);

    static nlohmann::json CallComplete(const std::string &prompt, const ModelDetails &model_details,
                                       const bool json_response = true);

    static nlohmann::json CallEmbedding(const vector<string> &inputs, const ModelDetails &model_details);

private:
    static std::pair<std::string, int32_t> GetQueriedModel(Connection &con, const std::string &model_name,
                                                           const std::string &provider_name);

    static nlohmann::json OpenAICallComplete(const std::string &prompt, const ModelDetails &model_details,
                                             const bool json_response);

    static nlohmann::json AzureCallComplete(const std::string &prompt, const ModelDetails &model_details,
                                            const bool json_response);

    static nlohmann::json OllamaCallComplete(const std::string &prompt, const ModelDetails &model_details,
                                             const bool json_response);

    static nlohmann::json AwsBedrockCallComplete(const std::string &prompt, const ModelDetails &model_details,
                                                 const bool json_response);

    static nlohmann::json OpenAICallEmbedding(const vector<string> &inputs, const ModelDetails &model_details);

    static nlohmann::json AzureCallEmbedding(const vector<string> &inputs, const ModelDetails &model_details);

    static nlohmann::json OllamaCallEmbedding(const std::string &input, const ModelDetails &model_details);

    static nlohmann::json AwsBedrockCallEmbedding(const std::string &input, const ModelDetails &model_details);

    static nlohmann::json CallCompleteProvider(const std::string &prompt, const ModelDetails &model_details,
                                               const bool json_response);

    static nlohmann::json CallEmbeddingProvider(const std::string &prompt, const ModelDetails &model_details);
};

} // namespace core

} // namespace flockmtl
