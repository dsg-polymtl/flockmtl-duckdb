#include "flockmtl/query_manager/query.hpp"

namespace flockmtl {

Query::Query(duckdb::Connection &connection) : connection(connection) {}

Query &Query::where(const std::string &where_clause) {
    this->where_clause.push_back(where_clause);
    return *this;
}

std::string Query::build() {
    if (select_clause) {
        return build_select();
    } else if (insert_clause) {
        return build_insert();
    } else if (create_clause) {
        return build_create();
    } else if (update_clause) {
        return build_update();
    } else if (delete_clause) {
        return build_delete();
    }

    std::runtime_error("Invalid query");
}

void Query::reset() {
    create_clause = false;
    create_type = CREATE_TYPE::TABLE;
    schema_name.clear();
    create_columns.clear();
    create_primary_key.clear();
    create_foreign_key.clear();

    update_clause = false;
    update_columns.clear();
    update_values.clear();

    delete_clause = false;

    insert_clause = false;
    insert_columns.clear();
    insert_values.clear();

    select_clause = false;
    select_columns.clear();
    order_by_clause.clear();
    limit_clause.clear();

    table_name.clear();
    where_clause.clear();
}

std::vector<std::vector<std::any>> Query::execute() {
    std::vector<std::vector<std::any>> result;
    auto query_result = connection.Query(build());

    if (select_clause) {
        for (int i = 0; i < query_result->RowCount(); i++) {
            result.emplace_back();
            for (int j = 0; j < query_result->ColumnCount(); j++) {
                auto value = query_result->GetValue(j, i);
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

    return result;
}

} // namespace flockmtl
