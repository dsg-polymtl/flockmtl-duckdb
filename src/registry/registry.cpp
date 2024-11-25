#include "flockmtl/registry/registry.hpp"

namespace flockmtl {

void Registry::Register(duckdb::DatabaseInstance& db) {
    RegisterAggregateFunctions(db);
    RegisterScalarFunctions(db);
}

void Registry::RegisterAggregateFunctions(duckdb::DatabaseInstance& db) { AggregateRegistry::Register(db); }

void Registry::RegisterScalarFunctions(duckdb::DatabaseInstance& db) { ScalarRegistry::Register(db); }

} // namespace flockmtl
