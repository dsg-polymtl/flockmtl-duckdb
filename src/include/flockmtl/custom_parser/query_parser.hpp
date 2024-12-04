#pragma once

#include "flockmtl/core/common.hpp"
#include "flockmtl/custom_parser/query/model_parser.hpp"
#include "flockmtl/custom_parser/query/prompt_parser.hpp"
#include "flockmtl/custom_parser/query_statements.hpp"
#include "flockmtl/custom_parser/tokenizer.hpp"

#include "fmt/format.h"
#include <memory>
#include <string>
#include <vector>

namespace flockmtl {

class QueryParser {
public:
    std::string ParseQuery(const std::string& query);

private:
    std::unique_ptr<QueryStatement> statement;
};

} // namespace flockmtl
