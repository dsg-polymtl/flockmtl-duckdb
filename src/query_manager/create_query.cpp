#include "flockmtl/query_manager/query.hpp"

namespace flockmtl {

Query &Query::create(CREATE_TYPE create_type) {
    this->create_clause = true;
    this->create_type = create_type;
    return *this;
}

Query &Query::table(const std::string &table_name) {
    this->table_name = table_name;
    return *this;
}

Query &Query::schema(const std::string &schema_name) {
    this->schema_name = schema_name;
    return *this;
}

Query &Query::column(const std::string &column) {
    this->create_columns.push_back(column);
    return *this;
}

Query &Query::primary_key(const std::string &primary_key) {
    this->create_primary_key = primary_key;
    return *this;
}

Query &Query::foreign_key(const std::string &foreign_key) {
    this->create_foreign_key = foreign_key;
    return *this;
}

std::string Query::build_create() {
    std::string query = "CREATE ";
    switch (this->create_type) {
    case CREATE_TYPE::TABLE:
        query += "TABLE ";
        query += this->table_name + " (";
        for (auto &column : this->create_columns) {
            query += column + ",";
        }
        if (!this->create_primary_key.empty()) {
            query += "PRIMARY KEY (" + this->create_primary_key + ")";
        }
        if (!this->create_foreign_key.empty()) {
            query += ", FOREIGN KEY (" + this->create_foreign_key + ")";
        }
        query += ");";
        return query;
    case CREATE_TYPE::SCHEMA:
        query += "SCHEMA ";
        query += this->schema_name + ";";
        return query;
    default:
        std::runtime_error("Invalid CREATE clause");
    }
}

} // namespace flockmtl
