#include "flockmtl/core/parser/query_parser.hpp"

#include "flockmtl/common.hpp"

#include <sstream>
#include <stdexcept>

namespace flockmtl {

namespace core {
std::string QueryParser::ParseQuery(const std::string &query) {
    Tokenizer tokenizer(query);

    Token token = tokenizer.NextToken();
    std::string value = StringUtil::Upper(token.value);
    if (token.type != TokenType::KEYWORD ||
        (value != "CREATE" && value != "DELETE" && value != "UPDATE" && value != "GET")) {
        throw std::runtime_error("Unknown keyword: " + token.value);
    }

    token = tokenizer.NextToken();
    value = StringUtil::Upper(token.value);
    if (token.type == TokenType::KEYWORD && value == "MODEL" || value == "MODELS") {
        ModelParser model_parser;
        model_parser.Parse(query, statement);
        return model_parser.ToSQL(*statement);
    } else if (token.type == TokenType::KEYWORD && (value == "PROMPT" || value == "PROMPTS")) {
        PromptParser prompt_parser;
        prompt_parser.Parse(query, statement);
        return prompt_parser.ToSQL(*statement);
    } else {
        throw std::runtime_error("Unknown keyword: " + token.value);
    }
}

} // namespace core

} // namespace flockmtl