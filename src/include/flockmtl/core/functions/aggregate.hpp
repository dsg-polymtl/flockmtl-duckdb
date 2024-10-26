#pragma once
#include "flockmtl/common.hpp"

namespace flockmtl {

namespace core {

struct CoreAggregateFunctions {
    static void Register(DatabaseInstance &db) {
        RegisterLlmMaxFunction(db);
    }

private:
    static void RegisterLlmMaxFunction(DatabaseInstance &db);
};

} // namespace core

} // namespace flockmtl
