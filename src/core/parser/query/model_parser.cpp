#include "flockmtl/core/parser/query/model_parser.hpp"

#include "flockmtl/common.hpp"

#include <sstream>
#include <stdexcept>

namespace flockmtl {

namespace core {
void ModelParser::Parse(const std::string &query, std::unique_ptr<QueryStatement> &statement) {
    Tokenizer tokenizer(query);
    Token token = tokenizer.NextToken();
    std::string value = StringUtil::Upper(token.value);

    if (token.type == TokenType::KEYWORD) {
        if (value == "CREATE") {
            ParseCreateModel(tokenizer, statement);
        } else if (value == "DELETE") {
            ParseDeleteModel(tokenizer, statement);
        } else if (value == "UPDATE") {
            ParseUpdateModel(tokenizer, statement);
        } else if (value == "GET") {
            ParseGetModel(tokenizer, statement);
        } else {
            throw std::runtime_error("Unknown keyword: " + token.value);
        }
    } else {
        throw std::runtime_error("Expected a keyword at the beginning of the query.");
    }
}

void ModelParser::ParseCreateModel(Tokenizer &tokenizer, std::unique_ptr<QueryStatement> &statement) {
    Token token = tokenizer.NextToken();
    std::string value = StringUtil::Upper(token.value);
    if (token.type != TokenType::KEYWORD || value != "MODEL") {
        throw std::runtime_error("Expected 'MODEL' after 'CREATE'.");
    }

    token = tokenizer.NextToken();
    if (token.type != TokenType::PARENTHESIS || token.value != "(") {
        throw std::runtime_error("Expected opening parenthesis '(' after 'MODEL'.");
    }

    token = tokenizer.NextToken();
    if (token.type != TokenType::STRING_LITERAL || token.value.empty()) {
        throw std::runtime_error("Expected non-empty string literal for model name.");
    }
    std::string model_name = token.value;

    token = tokenizer.NextToken();
    if (token.type != TokenType::SYMBOL || token.value != ",") {
        throw std::runtime_error("Expected comma ',' after model name.");
    }

    token = tokenizer.NextToken();
    if (token.type != TokenType::STRING_LITERAL || token.value.empty()) {
        throw std::runtime_error("Expected non-empty string literal for model.");
    }
    std::string model = token.value;

    token = tokenizer.NextToken();
    if (token.type != TokenType::SYMBOL || token.value != ",") {
        throw std::runtime_error("Expected comma ',' after model.");
    }

    token = tokenizer.NextToken();
    if (token.type != TokenType::NUMBER || token.value.empty()) {
        throw std::runtime_error("Expected integer value for max_tokens.");
    }
    int max_tokens = std::stoi(token.value);

    token = tokenizer.NextToken();
    if (token.type != TokenType::PARENTHESIS || token.value != ")") {
        throw std::runtime_error("Expected closing parenthesis ')' after max_tokens.");
    }

    token = tokenizer.NextToken();
    if (token.type == TokenType::END_OF_FILE) {
        auto create_statement = std::make_unique<CreateModelStatement>();
        create_statement->model_name = model_name;
        create_statement->model = model;
        create_statement->max_tokens = max_tokens;
        statement = std::move(create_statement);
    } else {
        throw std::runtime_error("Unexpected characters after the closing parenthesis. Only a semicolon is allowed.");
    }
}

void ModelParser::ParseDeleteModel(Tokenizer &tokenizer, std::unique_ptr<QueryStatement> &statement) {
    Token token = tokenizer.NextToken();
    std::string value = StringUtil::Upper(token.value);
    if (token.type != TokenType::KEYWORD || value != "MODEL") {
        throw std::runtime_error("Expected 'MODEL' after 'DELETE'.");
    }

    token = tokenizer.NextToken();
    if (token.type != TokenType::STRING_LITERAL || token.value.empty()) {
        throw std::runtime_error("Expected non-empty string literal for model name.");
    }

    std::string model_name = token.value;
    token = tokenizer.NextToken();
    if (token.type == TokenType::SYMBOL || token.value == ";") {
        auto delete_statement = std::make_unique<DeleteModelStatement>();
        delete_statement->model_name = model_name;
        statement = std::move(delete_statement);
    } else {
        throw std::runtime_error("Unexpected characters after the closing parenthesis. Only a semicolon is allowed.");
    }
}

void ModelParser::ParseUpdateModel(Tokenizer &tokenizer, std::unique_ptr<QueryStatement> &statement) {
    Token token = tokenizer.NextToken();
    std::string value = StringUtil::Upper(token.value);
    if (token.type != TokenType::KEYWORD || value != "MODEL") {
        throw std::runtime_error("Expected 'MODEL' after 'UPDATE'.");
    }

    token = tokenizer.NextToken();
    if (token.type != TokenType::PARENTHESIS || token.value != "(") {
        throw std::runtime_error("Expected opening parenthesis '(' after 'MODEL'.");
    }

    token = tokenizer.NextToken();
    if (token.type != TokenType::STRING_LITERAL || token.value.empty()) {
        throw std::runtime_error("Expected non-empty string literal for model name.");
    }
    std::string model_name = token.value;

    token = tokenizer.NextToken();
    if (token.type != TokenType::SYMBOL || token.value != ",") {
        throw std::runtime_error("Expected comma ',' after model name.");
    }

    token = tokenizer.NextToken();
    if (token.type != TokenType::STRING_LITERAL || token.value.empty()) {
        throw std::runtime_error("Expected non-empty string literal for model.");
    }
    std::string new_model = token.value;

    token = tokenizer.NextToken();
    if (token.type != TokenType::SYMBOL || token.value != ",") {
        throw std::runtime_error("Expected comma ',' after model.");
    }

    token = tokenizer.NextToken();
    if (token.type != TokenType::NUMBER || token.value.empty()) {
        throw std::runtime_error("Expected integer value for new max_tokens.");
    }
    int new_max_tokens = std::stoi(token.value);

    token = tokenizer.NextToken();
    if (token.type != TokenType::PARENTHESIS || token.value != ")") {
        throw std::runtime_error("Expected closing parenthesis ')' after new max_tokens.");
    }

    token = tokenizer.NextToken();
    if (token.type == TokenType::END_OF_FILE) {
        auto update_statement = std::make_unique<UpdateModelStatement>();
        update_statement->new_model = new_model;
        update_statement->model_name = model_name;
        update_statement->new_max_tokens = new_max_tokens;
        statement = std::move(update_statement);
    } else {
        throw std::runtime_error("Unexpected characters after the closing parenthesis. Only a semicolon is allowed.");
    }
}

void ModelParser::ParseGetModel(Tokenizer &tokenizer, std::unique_ptr<QueryStatement> &statement) {
    Token token = tokenizer.NextToken();
    std::string value = StringUtil::Upper(token.value);
    if (token.type != TokenType::KEYWORD || (value != "MODEL" && value != "MODELS")) {
        throw std::runtime_error("Expected 'MODEL' after 'GET'.");
    }

    token = tokenizer.NextToken();
    if (token.type == TokenType::SYMBOL || token.value == ";") {
        auto get_all_statement = std::make_unique<GetAllModelStatement>();
        statement = std::move(get_all_statement);
    } else {
        if (token.type != TokenType::STRING_LITERAL || token.value.empty()) {
            throw std::runtime_error("Expected non-empty string literal for model name.");
        }
        std::string model_name = token.value;

        token = tokenizer.NextToken();
        if (token.type == TokenType::SYMBOL || token.value == ";") {
            auto get_statement = std::make_unique<GetModelStatement>();
            get_statement->model_name = model_name;
            statement = std::move(get_statement);
        } else {
            throw std::runtime_error("Unexpected characters after the model name. Only a semicolon is allowed.");
        }
    }
}

std::string ModelParser::ToSQL(const QueryStatement &statement) const {
    std::ostringstream sql;

    switch (statement.type) {
    case StatementType::CREATE_MODEL: {
        const auto &create_stmt = static_cast<const CreateModelStatement &>(statement);
        sql << "INSERT INTO flockmtl_config.FLOCKMTL_MODEL_INTERNAL_TABLE(model_name, model, max_tokens) VALUES ('"
            << create_stmt.model_name << "', '" << create_stmt.model << "', " << create_stmt.max_tokens << ");";
        break;
    }
    case StatementType::DELETE_MODEL: {
        const auto &delete_stmt = static_cast<const DeleteModelStatement &>(statement);
        sql << "DELETE FROM flockmtl_config.FLOCKMTL_MODEL_INTERNAL_TABLE WHERE model_name = '"
            << delete_stmt.model_name << "';";
        break;
    }
    case StatementType::UPDATE_MODEL: {
        const auto &update_stmt = static_cast<const UpdateModelStatement &>(statement);
        sql << "UPDATE flockmtl_config.FLOCKMTL_MODEL_INTERNAL_TABLE SET "
            << "max_tokens = " << update_stmt.new_max_tokens << ", "
            << "model = '" << update_stmt.new_model << "' "
            << "WHERE model_name = '" << update_stmt.model_name << "';";
        break;
    }
    case StatementType::GET_MODEL: {
        const auto &get_stmt = static_cast<const GetModelStatement &>(statement);
        sql << "SELECT * FROM flockmtl_config.FLOCKMTL_MODEL_INTERNAL_TABLE WHERE model_name = '" << get_stmt.model_name
            << "';";
        break;
    }
    case StatementType::GET_ALL_MODEL: {
        sql << "SELECT * FROM flockmtl_config.FLOCKMTL_MODEL_INTERNAL_TABLE;";
        break;
    }
    default:
        throw std::runtime_error("Unknown statement type.");
    }

    return sql.str();
}

} // namespace core

} // namespace flockmtl
