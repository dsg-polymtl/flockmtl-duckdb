#include "flockmtl/core/module.hpp"

#include "flockmtl/common.hpp"
#include "flockmtl/core/config/config.hpp"
#include "flockmtl/core/functions/aggregate.hpp"
#include "flockmtl/core/functions/scalar.hpp"

namespace flockmtl {
namespace core {

DatabaseInstance *CoreModule::db;

Connection CoreModule::GetConnection(DatabaseInstance *db) {
    if (db) {
        CoreModule::db = db;
    }
    Connection con(*CoreModule::db);
    return con;
}

void CoreModule::Register(DatabaseInstance &db) {
    CoreModule::GetConnection(&db);
    CoreScalarFunctions::Register(db);
    CoreAggregateFunctions::Register(db);
    Config::Configure(db);
}

} // namespace core
} // namespace flockmtl
