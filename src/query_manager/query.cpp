#include "flockmtl/query_manager/query.hpp"

namespace flockmtl {

Query::Query(duckdb::Connection &connection) : connection(connection) {}

Query &Query::select(const std::string &select_clause) {
    this->select_clause = select_clause;
    return *this;
}

Query &Query::from(const std::string &table) {
    this->table = table;
    return *this;
}

Query &Query::from(Table table) {
    switch (table) {
    case Table::DEFAULT_MODEL:
        this->table = "flockmtl_config.FLOCKMTL_MODEL_DEFAULT_INTERNAL_TABLE";
        break;
    case Table::USER_DEFINED_MODEL:
        this->table = "flockmtl_config.FLOCKMTL_MODEL_USER_DEFINED_INTERNAL_TABLE";
        break;
    case Table::PROMPT:
        this->table = "flockmtl_config.FLOCKMTL_PROMPT_INTERNAL_TABLE";
        break;
    case Table::SECRET:
        this->table = "flockmtl_config.FLOCKMTL_SECRET_INTERNAL_TABLE";
        break;
    }
    return *this;
}

Query &Query::where(const std::string &where_clause) {
    this->where_clause = where_clause;
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

// Build query string
std::string Query::build() {
    if (table.empty()) {
        throw std::runtime_error("Table name is required");
    }

    if (select_clause.empty()) {
        select_clause = "*";
    }

    std::string query = "SELECT " + select_clause + " FROM " + table;
    if (!where_clause.empty()) {
        query += " WHERE " + where_clause;
    }
    if (!order_by_clause.empty()) {
        query += " ORDER BY " + order_by_clause;
    }
    if (!limit_clause.empty()) {
        query += " LIMIT " + limit_clause;
    }
    return query;
}

// Execute query
std::vector<std::vector<std::any>> Query::execute() {
    std::vector<std::vector<std::any>> result;

    auto query_result = connection.Query(build());
    for (int i = 0; i < query_result->RowCount(); i++) {
        result.emplace_back();
        for (int j = 0; j < query_result->ColumnCount(); j++) {
            auto value = query_result->GetValue(i, j);
            if (value.type() == duckdb::LogicalType::INTEGER) {
                result[i].emplace_back(value.GetValue<int>());
            } else if (value.type() == duckdb::LogicalType::BIGINT) {
                result[i].emplace_back(value.GetValue<int64_t>());
            } else if (value.type() == duckdb::LogicalType::FLOAT) {
                result[i].emplace_back(value.GetValue<float>());
            } else if (value.type() == duckdb::LogicalType::DOUBLE) {
                result[i].emplace_back(value.GetValue<double>());
            } else if (value.type() == duckdb::LogicalType::VARCHAR) {
                result[i].emplace_back(value.GetValue<std::string>());
            } else {
                result[i].emplace_back(value.GetValue<std::string>());
            }
        }
    }
    return result;
}

} // namespace flockmtl
