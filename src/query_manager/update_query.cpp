#include "flockmtl/query_manager/query.hpp"

namespace flockmtl {

// update

Query &Query::update(const std::string &table_name) {
    this->update_clause = true;
    this->table_name = table_name;
    return *this;
}

Query &Query::set(const std::vector<std::string> &columns) {
    this->update_columns = columns;
    return *this;
}

Query &Query::values(const std::vector<std::string> &values) {
    this->update_values = values;
    return *this;
}

std::string Query::build_update() {
    std::string query = "UPDATE " + table_name + " SET ";
    if (update_columns.size() != update_values.size()) {
        throw std::runtime_error("Invalid number of columns and values");
    }
    for (size_t i = 0; i < update_columns.size(); i++) {
        query += update_columns[i] + " = " + update_values[i];
        if (i != update_columns.size() - 1) {
            query += ", ";
        }
    }
    if (!where_clause.empty()) {
        query += " WHERE ";
        for (size_t i = 0; i < where_clause.size(); i++) {
            query += where_clause[i];
            if (i != where_clause.size() - 1) {
                query += " AND ";
            }
        }
    }
    return query;
}

} // namespace flockmtl
