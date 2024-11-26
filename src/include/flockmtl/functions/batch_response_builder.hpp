#pragma once
#include <nlohmann/json.hpp>
#include "flockmtl/core/common.hpp"
#include "flockmtl/model_manager/model.hpp"
#include "flockmtl/prompt_manager/prompt_manager.hpp"
#include "flockmtl/model_manager/tiktoken.hpp"

namespace flockmtl {

std::vector<nlohmann::json> CastVectorOfStructsToJson(duckdb::Vector& struct_vector, int size);

} // namespace flockmtl
