#pragma once

#include "flockmtl/common.hpp"
#include "flockmtl/core/parser/query/model_parser.hpp"
#include "flockmtl/core/parser/query/prompt_parser.hpp"
#include "flockmtl/core/parser/query_statements.hpp"
#include "flockmtl/core/parser/tokenizer.hpp"

#include <memory>
#include <string>
#include <vector>

namespace flockmtl {
namespace core {
class QueryParser {
public:
    std::string ParseQuery(const std::string &query);

private:
    std::unique_ptr<QueryStatement> statement;
};
} // namespace core
} // namespace flockmtl
