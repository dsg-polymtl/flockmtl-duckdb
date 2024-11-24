#include "flockmtl/core/config/config.hpp"

#include <iostream>

namespace flockmtl {
namespace core {

std::string Config::get_schema_name() { return "flockmtl_config"; }

std::string Config::get_default_models_table_name() { return "FLOCKMTL_MODEL_DEFAULT_INTERNAL_TABLE"; }

std::string Config::get_user_defined_models_table_name() { return "FLOCKMTL_MODEL_USER_DEFINED_INTERNAL_TABLE"; }

std::string Config::get_prompts_table_name() { return "FLOCKMTL_PROMPT_INTERNAL_TABLE"; }

std::string Config::get_secrets_table_name() { return "FLOCKMTL_SECRET_INTERNAL_TABLE"; }

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

    Query query(con);
    auto result = query.from("information_schema.schemata").where("schema_name = '" + schema_name + "'").execute();

    if (result.empty()) {
        query.reset();
        query.create(CREATE_TYPE::SCHEMA).schema(schema_name).execute();
    }
}

void Config::setup_default_models_config(duckdb::Connection &con, std::string &schema_name) {
    const std::string table_name = Config::get_default_models_table_name();

    Query query(con);
    auto result = query.from("information_schema.tables")
                      .where("table_schema = '" + schema_name + "'")
                      .where("table_name = '" + table_name + "'")
                      .execute();

    if (result.empty()) {
        con.Query("LOAD JSON;");
        query.reset();
        query.create(CREATE_TYPE::TABLE)
            .table(schema_name + "." + table_name)
            .column("model_name VARCHAR NOT NULL PRIMARY KEY")
            .column("model VARCHAR NOT NULL")
            .column("provider_name VARCHAR NOT NULL")
            .column("model_args JSON NOT NULL")
            .execute();

        query.reset();
        query.insert(schema_name + "." + table_name)
            .columns("model_name, model, provider_name, model_args")
            .value("'default', 'gpt-4o-mini', 'openai', '{\"context_window\": 128000, \"max_output_tokens\": 16384}'")
            .value(
                "'gpt-4o-mini', 'gpt-4o-mini', 'openai', '{\"context_window\": 128000, \"max_output_tokens\": 16384}'")
            .value("'gpt-4o', 'gpt-4o', 'openai', '{\"context_window\": 128000, \"max_output_tokens\": 16384}'")
            .value("'text-embedding-3-large', 'text-embedding-3-large', 'openai', "
                   "'{\"context_window\": " +
                   std::to_string(Config::default_context_window) +
                   ", \"max_output_tokens\": " + std::to_string(Config::default_max_output_tokens) + "}'")
            .value("'text-embedding-3-small', 'text-embedding-3-small', 'openai', "
                   "'{\"context_window\": " +
                   std::to_string(Config::default_context_window) +
                   ", \"max_output_tokens\": " + std::to_string(Config::default_max_output_tokens) + "}'")
            .execute();
    }
}

void Config::setup_user_defined_models_config(duckdb::Connection &con, std::string &schema_name) {
    const std::string table_name = Config::get_user_defined_models_table_name();
    // Ensure schema exists
    Query query(con);
    auto result = query.from("information_schema.tables")
                      .where("table_schema = '" + schema_name + "'")
                      .where("table_name = '" + table_name + "'")
                      .execute();
    if (result.empty()) {
        con.Query("LOAD JSON;");
        query.reset();
        query.create(CREATE_TYPE::TABLE)
            .table(schema_name + "." + table_name)
            .column("model_name VARCHAR NOT NULL PRIMARY KEY")
            .column("model VARCHAR NOT NULL")
            .column("provider_name VARCHAR NOT NULL")
            .column("model_args JSON NOT NULL")
            .execute();
    }
}

void Config::ConfigModelTable(duckdb::Connection &con, std::string &schema_name) {
    setup_default_models_config(con, schema_name);
    setup_user_defined_models_config(con, schema_name);
}

void Config::ConfigPromptTable(duckdb::Connection &con, std::string &schema_name) {
    const std::string table_name = "FLOCKMTL_PROMPT_INTERNAL_TABLE";

    Query query(con);
    auto result = query.from("information_schema.tables")
                      .where("table_schema = '" + schema_name + "'")
                      .where("table_name = '" + table_name + "'")
                      .execute();

    if (result.empty()) {
        query.reset();
        query.create(CREATE_TYPE::TABLE)
            .table(schema_name + "." + table_name)
            .column("prompt_name VARCHAR NOT NULL")
            .column("prompt VARCHAR NOT NULL")
            .column("update_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP")
            .column("version INT DEFAULT 1")
            .primary_key("prompt_name, version")
            .execute();

        query.reset();
        query.insert(schema_name + "." + table_name)
            .columns("prompt_name, prompt")
            .value("'hello-world', 'Tell me hello world'")
            .execute();
    }
}

void Config::ConfigSecretTable(duckdb::Connection &con, std::string &schema_name) {
    const std::string table_name = get_secrets_table_name();
    Query query(con);
    auto result = query.from("information_schema.tables")
                      .where("table_schema = '" + schema_name + "'")
                      .where("table_name = '" + table_name + "'")
                      .execute();

    if (result.empty()) {
        query.reset();
        query.create(CREATE_TYPE::TABLE)
            .table(schema_name + "." + table_name)
            .column("provider VARCHAR NOT NULL PRIMARY KEY")
            .column("secret VARCHAR NOT NULL")
            .execute();
    }
}

} // namespace core
} // namespace flockmtl
