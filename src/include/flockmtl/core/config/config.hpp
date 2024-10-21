#ifndef FLOCKMTL_CONFIG_CONFIG_HPP
#define FLOCKMTL_CONFIG_CONFIG_HPP

#include "duckdb/main/connection.hpp"
#include "duckdb/main/database.hpp"

namespace flockmtl {
namespace core {

class Config {
public:
    static void Configure(duckdb::DatabaseInstance &db);

private:
    static void ConfigSchema(duckdb::Connection &con, std::string &schema_name);
    static void ConfigModelTable(duckdb::Connection &con, std::string &schema_name);
    static void ConfigPromptTable(duckdb::Connection &con, std::string &schema_name);
};

} // namespace core
} // namespace flockmtl

#endif // FLOCKMTL_CONFIG_CONFIG_HPP
