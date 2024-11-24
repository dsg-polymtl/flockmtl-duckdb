#pragma once
#include <nlohmann/json.hpp>
#include <flockmtl/common.hpp>
#include <flockmtl/model_manager/model.hpp>
#include <flockmtl/prompt_manager/prompt_manager.hpp>

namespace flockmtl {
namespace core {

std::vector<nlohmann::json> CastVectorOfStructsToJson(Vector &struct_vector, int size);

struct PromptDetails {
    std::string prompt_name;
    std::string prompt;
    int version;
};

std::string ConstructMarkdownHeader(const nlohmann::json &tuple);

std::string ConstructMarkdownSingleTuple(const nlohmann::json &tuple);

std::string ConstructMarkdownArrayTuples(const nlohmann::json &tuples);

PromptDetails CreatePromptDetails(Connection &con, const nlohmann::json prompt_details_json);

nlohmann::json Complete(const nlohmann::json &tuples, const std::string &user_prompt, ScalarFunctionType function_type,
                        ModelDetails &model_details);

nlohmann::json BatchAndComplete(std::vector<nlohmann::json> &tuples, Connection &con, std::string user_prompt_name,
                                ScalarFunctionType function_type, Model &model);

} // namespace core
} // namespace flockmtl