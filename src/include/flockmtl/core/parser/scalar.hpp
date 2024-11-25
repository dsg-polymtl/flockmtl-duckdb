#pragma once
#include "flockmtl/core/common.hpp"

#include <nlohmann/json.hpp>

namespace flockmtl {

struct CoreScalarParsers {
    static std::vector<nlohmann::json> Struct2Json(duckdb::Vector& struct_vector, int size);
    static void LlmCompleteJsonScalarParser(duckdb::DataChunk& args);
    static void LlmCompleteScalarParser(duckdb::DataChunk& args);
    static void LlmFilterScalarParser(duckdb::DataChunk& args);
    static void LlmEmbeddingScalarParser(duckdb::DataChunk& args);
    static void FusionRelativeScalarParser(duckdb::DataChunk& args);
};

} // namespace flockmtl
