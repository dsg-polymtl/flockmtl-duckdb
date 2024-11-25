#pragma once
#include <nlohmann/json.hpp>
#include "flockmtl/core/common.hpp"
#include "flockmtl/model_manager/model.hpp"
#include "flockmtl/prompt_manager/prompt_manager.hpp"
#include "flockmtl/model_manager/tiktoken.hpp"

namespace flockmtl {

std::vector<nlohmann::json> CastVectorOfStructsToJson(duckdb::Vector& struct_vector, int size);

PromptDetails CreatePromptDetails(duckdb::Connection& con, const nlohmann::json prompt_details_json);

nlohmann::json Complete(const nlohmann::json& tuples, const std::string& user_prompt, ScalarFunctionType function_type,
                        ModelDetails& model_details);

nlohmann::json BatchAndComplete(std::vector<nlohmann::json>& tuples, duckdb::Connection& con,
                                std::string user_prompt_name, ScalarFunctionType function_type, Model& model);

} // namespace flockmtl
