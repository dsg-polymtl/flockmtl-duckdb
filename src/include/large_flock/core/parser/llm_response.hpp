#pragma once
#include "large_flock/common.hpp"

namespace large_flock {

namespace core {

struct CoreLlmResponseParsers {
    // llm_complete_json response parser that return a vector of strings
    static std::vector<std::string> LlmCompleteJsonResponseParser(std::string &response, int num_responses);
};

} // namespace core

} // namespace large_flock
