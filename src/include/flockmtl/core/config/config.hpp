#ifndef FLOCKMTL_CONFIG_CONFIG_HPP
#define FLOCKMTL_CONFIG_CONFIG_HPP

#include "duckdb/main/connection.hpp"
#include "duckdb/main/database.hpp"

namespace flockmtl {
namespace core {

class Config {
public:
    static void Configure(duckdb::DatabaseInstance &db);
    static std::string get_schema_name ();
    static std::string get_default_models_table_name ();
    static std::string get_user_defined_models_table_name ();
    static std::string get_prompts_table_name ();

private:
    static void ConfigSchema(duckdb::Connection &con, std::string &schema_name);
    static void ConfigModelTable(duckdb::Connection &con, std::string &schema_name);
    static void ConfigPromptTable(duckdb::Connection &con, std::string &schema_name);
    static void setup_default_models_config (duckdb::Connection &con, std::string &schema_name);
    static void setup_user_defined_models_config (duckdb::Connection &con, std::string &schema_name);
};

} // namespace core
} // namespace flockmtl

#endif // FLOCKMTL_CONFIG_CONFIG_HPP
