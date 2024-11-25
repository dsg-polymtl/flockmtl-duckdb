#pragma once

#include "flockmtl/core/common.hpp"
#include "flockmtl/custom_parser/query_statements.hpp"
#include "flockmtl/custom_parser/tokenizer.hpp"

#include "fmt/format.h"
#include <memory>
#include <string>
#include <vector>

namespace flockmtl {

// Specific SECRET statements
class CreateSecretStatement : public QueryStatement {
public:
    CreateSecretStatement() { type = StatementType::CREATE_SECRET; }
    std::string provider;
    std::string secret;
};

class DeleteSecretStatement : public QueryStatement {
public:
    DeleteSecretStatement() { type = StatementType::DELETE_SECRET; }
    std::string provider;
};

class UpdateSecretStatement : public QueryStatement {
public:
    UpdateSecretStatement() { type = StatementType::UPDATE_SECRET; }
    std::string provider;
    std::string secret;
};

class GetSecretStatement : public QueryStatement {
public:
    GetSecretStatement() { type = StatementType::GET_SECRET; }
    std::string provider;
};

class GetAllSecretStatement : public QueryStatement {
public:
    GetAllSecretStatement() { type = StatementType::GET_ALL_SECRET; }
    std::string provider;
};

class SecretParser {
public:
    void Parse(const std::string& query, std::unique_ptr<QueryStatement>& statement);
    std::string ToSQL(const QueryStatement& statement) const;

private:
    void ParseCreateSecret(Tokenizer& tokenizer, std::unique_ptr<QueryStatement>& statement);
    void ParseDeleteSecret(Tokenizer& tokenizer, std::unique_ptr<QueryStatement>& statement);
    void ParseUpdateSecret(Tokenizer& tokenizer, std::unique_ptr<QueryStatement>& statement);
    void ParseGetSecret(Tokenizer& tokenizer, std::unique_ptr<QueryStatement>& statement);
};

} // namespace flockmtl
