#include "flockmtl/query_manager/query.hpp"

namespace flockmtl {

Query &Query::insert(const std::string &table_name) {
    this->insert_clause = true;
    this->table_name = table_name;
    return *this;
}

Query &Query::columns(const std::string &columns) {
    this->insert_columns = columns;
    return *this;
}

Query &Query::value(const std::string &value) {
    this->insert_values.push_back(value);
    return *this;
}

std::string Query::build_insert() {
    auto query = "INSERT INTO " + this->table_name + " (" + this->insert_columns + ") VALUES ";
    for (auto &value : this->insert_values) {
        query += "(" + value + "),";
    }
    query.pop_back();
    query += ";";
    return query;
}

} // namespace flockmtl
