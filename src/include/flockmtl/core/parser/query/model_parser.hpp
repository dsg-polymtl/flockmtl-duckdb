#pragma once

#include "flockmtl/common.hpp"
#include "flockmtl/core/parser/query_statements.hpp"
#include "flockmtl/core/parser/tokenizer.hpp"

#include <memory>
#include <string>
#include <vector>

namespace flockmtl {

namespace core {

// Specific statements
class CreateModelStatement : public QueryStatement {
public:
    CreateModelStatement() {
        type = StatementType::CREATE_MODEL;
    }
    std::string model_name;
    std::string model;
    int max_tokens;
};

class DeleteModelStatement : public QueryStatement {
public:
    DeleteModelStatement() {
        type = StatementType::DELETE_MODEL;
    }
    std::string model_name;
};

class UpdateModelStatement : public QueryStatement {
public:
    UpdateModelStatement() {
        type = StatementType::UPDATE_MODEL;
    }
    std::string model_name;
    std::string new_model;
    int new_max_tokens;
};

class GetModelStatement : public QueryStatement {
public:
    GetModelStatement() {
        type = StatementType::GET_MODEL;
    }
    std::string model_name;
};

class GetAllModelStatement : public QueryStatement {
public:
    GetAllModelStatement() {
        type = StatementType::GET_ALL_MODEL;
    }
};

class ModelParser {
public:
    void Parse(const std::string &query, std::unique_ptr<QueryStatement> &statement);
    std::string ToSQL(const QueryStatement &statement) const;

private:
    void ParseCreateModel(Tokenizer &tokenizer, std::unique_ptr<QueryStatement> &statement);
    void ParseDeleteModel(Tokenizer &tokenizer, std::unique_ptr<QueryStatement> &statement);
    void ParseUpdateModel(Tokenizer &tokenizer, std::unique_ptr<QueryStatement> &statement);
    void ParseGetModel(Tokenizer &tokenizer, std::unique_ptr<QueryStatement> &statement);
};
} // namespace core

} // namespace flockmtl