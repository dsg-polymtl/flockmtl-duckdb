#pragma once
#include "large_flock/common.hpp"

namespace large_flock {

namespace core {

struct CoreScalarFunctions {
    static void Register(DatabaseInstance &db) {
        RegisterLlmCompleteJsonScalarFunction(db);
        RegisterLlmCompleteScalarFunction(db);
        RegisterLlmEmbeddingScalarFunction(db);
        RegisterLlmFilterScalarFunction(db);
    }

private:
    static void RegisterLlmCompleteJsonScalarFunction(DatabaseInstance &db);
    static void RegisterLlmCompleteScalarFunction(DatabaseInstance &db);
    static void RegisterLlmEmbeddingScalarFunction(DatabaseInstance &db);
    static void RegisterLlmFilterScalarFunction(DatabaseInstance &db);
};

} // namespace core

} // namespace large_flock
