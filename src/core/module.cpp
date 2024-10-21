#include "flockmtl/core/module.hpp"

#include "flockmtl/common.hpp"
#include "flockmtl/core/config/config.hpp"
#include "flockmtl/core/functions/scalar.hpp"

namespace flockmtl {
namespace core {

void CoreModule::Register(DatabaseInstance &db) {
    CoreScalarFunctions::Register(db);
    Config::Configure(db);
}

} // namespace core
} // namespace flockmtl
