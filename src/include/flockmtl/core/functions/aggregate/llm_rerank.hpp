#pragma once
#include "flockmtl/common.hpp"

#include <nlohmann/json.hpp>
#include "flockmtl/model_manager/model.hpp"
#include "flockmtl/prompt_manager/prompt_manager.hpp"

namespace flockmtl {
namespace core {

class LlmReranker {
public:
    LlmReranker(Model &model, std::string &user_prompt);

    nlohmann::json SlidingWindowRerank(nlohmann::json &tuples);

private:
    Model &model;
    std::string search_query;
    std::string llm_reranking_template;
    int available_tokens;
    AggregateFunctionType function_type;

    int CalculateFixedTokens() const;

    vector<int> LlmRerankWithSlidingWindow(const nlohmann::json &tuples);
};

} // namespace core

} // namespace flockmtl
