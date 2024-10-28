#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include "flockmtl/common.hpp"
#include "flockmtl/core/module.hpp"

namespace flockmtl {
namespace core {

struct LlmMinOrMaxState {
public:
    std::vector<nlohmann::json> value;

    void Initialize();
    void Update(const nlohmann::json &input);
    void Combine(const LlmMinOrMaxState &source);
};

class LlmMinOrMax {
public:
    std::string model;
    int model_context_size;
    std::string user_prompt;
    std::string llm_min_or_max_template;
    int available_tokens;

    LlmMinOrMax(std::string &model, int model_context_size, std::string &user_prompt,
                std::string &llm_min_or_max_template);

    nlohmann::json GetElement(const nlohmann::json &tuples);
    nlohmann::json LlmMinOrMaxCall(nlohmann::json &tuples);

private:
    int calculateFixedTokens() const;
};

struct LlmMinOrMaxOperation {
    static std::string model_name;
    static std::string prompt_name;
    static std::unordered_map<void *, std::shared_ptr<LlmMinOrMaxState>> state_map;

    static void Initialize(const AggregateFunction &, data_ptr_t state_p);

    static void Operation(Vector inputs[], AggregateInputData &aggr_input_data, idx_t input_count, Vector &states,
                          idx_t count);

    static void Combine(Vector &source, Vector &target, AggregateInputData &aggr_input_data, idx_t count);

    static void FinalizeResults(Vector &states, AggregateInputData &aggr_input_data, Vector &result, idx_t count,
                                idx_t offset, string llm_prompt_template);

    template <int option>
    static void Finalize(Vector &states, AggregateInputData &aggr_input_data, Vector &result, idx_t count,
                         idx_t offset);

    static void SimpleUpdate(Vector inputs[], AggregateInputData &aggr_input_data, idx_t input_count,
                             data_ptr_t state_p, idx_t count);

    static bool IgnoreNull();
};

} // namespace core
} // namespace flockmtl
