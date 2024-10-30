#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include "flockmtl/common.hpp"
#include "flockmtl/core/module.hpp"
#include "templates/llm_min_or_max_prompt_template.hpp"

namespace flockmtl {
namespace core {

struct LlmAggState {
public:
    std::vector<nlohmann::json> value;

    void Initialize();
    void Update(const nlohmann::json &input);
    void Combine(const LlmAggState &source);
};

class LlmMinOrMax {
public:
    std::string model;
    int model_context_size;
    std::string search_query;
    std::string llm_min_or_max_template;
    int available_tokens;

    LlmMinOrMax(std::string &model, int model_context_size, std::string &search_query,
                std::string &llm_min_or_max_template);

    nlohmann::json GetMaxOrMinTupleId(const nlohmann::json &tuples);
    nlohmann::json Evaluate(nlohmann::json &tuples);

private:
    int calculateFixedTokens() const;
};

struct LlmAggOperation {
    static std::string model_name;
    static std::string search_query;
    static std::unordered_map<void *, std::shared_ptr<LlmAggState>> state_map;

    static void Initialize(const AggregateFunction &, data_ptr_t state_p);

    static void Operation(Vector inputs[], AggregateInputData &aggr_input_data, idx_t input_count, Vector &states,
                          idx_t count);

    static void Combine(Vector &source, Vector &target, AggregateInputData &aggr_input_data, idx_t count);

    static void FinalizeResults(Vector &states, AggregateInputData &aggr_input_data, Vector &result, idx_t count,
                                idx_t offset, string llm_prompt_template);

    template <MinOrMax option>
    static void MinOrMaxFinalize(Vector &states, AggregateInputData &aggr_input_data, Vector &result, idx_t count,
                         idx_t offset);

    static void RerankerFinalize(Vector &states, AggregateInputData &aggr_input_data, Vector &result, idx_t count,
                         idx_t offset);

    static void SimpleUpdate(Vector inputs[], AggregateInputData &aggr_input_data, idx_t input_count,
                             data_ptr_t state_p, idx_t count);

    static bool IgnoreNull();
};

} // namespace core
} // namespace flockmtl
