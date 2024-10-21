#ifndef TIKTOKEN_HPP
#define TIKTOKEN_HPP
#include "flockmtl/common.hpp"

namespace flockmtl {

namespace core {

class Tiktoken {
public:
    static int GetNumTokens(const std::string &str);
};

} // namespace core

} // namespace flockmtl

#endif // TIKTOKEN_HPP
