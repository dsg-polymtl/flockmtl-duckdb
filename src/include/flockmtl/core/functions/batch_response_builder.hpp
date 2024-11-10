#pragma once
#include <nlohmann/json.hpp>
#include <flockmtl/common.hpp>
#include <flockmtl/core/model_manager/model_manager.hpp>

namespace flockmtl {
namespace core {

std::vector<nlohmann::json> CastVectorOfStructsToJson(Vector &struct_vector, int size);

struct PromptDetails {
    std::string prompt_name;
    std::string prompt;
};

PromptDetails CreatePromptDetails(Connection &con, const nlohmann::json prompt_details_json);

nlohmann::json Complete(const nlohmann::json &tuples, const std::string &user_prompt, const std::string &llm_template,
                        const ModelDetails &model_details);

nlohmann::json BatchAndComplete(std::vector<nlohmann::json> &tuples, Connection &con, std::string user_prompt_name,
                                const std::string &llm_template, const ModelDetails &model_details);

} // namespace core
} // namespace flockmtl