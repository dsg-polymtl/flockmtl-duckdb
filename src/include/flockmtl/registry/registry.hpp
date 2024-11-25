#pragma once

#include "flockmtl/core/common.hpp"
#include "flockmtl/registry/aggregate.hpp"
#include "flockmtl/registry/scalar.hpp"

namespace flockmtl {

class Registry {
public:
    static void Register(duckdb::DatabaseInstance& db);

private:
    static void RegisterAggregateFunctions(duckdb::DatabaseInstance& db);
    static void RegisterScalarFunctions(duckdb::DatabaseInstance& db);
};

} // namespace flockmtl
