#pragma once

#include <stdexcept>
#include <vector>
#include <string>
#include <any>

#include "duckdb/main/connection.hpp"
#include "flockmtl/core/module.hpp"

namespace flockmtl {

enum class Table { DEFAULT_MODEL, USER_DEFINED_MODEL, PROMPT, SECRET };

class Query {
public:
    explicit Query(duckdb::Connection &connection);

    Query &select(const std::string &select_clause);
    Query &from(const std::string &table);
    Query &from(Table table);
    Query &where(const std::string &where_clause);
    Query &order_by(const std::string &order_by_clause);
    Query &limit(const std::string &limit_clause);

    std::string build();
    std::vector<std::vector<std::any>> execute();

private:
    duckdb::Connection &connection;
    std::string select_clause;
    std::string table;
    std::string where_clause;
    std::string order_by_clause;
    std::string limit_clause;
};

} // namespace flockmtl
