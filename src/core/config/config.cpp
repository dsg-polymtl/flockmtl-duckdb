#include "flockmtl/core/config.hpp"
#include "flockmtl/secret_manager/secret_manager.hpp"

namespace flockmtl {

duckdb::DatabaseInstance* Config::db;

duckdb::Connection Config::GetConnection(duckdb::DatabaseInstance* db) {
    if (db) {
        Config::db = db;
    }
    duckdb::Connection con(*Config::db);
    return con;
}

std::string Config::get_schema_name() { return "flockmtl_config"; }

void Config::ConfigSchema(duckdb::Connection& con, std::string& schema_name) {

    // Check if schema exists using fmt
    auto result = con.Query(duckdb_fmt::format(" SELECT * "
                                               "   FROM information_schema.schemata "
                                               "  WHERE schema_name = '{}'; ",
                                               schema_name));
    if (result->RowCount() == 0) {
        con.Query(duckdb_fmt::format("CREATE SCHEMA {};", schema_name));
    }
}

void Config::Configure(duckdb::DatabaseInstance& db) {
    auto con = Config::GetConnection(&db);
    Registry::Register(db);
    SecretManager::Register(db);

    con.BeginTransaction();

    std::string schema = Config::get_schema_name();
    ConfigSchema(con, schema);
    ConfigModelTable(con, schema);
    ConfigPromptTable(con, schema);

    con.Commit();
}

} // namespace flockmtl
