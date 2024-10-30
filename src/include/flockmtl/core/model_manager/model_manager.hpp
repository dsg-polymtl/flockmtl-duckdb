#pragma once
#include "flockmtl/common.hpp"
#include "nlohmann/json.hpp"
#include <utility>

namespace flockmtl {
namespace core {

struct ModelManager {
public:
    static nlohmann::json CallComplete(const std::string &prompt, const std::string &model, const std::string &provider,
                                       const nlohmann::json &settings, const bool json_response = true);

    static nlohmann::json CallEmbedding(const std::string &input, const std::string &model, const std::string &provider);

    static nlohmann::json CallComplete(const std::string &prompt, const std::string &model, const nlohmann::json &settings,
                                       const bool json_response = true);

    static nlohmann::json CallEmbedding(const std::string &input, const std::string &model);

    static std::pair<std::string, int32_t> GetQueriedModel (Connection& con, const std::string& model_name, const std::string& provider_name);

private:
    static nlohmann::json OpenAICallComplete(const std::string &prompt, const std::string &model,
                                                const nlohmann::json &settings, const bool json_response);

    static nlohmann::json AzureCallComplete(const std::string &prompt, const std::string &model,
                                                const nlohmann::json &settings, const bool json_response);

    static nlohmann::json OpenAICallEmbedding(const std::string &input, const std::string &model);

    static nlohmann::json AzureCallEmbedding(const std::string &input, const std::string &model);
};

} // namespace core

} // namespace flockmtl
