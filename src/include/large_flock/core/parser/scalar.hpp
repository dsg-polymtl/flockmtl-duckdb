#pragma once
#include "large_flock/common.hpp"

#include <nlohmann/json.hpp>

namespace large_flock {

namespace core {

struct CoreScalarParsers {
    static std::vector<nlohmann::json> Struct2Json(Vector &struct_vector, int size);
    static void LlmMapScalarParser(DataChunk &args);
    static void LlmCompleteScalarParser(DataChunk &args);
    static void LlmEmbeddingScalarParser(DataChunk &args);
};

} // namespace core

} // namespace large_flock
