#include "flockmtl/custom_parser/query/secret_parser.hpp"

#include "flockmtl/common.hpp"
#include "flockmtl/core/module.hpp"

#include <sstream>
#include <stdexcept>

namespace flockmtl {

void SecretParser::Parse(const std::string& query, std::unique_ptr<QueryStatement>& statement) {
    Tokenizer tokenizer(query);
    Token token = tokenizer.NextToken();
    std::string value = StringUtil::Upper(token.value);

    if (token.type == TokenType::KEYWORD) {
        if (value == "CREATE") {
            ParseCreateSecret(tokenizer, statement);
        } else if (value == "DELETE") {
            ParseDeleteSecret(tokenizer, statement);
        } else if (value == "UPDATE") {
            ParseUpdateSecret(tokenizer, statement);
        } else if (value == "GET") {
            ParseGetSecret(tokenizer, statement);
        } else {
            throw std::runtime_error("Unknown keyword: " + token.value);
        }
    } else {
        throw std::runtime_error("Unknown keyword: " + token.value);
    }
}

void SecretParser::ParseCreateSecret(Tokenizer& tokenizer, std::unique_ptr<QueryStatement>& statement) {
    Token token = tokenizer.NextToken();
    std::string value = StringUtil::Upper(token.value);
    if (token.type != TokenType::KEYWORD || value != "SECRET") {
        throw std::runtime_error("Unknown keyword: " + token.value);
    }

    // the create secret format is as next CREATE SECRET OPENAI='key';
    token = tokenizer.NextToken();
    value = StringUtil::Upper(token.value);
    if (token.type != TokenType::KEYWORD || (value != "OPENAI" && value != "AZURE")) {
        throw std::runtime_error("Expected 'OPENAI' keyword after 'SECRET'.");
    }

    auto provider = StringUtil::Lower(value);

    token = tokenizer.NextToken();
    if (token.type != TokenType::SYMBOL || token.value != "=") {
        throw std::runtime_error("Expected '=' after 'OPENAI'.");
    }

    token = tokenizer.NextToken();
    if (token.type != TokenType::STRING_LITERAL || token.value.empty()) {
        throw std::runtime_error("Expected non-empty string literal for secret.");
    }
    auto secret = token.value;

    token = tokenizer.NextToken();
    if (token.type == TokenType::SYMBOL || token.value == ";") {
        auto create_statement = make_uniq<CreateSecretStatement>();
        create_statement->provider = provider;
        create_statement->secret = secret;
        statement = std::move(create_statement);
    } else {
        throw std::runtime_error("Unexpected characters after the secret. Only a semicolon is allowed.");
    }
}

void SecretParser::ParseDeleteSecret(Tokenizer& tokenizer, std::unique_ptr<QueryStatement>& statement) {
    auto token = tokenizer.NextToken();
    std::string value = StringUtil::Upper(token.value);
    if (token.type != TokenType::KEYWORD || value != "SECRET") {
        throw std::runtime_error("Unknown keyword: " + token.value);
    }

    token = tokenizer.NextToken();
    value = StringUtil::Upper(token.value);
    if (token.type != TokenType::KEYWORD || (value != "OPENAI" && value != "AZURE")) {
        throw std::runtime_error("Expected 'OPENAI' keyword after 'SECRET'.");
    }

    auto provider = StringUtil::Lower(value);

    token = tokenizer.NextToken();
    if (token.type == TokenType::SYMBOL || token.value == ";") {
        auto delete_statement = make_uniq<DeleteSecretStatement>();
        delete_statement->provider = provider;
        statement = std::move(delete_statement);
    } else {
        throw std::runtime_error("Unexpected characters after the secret. Only a semicolon is allowed.");
    }
}

void SecretParser::ParseUpdateSecret(Tokenizer& tokenizer, std::unique_ptr<QueryStatement>& statement) {
    auto token = tokenizer.NextToken();
    if (token.type != TokenType::KEYWORD || StringUtil::Upper(token.value) != "SECRET") {
        throw std::runtime_error("Unknown keyword: " + token.value);
    }

    token = tokenizer.NextToken();
    auto value = StringUtil::Upper(token.value);
    if (token.type != TokenType::KEYWORD || (value != "OPENAI" && value != "AZURE")) {
        throw std::runtime_error("Expected 'OPENAI' keyword after 'SECRET'.");
    }

    auto provider = StringUtil::Lower(value);

    token = tokenizer.NextToken();
    if (token.type != TokenType::SYMBOL || token.value != "=") {
        throw std::runtime_error("Expected '=' after 'OPENAI'.");
    }

    token = tokenizer.NextToken();
    if (token.type != TokenType::STRING_LITERAL || token.value.empty()) {
        throw std::runtime_error("Expected non-empty string literal for secret.");
    }
    auto secret = token.value;

    token = tokenizer.NextToken();
    if (token.type == TokenType::SYMBOL || token.value == ";") {
        auto update_statement = make_uniq<UpdateSecretStatement>();
        update_statement->provider = provider;
        update_statement->secret = secret;
        statement = std::move(update_statement);
    } else {
        throw std::runtime_error("Unexpected characters after the secret. Only a semicolon is allowed.");
    }
}

void SecretParser::ParseGetSecret(Tokenizer& tokenizer, std::unique_ptr<QueryStatement>& statement) {
    Token token = tokenizer.NextToken();
    auto value = StringUtil::Upper(token.value);
    if (token.type != TokenType::KEYWORD || (value != "SECRET" && value != "SECRETS")) {
        throw std::runtime_error("Unknown keyword: " + token.value);
    }

    token = tokenizer.NextToken();
    if ((token.type == TokenType::SYMBOL || token.value == ";") && value == "SECRETS") {
        auto get_all_statement = make_uniq<GetAllSecretStatement>();
        statement = std::move(get_all_statement);
    } else {
        value = StringUtil::Upper(token.value);
        if (token.type != TokenType::KEYWORD || (value != "OPENAI" && value != "AZURE")) {
            throw std::runtime_error("Expected 'OPENAI' or 'AZURE' keyword after 'SECRET'.");
        }
        auto provider = StringUtil::Lower(value);

        token = tokenizer.NextToken();
        if (token.type == TokenType::SYMBOL || token.value == ";") {
            auto get_statement = make_uniq<GetSecretStatement>();
            get_statement->provider = provider;
            statement = std::move(get_statement);
        } else {
            throw std::runtime_error("Unexpected characters after the secret. Only a semicolon is allowed.");
        }
    }
}

std::string SecretParser::ToSQL(const QueryStatement& statement) const {
    std::string query;

    switch (statement.type) {
    case StatementType::CREATE_SECRET: {
        const auto& create_stmt = static_cast<const CreateSecretStatement&>(statement);
        auto con = core::CoreModule::GetConnection();
        auto result = con.Query(
            duckdb_fmt::format("SELECT * FROM flockmtl_config.FLOCKMTL_SECRET_INTERNAL_TABLE WHERE provider = '{}';",
                               create_stmt.provider));
        if (result->RowCount() > 0) {
            throw std::runtime_error("OPENAI secret already exists.");
        }
        query =
            duckdb_fmt::format("INSERT INTO flockmtl_config.FLOCKMTL_SECRET_INTERNAL_TABLE(provider, secret) VALUES "
                               "('{}', '{}');",
                               create_stmt.provider, create_stmt.secret);
        break;
    }
    case StatementType::DELETE_SECRET: {
        const auto& delete_stmt = static_cast<const DeleteSecretStatement&>(statement);
        query = duckdb_fmt::format("DELETE FROM flockmtl_config.FLOCKMTL_SECRET_INTERNAL_TABLE WHERE provider = '{}';",
                                   delete_stmt.provider);
        break;
    }
    case StatementType::UPDATE_SECRET: {
        const auto& update_stmt = static_cast<const UpdateSecretStatement&>(statement);
        auto con = core::CoreModule::GetConnection();
        auto result = con.Query(
            duckdb_fmt::format("SELECT * FROM flockmtl_config.FLOCKMTL_SECRET_INTERNAL_TABLE WHERE provider = '{}';",
                               update_stmt.provider));
        if (result->RowCount() == 0) {
            throw std::runtime_error(duckdb_fmt::format("Provider '{}' secret does not exist.", update_stmt.provider));
        }
        query = duckdb_fmt::format(
            "UPDATE flockmtl_config.FLOCKMTL_SECRET_INTERNAL_TABLE SET secret = '{}' WHERE provider = '{}';",
            update_stmt.secret, update_stmt.provider);
        break;
    }
    case StatementType::GET_SECRET: {
        const auto& get_stmt = static_cast<const GetSecretStatement&>(statement);
        query = duckdb_fmt::format(
            "SELECT * FROM flockmtl_config.FLOCKMTL_SECRET_INTERNAL_TABLE WHERE provider = '{}';", get_stmt.provider);
        break;
    }
    case StatementType::GET_ALL_SECRET: {
        query = "SELECT * FROM flockmtl_config.FLOCKMTL_SECRET_INTERNAL_TABLE;";
        break;
    }
    default:
        throw std::runtime_error("Unknown statement type.");
    }

    return query;
}

} // namespace flockmtl
