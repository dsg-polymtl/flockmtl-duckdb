#include "flockmtl/functions/scalar/fusion_relative.hpp"
#include "flockmtl/registry/registry.hpp"

namespace flockmtl {

void ScalarRegistry::RegisterFusionRelative(duckdb::DatabaseInstance& db) {
    duckdb::ExtensionUtil::RegisterFunction(
        db, duckdb::ScalarFunction("fusion_relative", {}, duckdb::LogicalType::DOUBLE, FusionRelative::Execute, nullptr,
                                   nullptr, nullptr, nullptr, duckdb::LogicalType::ANY));
}

} // namespace flockmtl
