#pragma once
#include <nlohmann/json.hpp>

#include "flockmtl/core/common.hpp"
#include "flockmtl/model_manager/model.hpp"
#include "flockmtl/prompt_manager/prompt_manager.hpp"

namespace flockmtl {

class LlmReranker {
public:
    LlmReranker(Model& model, std::string& user_prompt);

    nlohmann::json SlidingWindowRerank(nlohmann::json& tuples);

private:
    Model& model;
    std::string search_query;
    std::string llm_reranking_template;
    int available_tokens;
    AggregateFunctionType function_type;

    int CalculateFixedTokens() const;

    std::vector<int> LlmRerankWithSlidingWindow(const nlohmann::json& tuples);
};

} // namespace flockmtl
