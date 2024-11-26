#include "flockmtl/functions/scalar/llm_complete.hpp"
#include "flockmtl/registry/registry.hpp"

namespace flockmtl {

void ScalarRegistry::RegisterLlmComplete(duckdb::DatabaseInstance& db) {
    duckdb::ExtensionUtil::RegisterFunction(db, duckdb::ScalarFunction("llm_complete", {}, duckdb::LogicalType::VARCHAR,
                                                                       LlmComplete::Execute, nullptr, nullptr, nullptr,
                                                                       nullptr, duckdb::LogicalType::ANY));
}

} // namespace flockmtl
