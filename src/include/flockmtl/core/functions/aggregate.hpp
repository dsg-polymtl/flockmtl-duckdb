#pragma once
#include "flockmtl/common.hpp"

namespace flockmtl {

namespace core {

struct CoreAggregateFunctions {
    static void Register(DatabaseInstance &db) {
        RegisterStringConcatFunction(db);
    }

private:
    static void RegisterStringConcatFunction(DatabaseInstance &db);
};

} // namespace core

} // namespace flockmtl
