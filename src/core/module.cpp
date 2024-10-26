#include "flockmtl/core/module.hpp"

#include "flockmtl/common.hpp"
#include "flockmtl/core/config/config.hpp"
#include "flockmtl/core/functions/aggregate.hpp"
#include "flockmtl/core/functions/scalar.hpp"

namespace flockmtl {
namespace core {

Connection &CoreModule::GetConnection(DatabaseInstance *db) {
    static Connection *con = nullptr;

    if (!con) {
        if (!db) {
            throw std::runtime_error("Connection is not initialized. Provide DatabaseInstance on first call.");
        }
        con = new Connection(*db);
    }

    return *con;
}

void CoreModule::Register(DatabaseInstance &db) {
    CoreModule::GetConnection(&db);
    CoreScalarFunctions::Register(db);
    CoreAggregateFunctions::Register(db);
    Config::Configure(db);
}

} // namespace core
} // namespace flockmtl
