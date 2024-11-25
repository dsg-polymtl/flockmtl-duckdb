#include "flockmtl/core/config.hpp"

namespace flockmtl {

std::string Config::get_default_models_table_name() { return "FLOCKMTL_MODEL_DEFAULT_INTERNAL_TABLE"; }

std::string Config::get_user_defined_models_table_name() { return "FLOCKMTL_MODEL_USER_DEFINED_INTERNAL_TABLE"; }

void Config::setup_default_models_config(duckdb::Connection& con, std::string& schema_name) {
    const std::string table_name = Config::get_default_models_table_name();
    // Ensure schema exists
    auto result = con.Query(duckdb_fmt::format(" SELECT table_name "
                                               "   FROM information_schema.tables "
                                               "  WHERE table_schema = '{}' "
                                               "    AND table_name = '{}'; ",
                                               schema_name, table_name));
    if (result->RowCount() == 0) {
        con.Query(duckdb_fmt::format(" LOAD JSON; "
                                     " CREATE TABLE {}.{} ( "
                                     " model_name VARCHAR NOT NULL PRIMARY KEY, "
                                     " model VARCHAR NOT NULL, "
                                     " provider_name VARCHAR NOT NULL, "
                                     " model_args JSON NOT NULL "
                                     " ); ",
                                     schema_name, table_name));

        con.Query(duckdb_fmt::format(
            " INSERT INTO {}.{} (model_name, model, provider_name, model_args) "
            " VALUES "
            " ('default', 'gpt-4o-mini', 'openai', '{{\"context_window\": 128000, \"max_output_tokens\": 16384}}'),"
            " ('gpt-4o-mini', 'gpt-4o-mini', 'openai', '{{\"context_window\": 128000, \"max_output_tokens\": 16384}}'),"
            " ('gpt-4o', 'gpt-4o', 'openai', '{{\"context_window\": 128000, \"max_output_tokens\": 16384}}'),"
            " ('text-embedding-3-large', 'text-embedding-3-large', 'openai', "
            " '{{\"context_window\": {}, \"max_output_tokens\": {}}}',"
            " ('text-embedding-3-small', 'text-embedding-3-small', 'openai', "
            " '{{\"context_window\": {}, \"max_output_tokens\": {}}}')",
            schema_name, table_name, Config::default_context_window, Config::default_max_output_tokens,
            Config::default_context_window, Config::default_max_output_tokens));
    }
}

void Config::setup_user_defined_models_config(duckdb::Connection& con, std::string& schema_name) {
    const std::string table_name = Config::get_user_defined_models_table_name();
    // Ensure schema exists
    auto result = con.Query(duckdb_fmt::format(" SELECT table_name "
                                               "   FROM information_schema.tables "
                                               "  WHERE table_schema = '{}' "
                                               "    AND table_name = '{}'; ",
                                               schema_name, table_name));
    if (result->RowCount() == 0) {
        con.Query(duckdb_fmt::format("   LOAD JSON; "
                                     " CREATE TABLE {}.{} ( "
                                     " model_name VARCHAR NOT NULL PRIMARY KEY, "
                                     " model VARCHAR NOT NULL, "
                                     " provider_name VARCHAR NOT NULL, "
                                     " model_args JSON NOT NULL"
                                     " ); ",
                                     schema_name, table_name));
    }
}

void Config::ConfigModelTable(duckdb::Connection& con, std::string& schema_name) {
    setup_default_models_config(con, schema_name);
    setup_user_defined_models_config(con, schema_name);
}

} // namespace flockmtl
