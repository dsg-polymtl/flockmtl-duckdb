#pragma once
#include "flockmtl/common.hpp"
#include <nlohmann/json.hpp>

namespace flockmtl {
namespace core {

class LlmReranker {
public:
    string model;
    int model_context_size;
    string user_prompt;
    string llm_reranking_template;
    int batch_size;

    LlmReranker(vector<nlohmann::json>& tuples, string& model, int model_context_size, string& user_prompt, string& llm_reranking_template);

    vector<int> LlmRerank(vector<pair<int, nlohmann::json>> tuples);

    vector<nlohmann::json> SlidingWindowRerank(vector<nlohmann::json>& tuples);

};

} // namespace core

} // namespace flockmtl
