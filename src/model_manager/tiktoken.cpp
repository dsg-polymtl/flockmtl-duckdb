#include <regex>

#include "flockmtl/model_manager/tiktoken.hpp"

namespace flockmtl {

int Tiktoken::GetNumTokens(const std::string &str) {
    std::regex word_regex(R"(\w+|[^\w\s])");
    auto words_begin = std::sregex_iterator(str.begin(), str.end(), word_regex);
    auto words_end = std::sregex_iterator();
    return std::distance(words_begin, words_end);
}

} // namespace flockmtl