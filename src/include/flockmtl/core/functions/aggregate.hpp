#pragma once
#include "flockmtl/common.hpp"

namespace flockmtl {

namespace core {

struct CoreAggregateFunctions {
    static void Register(DatabaseInstance &db) {
        RegisterLlmFirstFunction(db);
        RegisterLlmLastFunction(db);
        RegisterLlmRerankFunction(db);
    }

private:
    static void RegisterLlmFirstFunction(DatabaseInstance &db);
    static void RegisterLlmLastFunction(DatabaseInstance &db);
    static void RegisterLlmRerankFunction(DatabaseInstance &db);
};

} // namespace core

} // namespace flockmtl
