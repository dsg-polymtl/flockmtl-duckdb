#include "flockmtl/custom_parser/query_parser.hpp"

#include "flockmtl/core/common.hpp"

#include <sstream>
#include <stdexcept>

namespace flockmtl {

std::string QueryParser::ParseQuery(const std::string& query) {
    Tokenizer tokenizer(query);

    Token token = tokenizer.NextToken();
    std::string value = duckdb::StringUtil::Upper(token.value);
    if (token.type != TokenType::KEYWORD ||
        (value != "CREATE" && value != "DELETE" && value != "UPDATE" && value != "GET")) {
        throw std::runtime_error(duckdb_fmt::format("Unknown keyword: {}", token.value));
    }

    token = tokenizer.NextToken();
    value = duckdb::StringUtil::Upper(token.value);
    if (token.type == TokenType::KEYWORD && value == "MODEL" || value == "MODELS") {
        ModelParser model_parser;
        model_parser.Parse(query, statement);
        return model_parser.ToSQL(*statement);
    } else if (token.type == TokenType::KEYWORD && ((value == "PROMPT" || value == "PROMPTS"))) {
        PromptParser prompt_parser;
        prompt_parser.Parse(query, statement);
        return prompt_parser.ToSQL(*statement);
    } else {
        throw std::runtime_error(duckdb_fmt::format("Unknown keyword: {}", token.value));
    }
}

} // namespace flockmtl
