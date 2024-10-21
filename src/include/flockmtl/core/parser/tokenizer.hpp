#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <string>

// Define token types
enum class TokenType { KEYWORD, STRING_LITERAL, SYMBOL, NUMBER, PARENTHESIS, END_OF_FILE, UNKNOWN };

// Token structure
struct Token {
    TokenType type;
    std::string value;
};

// Tokenizer class declaration
class Tokenizer {
public:
    Tokenizer(const std::string &query);
    Token NextToken();
    std::string GetQuery();

private:
    std::string query_;
    int position_;

    void SkipWhitespace();
    Token ParseStringLiteral();
    Token ParseKeyword();
    Token ParseSymbol();
    Token ParseNumber();      // Added for handling numbers
    Token ParseParenthesis(); // Added for handling parentheses
    Token GetNextToken();
};

// Utility function to convert TokenType to string
std::string TokenTypeToString(TokenType type);

#endif // TOKENIZER_HPP
