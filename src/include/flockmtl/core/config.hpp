#pragma once

#include <fmt/format.h>

#include "flockmtl/core/common.hpp"
#include "flockmtl/registry/registry.hpp"

namespace flockmtl {

class Config {
public:
    static duckdb::DatabaseInstance* db;
    static duckdb::Connection GetConnection(duckdb::DatabaseInstance* db = nullptr);

    static void Configure(duckdb::DatabaseInstance& db);
    static std::string get_schema_name();
    static std::string get_default_models_table_name();
    static std::string get_user_defined_models_table_name();
    static std::string get_prompts_table_name();
    constexpr static int32_t default_context_window = 128000;
    constexpr static int32_t default_max_output_tokens = 4096;

private:
    static void ConfigSchema(duckdb::Connection& con, std::string& schema_name);
    static void ConfigModelTable(duckdb::Connection& con, std::string& schema_name);
    static void ConfigPromptTable(duckdb::Connection& con, std::string& schema_name);
    static void setup_default_models_config(duckdb::Connection& con, std::string& schema_name);
    static void setup_user_defined_models_config(duckdb::Connection& con, std::string& schema_name);
};

} // namespace flockmtl
