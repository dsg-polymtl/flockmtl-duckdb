#include "flockmtl/query_manager/query.hpp"

namespace flockmtl {

// delete

Query &Query::delete_from(const std::string &table_name) {
    this->delete_clause = true;
    this->table_name = table_name;
    return *this;
}

std::string Query::build_delete() {
    std::string query = "DELETE FROM " + table_name;
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
