#include "flockmtl/functions/scalar/llm_filter.hpp"
#include "flockmtl/registry/registry.hpp"

namespace flockmtl {

void ScalarRegistry::RegisterLlmFilter(duckdb::DatabaseInstance& db) {
    duckdb::ExtensionUtil::RegisterFunction(db, duckdb::ScalarFunction("llm_filter", {}, duckdb::LogicalType::VARCHAR,
                                                                       LlmFilter::Execute, nullptr, nullptr, nullptr,
                                                                       nullptr, duckdb::LogicalType::ANY));
}

} // namespace flockmtl
