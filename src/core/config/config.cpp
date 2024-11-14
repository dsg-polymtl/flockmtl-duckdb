#include "flockmtl/core/config/config.hpp"

#include <iostream>

namespace flockmtl {
namespace core {

std::string Config::get_schema_name() {
    return "flockmtl_config";
}

std::string Config::get_default_models_table_name() {
    return "FLOCKMTL_MODEL_DEFAULT_INTERNAL_TABLE";
}

std::string Config::get_user_defined_models_table_name() {
    return "FLOCKMTL_MODEL_USER_DEFINED_INTERNAL_TABLE";
}

std::string Config::get_prompts_table_name() {
    return "FLOCKMTL_PROMPT_INTERNAL_TABLE";
}

std::string Config::get_secrets_table_name() {
    return "FLOCKMTL_SECRET_INTERNAL_TABLE";
}

void Config::Configure(duckdb::DatabaseInstance &db) {
    std::string schema = Config::get_schema_name();
    duckdb::Connection con(db);
    con.BeginTransaction();

    ConfigSchema(con, schema);
    ConfigModelTable(con, schema);
    ConfigPromptTable(con, schema);
    ConfigSecretTable(con, schema);

    con.Commit();
}

void Config::ConfigSchema(duckdb::Connection &con, std::string &schema_name) {

    // Ensure schema exists
    auto result = con.Query("SELECT * FROM information_schema.schemata WHERE schema_name = '" + schema_name + "';");
    if (result->RowCount() == 0) {
        con.Query("CREATE SCHEMA " + schema_name + ";");
    }
}

void Config::setup_default_models_config(duckdb::Connection &con, std::string &schema_name) {
    const std::string table_name = Config::get_default_models_table_name();
    // Ensure schema exists
    auto result = con.Query("SELECT table_name FROM information_schema.tables WHERE table_schema = '" + schema_name +
                            "' AND table_name = '" + table_name + "';");
    if (result->RowCount() == 0) {
        con.Query("LOAD JSON;"
                  "CREATE TABLE " +
                  schema_name + "." + table_name +
                  " ("
                  "model_name VARCHAR NOT NULL PRIMARY KEY, "
                  "model VARCHAR NOT NULL, "
                  "provider_name VARCHAR NOT NULL, "
                  "model_args JSON NOT NULL"
                  ");");

        con.Query(
            "INSERT INTO " + schema_name + "." + table_name +
            " (model_name, model, provider_name, model_args) VALUES "
            "('default', 'gpt-4o-mini', 'openai', '{\"context_window\": 128000, \"max_output_tokens\": 16384}'),"
            "('gpt-4o-mini', 'gpt-4o-mini', 'openai', '{\"context_window\": 128000, \"max_output_tokens\": 16384}'),"
            "('gpt-4o', 'gpt-4o', 'openai', '{\"context_window\": 128000, \"max_output_tokens\": 16384}'),"
            "('text-embedding-3-large', 'text-embedding-3-large', 'openai', "
            "'{\"context_window\": " +
            std::to_string(Config::default_context_window) +
            ", "
            "\"max_output_tokens\": " +
            std::to_string(Config::default_max_output_tokens) +
            "}'),"
            "('text-embedding-3-small', 'text-embedding-3-small', 'openai', "
            "'{\"context_window\": " +
            std::to_string(Config::default_context_window) +
            ", "
            "\"max_output_tokens\": " +
            std::to_string(Config::default_max_output_tokens) + "}');");
    }
}

void Config::setup_user_defined_models_config(duckdb::Connection &con, std::string &schema_name) {
    const std::string table_name = Config::get_user_defined_models_table_name();
    // Ensure schema exists
    auto result = con.Query("SELECT table_name FROM information_schema.tables WHERE table_schema = '" + schema_name +
                            "' AND table_name = '" + table_name + "';");
    if (result->RowCount() == 0) {
        con.Query("LOAD JSON;"
                  "CREATE TABLE " +
                  schema_name + "." + table_name +
                  " ("
                  "model_name VARCHAR NOT NULL PRIMARY KEY,"
                  "model VARCHAR,"
                  "provider_name VARCHAR NOT NULL,"
                  "model_args JSON NOT NULL"
                  ");");
    }
}

void Config::ConfigModelTable(duckdb::Connection &con, std::string &schema_name) {
    setup_default_models_config(con, schema_name);
    setup_user_defined_models_config(con, schema_name);
}

void Config::ConfigPromptTable(duckdb::Connection &con, std::string &schema_name) {
    const std::string table_name = "FLOCKMTL_PROMPT_INTERNAL_TABLE";

    auto result = con.Query("SELECT table_name FROM information_schema.tables WHERE table_schema = '" + schema_name +
                            "' AND table_name = '" + table_name + "';");
    if (result->RowCount() == 0) {
        con.Query("CREATE TABLE " + schema_name + "." + table_name +
                  " ("
                  "prompt_name VARCHAR NOT NULL,"
                  "prompt VARCHAR NOT NULL,"
                  "update_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                  "version INT DEFAULT 1,"
                  "PRIMARY KEY (prompt_name, version)"
                  ");");

        con.Query("INSERT INTO " + schema_name + "." + table_name +
                  " (prompt_name ,prompt) VALUES ('hello-world', 'Tell me hello world');");
    }
}

void Config::ConfigSecretTable(duckdb::Connection &con, std::string &schema_name) {
    const std::string table_name = get_secrets_table_name();

    auto result = con.Query("SELECT table_name FROM information_schema.tables WHERE table_schema = '" + schema_name +
                            "' AND table_name = '" + table_name + "';");
    if (result->RowCount() == 0) {
        con.Query("CREATE TABLE " + schema_name + "." + table_name +
                  " ("
                  "provider VARCHAR NOT NULL PRIMARY KEY,"
                  "secret VARCHAR NOT NULL"
                  ");");
    }
}

} // namespace core
} // namespace flockmtl
