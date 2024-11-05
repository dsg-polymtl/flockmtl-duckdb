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
    if (token.type != TokenType::STRING_LITERAL || token.value.empty()) {
        throw std::runtime_error("Expected non-empty string literal for provider_name.");
    }
    std::string provider_name = token.value;

    token = tokenizer.NextToken();
    if (token.type != TokenType::SYMBOL || token.value != ",") {
        throw std::runtime_error("Expected comma ',' after provider_name.");
    }

    token = tokenizer.NextToken();
    if (token.type != TokenType::NUMBER || token.value.empty()) {
        throw std::runtime_error("Expected integer value for context_window.");
    }
    int context_window = std::stoi(token.value);

    token = tokenizer.NextToken();
    if (token.type != TokenType::SYMBOL || token.value != ",") {
        throw std::runtime_error("Expected comma ',' after context_window.");
    }

    token = tokenizer.NextToken();
    if (token.type != TokenType::NUMBER || token.value.empty()) {
        throw std::runtime_error("Expected integer value for max_output_tokens.");
    }
    int max_output_tokens = std::stoi(token.value);

    token = tokenizer.NextToken();
    if (token.type != TokenType::PARENTHESIS || token.value != ")") {
        throw std::runtime_error("Expected closing parenthesis ')' after max_output_tokens.");
    }

    token = tokenizer.NextToken();
    if (token.type == TokenType::END_OF_FILE) {
        auto create_statement = std::make_unique<CreateModelStatement>();
        create_statement->model_name = model_name;
        create_statement->model = model;
        create_statement->provider_name = provider_name;
        create_statement->context_window = context_window;
        create_statement->max_output_tokens = max_output_tokens;
        statement = std::move(create_statement);
    } else {
        throw std::runtime_error("Unexpected characters after the closing parenthesis. Only a semicolon is allowed.");
    }
}

void ModelParser::ParseDeleteModel(Tokenizer &tokenizer, std::unique_ptr<QueryStatement> &statement) {
    Token token = tokenizer.NextToken();
    std::string value = StringUtil::Upper(token.value);
    if (token.type != TokenType::KEYWORD || value != "MODEL") {
        throw std::runtime_error("Unknown keyword: " + token.value);
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
    if (token.type != TokenType::STRING_LITERAL || token.value.empty()) {
        throw std::runtime_error("Expected non-empty string literal for provider_name.");
    }
    std::string provider_name = token.value;

    token = tokenizer.NextToken();
    if (token.type != TokenType::SYMBOL || token.value != ",") {
        throw std::runtime_error("Expected comma ',' after provider_name.");
    }

    token = tokenizer.NextToken();
    if (token.type != TokenType::NUMBER || token.value.empty()) {
        throw std::runtime_error("Expected integer value for new context_window.");
    }
    int new_context_window = std::stoi(token.value);

    token = tokenizer.NextToken();
    if (token.type != TokenType::SYMBOL || token.value != ",") {
        throw std::runtime_error("Expected comma ',' after context_window.");
    }

    token = tokenizer.NextToken();
    if (token.type != TokenType::NUMBER || token.value.empty()) {
        throw std::runtime_error("Expected integer value for new max_output_tokens.");
    }
    int new_max_output_tokens = std::stoi(token.value);

    token = tokenizer.NextToken();
    if (token.type != TokenType::PARENTHESIS || token.value != ")") {
        throw std::runtime_error("Expected closing parenthesis ')' after new max_output_tokens.");
    }

    token = tokenizer.NextToken();
    if (token.type == TokenType::END_OF_FILE) {
        auto update_statement = std::make_unique<UpdateModelStatement>();
        update_statement->new_model = new_model;
        update_statement->model_name = model_name;
        update_statement->provider_name = provider_name;
        update_statement->new_context_window = new_context_window;
        update_statement->new_max_output_tokens = new_max_output_tokens;
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
        sql << "INSERT INTO flockmtl_config.FLOCKMTL_MODEL_USER_DEFINED_INTERNAL_TABLE(model_name, model, "
               "provider_name, context_window, max_output_tokens) VALUES ('"
            << create_stmt.model_name << "', '" << create_stmt.model << "', '" << create_stmt.provider_name << "', '"
            << create_stmt.context_window << "', '" << create_stmt.max_output_tokens << "');";
        break;
    }
    case StatementType::DELETE_MODEL: {
        const auto &delete_stmt = static_cast<const DeleteModelStatement &>(statement);
        sql << "DELETE FROM flockmtl_config.FLOCKMTL_MODEL_USER_DEFINED_INTERNAL_TABLE WHERE model_name = '"
            << delete_stmt.model_name << "';";
        break;
    }
    case StatementType::UPDATE_MODEL: {
        const auto &update_stmt = static_cast<const UpdateModelStatement &>(statement);
        sql << "UPDATE flockmtl_config.FLOCKMTL_MODEL_USER_DEFINED_INTERNAL_TABLE SET "
            << "model = '" << update_stmt.new_model << "', "
            << "provider_name = '" << update_stmt.provider_name << "', "
            << "context_window = " << update_stmt.new_context_window << ", "
            << "max_output_tokens = " << update_stmt.new_max_output_tokens << ", "
            << "WHERE model_name = '" << update_stmt.model_name << "';";
        break;
    }
    case StatementType::GET_MODEL: {
        const auto &get_stmt = static_cast<const GetModelStatement &>(statement);
        sql << "SELECT * FROM flockmtl_config.FLOCKMTL_MODEL_DEFAULT_INTERNAL_TABLE WHERE model_name = '"
            << get_stmt.model_name << "'"
            << " UNION ALL "
            << "SELECT * FROM flockmtl_config.FLOCKMTL_MODEL_USER_DEFINED_INTERNAL_TABLE WHERE model_name = '"
            << get_stmt.model_name << "'"
            << ";";
        break;
    }

    case StatementType::GET_ALL_MODEL: {
        sql << "SELECT * FROM flockmtl_config.FLOCKMTL_MODEL_DEFAULT_INTERNAL_TABLE "
            << "UNION ALL "
            << "SELECT * FROM flockmtl_config.FLOCKMTL_MODEL_USER_DEFINED_INTERNAL_TABLE;";
        break;
    }
    default:
        throw std::runtime_error("Unknown statement type.");
    }

    return sql.str();
}

} // namespace core

} // namespace flockmtl
