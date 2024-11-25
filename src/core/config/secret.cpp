#include "flockmtl/core/config.hpp"

namespace flockmtl {

std::string Config::get_secrets_table_name() { return "FLOCKMTL_SECRET_INTERNAL_TABLE"; }

void Config::ConfigSecretTable(duckdb::Connection& con, std::string& schema_name) {
    const std::string table_name = get_secrets_table_name();

    auto result = con.Query(duckdb_fmt::format(" SELECT table_name "
                                               "   FROM information_schema.tables "
                                               "  WHERE table_schema = '{}' "
                                               "    AND table_name = '{}'; ",
                                               schema_name, table_name));
    if (result->RowCount() == 0) {
        con.Query(duckdb_fmt::format(" CREATE TABLE {}.{} ( "
                                     " provider VARCHAR NOT NULL PRIMARY KEY, "
                                     " secret VARCHAR NOT NULL "
                                     " ); ",
                                     schema_name, table_name));
    }
}

} // namespace flockmtl
