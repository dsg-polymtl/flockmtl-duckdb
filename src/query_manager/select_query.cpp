#include "flockmtl/query_manager/query.hpp"

namespace flockmtl {

Query &Query::select(const std::string &select_columns) {
    this->select_clause = true;
    this->select_columns = select_columns;
    return *this;
}

Query &Query::from(const std::string &table_name) {
    this->table_name = table_name;
    return *this;
}

Query &Query::from(FLOCK_TABLE table) {
    switch (table) {
    case FLOCK_TABLE::DEFAULT_MODEL:
        this->table_name = "flockmtl_config.FLOCKMTL_MODEL_DEFAULT_INTERNAL_TABLE";
        break;
    case FLOCK_TABLE::USER_DEFINED_MODEL:
        this->table_name = "flockmtl_config.FLOCKMTL_MODEL_USER_DEFINED_INTERNAL_TABLE";
        break;
    case FLOCK_TABLE::PROMPT:
        this->table_name = "flockmtl_config.FLOCKMTL_PROMPT_INTERNAL_TABLE";
        break;
    case FLOCK_TABLE::SECRET:
        this->table_name = "flockmtl_config.FLOCKMTL_SECRET_INTERNAL_TABLE";
        break;
    }
    return *this;
}

Query &Query::order_by(const std::string &order_by_clause) {
    this->order_by_clause = order_by_clause;
    return *this;
}

Query &Query::limit(const std::string &limit_clause) {
    this->limit_clause = limit_clause;
    return *this;
}

std::string Query::build_select() {
    if (table_name.empty()) {
        throw std::runtime_error("Table name is required");
    }

    if (select_columns.empty()) {
        select_clause = "*";
    }

    std::string query = "SELECT " + select_columns + " FROM " + table_name;
    if (!where_clause.empty()) {
        query += " WHERE ";
        for (auto i = 0; i < where_clause.size(); i++) {
            query += where_clause[i];
            if (i < where_clause.size() - 1) {
                query += " AND ";
            }
        }
    }
    if (!order_by_clause.empty()) {
        query += " ORDER BY " + order_by_clause;
    }
    if (!limit_clause.empty()) {
        query += " LIMIT " + limit_clause;
    }
    return query;
}

} // namespace flockmtl
