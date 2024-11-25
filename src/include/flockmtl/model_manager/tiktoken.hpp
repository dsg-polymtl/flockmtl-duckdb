#pragma once

#include "flockmtl/core/common.hpp"

namespace flockmtl {

class Tiktoken {
public:
    static int GetNumTokens(const std::string& str);
};

} // namespace flockmtl
