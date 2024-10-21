#pragma once
#include "flockmtl/common.hpp"

#include <nlohmann/json.hpp>

namespace flockmtl {

namespace core {

struct CoreScalarParsers {
    static std::vector<nlohmann::json> Struct2Json(Vector &struct_vector, int size);
    static void LlmCompleteJsonScalarParser(DataChunk &args);
    static void LlmCompleteScalarParser(DataChunk &args);
    static void LlmFilterScalarParser(DataChunk &args);
    static void LlmEmbeddingScalarParser(DataChunk &args);
};

} // namespace core

} // namespace flockmtl
