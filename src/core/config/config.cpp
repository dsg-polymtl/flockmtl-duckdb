#include "flockmtl/core/config/config.hpp"

#include <iostream>

namespace flockmtl {
namespace core {

void Config::Configure(duckdb::DatabaseInstance &db) {
    std::string schema = "flockmtl_config";
    duckdb::Connection con(db);
    con.BeginTransaction();

    ConfigSchema(con, schema);
    ConfigModelTable(con, schema);
    ConfigPromptTable(con, schema);

    con.Commit();
}

void Config::ConfigSchema(duckdb::Connection &con, std::string &schema_name) {

    // Ensure schema exists
    auto result = con.Query("SELECT * FROM information_schema.schemata WHERE schema_name = '" + schema_name + "';");
    if (result->RowCount() == 0) {
        con.Query("CREATE SCHEMA " + schema_name + ";");
    }
}

void Config::ConfigModelTable(duckdb::Connection &con, std::string &schema_name) {
    const std::string table_name = "FLOCKMTL_MODEL_INTERNAL_TABLE";

    // Ensure schema exists
    auto result = con.Query("SELECT table_name FROM information_schema.tables WHERE table_schema = '" + schema_name +
                            "' AND table_name = '" + table_name + "';");
    if (result->RowCount() == 0) {
        con.Query("CREATE TABLE " + schema_name + "." + table_name +
                  " ("
                  "model_name VARCHAR NOT NULL PRIMARY KEY,"
                  "model VARCHAR,"
                  "max_tokens INTEGER NOT NULL"
                  ");");

        con.Query("INSERT INTO " + schema_name + "." + table_name +
                  " (model_name, model, max_tokens) VALUES "
                  "('default', 'gpt-4o-mini', 128000),"
                  "('gpt-4o-mini', 'gpt-4o-mini', 128000),"
                  "('gpt-4o', 'gpt-4o', 128000)");
    }
}

void Config::ConfigPromptTable(duckdb::Connection &con, std::string &schema_name) {
    const std::string table_name = "FLOCKMTL_PROMPT_INTERNAL_TABLE";

    auto result = con.Query("SELECT table_name FROM information_schema.tables WHERE table_schema = '" + schema_name +
                            "' AND table_name = '" + table_name + "';");
    if (result->RowCount() == 0) {
        con.Query("CREATE TABLE " + schema_name + "." + table_name +
                  " ("
                  "prompt_name VARCHAR NOT NULL PRIMARY KEY,"
                  "prompt VARCHAR NOT NULL"
                  ");");

        con.Query("INSERT INTO " + schema_name + "." + table_name +
                  " (prompt_name ,prompt) VALUES ('hello-world', 'Tell me hello world');");
    }
}

} // namespace core
} // namespace flockmtl
