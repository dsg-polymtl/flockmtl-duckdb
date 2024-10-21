#include "flockmtl/core/parser/tokenizer.hpp"

#include <cctype>
#include <iostream>
#include <stdexcept>

// Constructor
Tokenizer::Tokenizer(const std::string &query) : query_(query), position_(0) {
}

// Skip whitespace
void Tokenizer::SkipWhitespace() {
    while (position_ < query_.size() && std::isspace(query_[position_])) {
        ++position_;
    }
}

std::string Tokenizer::GetQuery() {
    return query_;
}

// Parse a string literal
Token Tokenizer::ParseStringLiteral() {
    if (query_[position_] != '\'') {
        throw std::runtime_error("String literal should start with a single quote.");
    }
    ++position_;
    int start = position_;
    while (position_ < query_.size() && query_[position_] != '\'') {
        ++position_;
    }
    if (position_ == query_.size()) {
        throw std::runtime_error("Unterminated string literal.");
    }
    std::string value = query_.substr(start, position_ - start);
    ++position_; // Move past the closing quote
    return {TokenType::STRING_LITERAL, value};
}

// Parse a keyword (word made of letters)
Token Tokenizer::ParseKeyword() {
    int start = position_;
    while (position_ < query_.size() && std::isalpha(query_[position_])) {
        ++position_;
    }
    std::string value = query_.substr(start, position_ - start);
    return {TokenType::KEYWORD, value};
}

// Parse a symbol (single character)
Token Tokenizer::ParseSymbol() {
    char ch = query_[position_];
    ++position_;
    return {TokenType::SYMBOL, std::string(1, ch)};
}

// Parse a number (sequence of digits)
Token Tokenizer::ParseNumber() {
    int start = position_;
    while (position_ < query_.size() && std::isdigit(query_[position_])) {
        ++position_;
    }
    std::string value = query_.substr(start, position_ - start);
    return {TokenType::NUMBER, value};
}

// Parse a parenthesis
Token Tokenizer::ParseParenthesis() {
    char ch = query_[position_];
    ++position_;
    return {TokenType::PARENTHESIS, std::string(1, ch)};
}

// Get the next token from the input
Token Tokenizer::GetNextToken() {
    SkipWhitespace();
    if (position_ >= query_.size()) {
        return {TokenType::END_OF_FILE, ""};
    }

    char ch = query_[position_];
    if (ch == '\'') {
        return ParseStringLiteral();
    } else if (std::isalpha(ch)) {
        return ParseKeyword();
    } else if (ch == ';' || ch == ',') {
        return ParseSymbol();
    } else if (ch == '(' || ch == ')') {
        return ParseParenthesis();
    } else if (std::isdigit(ch)) {
        return ParseNumber();
    } else {
        return {TokenType::UNKNOWN, std::string(1, ch)};
    }
}

// Get the next token
Token Tokenizer::NextToken() {
    return GetNextToken();
}

// Convert TokenType to string
std::string TokenTypeToString(TokenType type) {
    switch (type) {
    case TokenType::KEYWORD:
        return "KEYWORD";
    case TokenType::STRING_LITERAL:
        return "STRING_LITERAL";
    case TokenType::SYMBOL:
        return "SYMBOL";
    case TokenType::NUMBER:
        return "NUMBER";
    case TokenType::PARENTHESIS:
        return "PARENTHESIS";
    case TokenType::END_OF_FILE:
        return "END_OF_FILE";
    case TokenType::UNKNOWN:
        return "UNKNOWN";
    default:
        return "UNKNOWN";
    }
}
