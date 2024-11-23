#pragma once

#include "flockmtl/common.hpp"

namespace flockmtl {

class Tiktoken {
public:
    static int GetNumTokens(const std::string &str);
};

} // namespace flockmtl
