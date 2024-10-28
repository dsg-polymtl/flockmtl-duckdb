#pragma once
#include "flockmtl/common.hpp"

namespace flockmtl {

namespace core {

struct CoreAggregateFunctions {
    static void Register(DatabaseInstance &db) {
        RegisterLlmMaxFunction(db);
        RegisterLlmMinFunction(db);
        RegisterLlmRerankFunction(db);
    }

private:
    static void RegisterLlmMaxFunction(DatabaseInstance &db);
    static void RegisterLlmMinFunction(DatabaseInstance &db);
    static void RegisterLlmRerankFunction(DatabaseInstance &db);
};

} // namespace core

} // namespace flockmtl
