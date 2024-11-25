#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include "flockmtl/core/common.hpp"
#include "flockmtl/prompt_manager/prompt_manager.hpp"
#include "flockmtl/model_manager/model.hpp"

namespace flockmtl {

struct LlmAggState {
public:
    std::vector<nlohmann::json> value;

    void Initialize();
    void Update(const nlohmann::json& input);
    void Combine(const LlmAggState& source);
};

class LlmFirstOrLast {
public:
    Model& model;
    std::string search_query;
    std::string llm_first_or_last_template;
    int available_tokens;
    AggregateFunctionType function_type;

    LlmFirstOrLast(Model& model, std::string& search_query, AggregateFunctionType function_type);

    int GetFirstOrLastTupleId(const nlohmann::json& tuples);
    nlohmann::json Evaluate(nlohmann::json& tuples);

private:
    int calculateFixedTokens() const;
};

struct LlmAggOperation {
    static Model& model;
    static std::string search_query;
    static std::unordered_map<void*, std::shared_ptr<LlmAggState>> state_map;

    static void Initialize(const duckdb::AggregateFunction&, duckdb::data_ptr_t state_p);

    static void Operation(duckdb::Vector inputs[], duckdb::AggregateInputData& aggr_input_data, idx_t input_count,
                          duckdb::Vector& states, idx_t count);

    static void Combine(duckdb::Vector& source, duckdb::Vector& target, duckdb::AggregateInputData& aggr_input_data,
                        idx_t count);

    static void FinalizeResults(duckdb::Vector& states, duckdb::AggregateInputData& aggr_input_data,
                                duckdb::Vector& result, idx_t count, idx_t offset, AggregateFunctionType function_type);

    template <AggregateFunctionType option>
    static void FirstOrLastFinalize(duckdb::Vector& states, duckdb::AggregateInputData& aggr_input_data,
                                    duckdb::Vector& result, idx_t count, idx_t offset);

    static void RerankerFinalize(duckdb::Vector& states, duckdb::AggregateInputData& aggr_input_data,
                                 duckdb::Vector& result, idx_t count, idx_t offset);

    static void SimpleUpdate(duckdb::Vector inputs[], duckdb::AggregateInputData& aggr_input_data, idx_t input_count,
                             duckdb::data_ptr_t state_p, idx_t count);

    static bool IgnoreNull();
};

} // namespace flockmtl
