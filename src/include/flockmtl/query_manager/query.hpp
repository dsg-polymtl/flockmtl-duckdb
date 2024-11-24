#pragma once

#include <stdexcept>
#include <vector>
#include <string>
#include <any>

#include "duckdb/main/connection.hpp"
#include "flockmtl/core/module.hpp"

namespace flockmtl {

enum class FLOCK_TABLE { DEFAULT_MODEL, USER_DEFINED_MODEL, PROMPT, SECRET };
enum class CREATE_TYPE { TABLE, SCHEMA };

class Query {
public:
    explicit Query(duckdb::Connection &connection);

    // CREATE clause
    Query &create(CREATE_TYPE type);
    Query &table(const std::string &table_name);
    Query &schema(const std::string &schema_name);
    Query &column(const std::string &column);
    Query &primary_key(const std::string &primary_key);
    Query &foreign_key(const std::string &foreign_key);

    // UPDATE clause
    Query &update(const std::string &table_name);
    Query &set(const std::vector<std::string> &columns);
    Query &values(const std::vector<std::string> &values);

    // DELETE clause
    Query &delete_from(const std::string &table_name);

    // INSERT clause
    Query &insert(const std::string &table_name);
    Query &columns(const std::string &columns);
    Query &value(const std::string &value);

    // SELECT clause
    Query &select(const std::string &select_clause);
    Query &from(const std::string &table_name);
    Query &from(FLOCK_TABLE table);
    Query &order_by(const std::string &order_by_clause);
    Query &limit(const std::string &limit_clause);

    // Global methods
    Query &where(const std::string &where_clause);

    void reset();
    std::string build();
    std::vector<std::vector<std::any>> execute();

private:
    std::string build_create();
    std::string build_insert();
    std::string build_select();
    std::string build_update();
    std::string build_delete();

private:
    duckdb::Connection &connection;

    // CREATE clause
    bool create_clause;
    CREATE_TYPE create_type;
    std::string schema_name;
    vector<std::string> create_columns;
    std::string create_primary_key;
    std::string create_foreign_key;

    // UPDATE clause
    bool update_clause;
    std::vector<std::string> update_columns;
    std::vector<std::string> update_values;

    // DELETE clause
    bool delete_clause;

    // INSERT clause
    bool insert_clause;
    std::string insert_columns;
    std::vector<std::string> insert_values;

    // SELECT clause
    bool select_clause;
    std::string select_columns;
    std::string order_by_clause;
    std::string limit_clause;

    // Global attributes
    std::string table_name;
    std::vector<std::string> where_clause;
};

} // namespace flockmtl
