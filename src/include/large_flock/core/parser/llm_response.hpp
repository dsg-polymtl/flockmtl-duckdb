#pragma once
#include "large_flock/common.hpp"

namespace large_flock {

namespace core {

struct CoreLlmResponseParsers {
    // llm_map response parser that return a vector of strings
    static std::vector<std::string> LlmMapResponseParser(std::string &response, int num_responses);
};

} // namespace core

} // namespace large_flock
