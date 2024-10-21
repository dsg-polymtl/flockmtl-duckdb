#pragma once
#include "flockmtl/common.hpp"

namespace flockmtl {

namespace core {

struct CoreLlmResponseParsers {
    // llm_complete_json response parser that return a vector of strings
    static std::vector<std::string> LlmCompleteJsonResponseParser(std::string &response, int num_responses);
};

} // namespace core

} // namespace flockmtl
