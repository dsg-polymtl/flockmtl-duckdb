#include <nlohmann/json.hpp>
#include <flockmtl/core/common.hpp>
#include <flockmtl/core/config.hpp>
#include <flockmtl/functions/aggregate/llm_agg.hpp>
#include <flockmtl/functions/batch_response_builder.hpp>
#include "flockmtl/prompt_manager/prompt_manager.hpp"
#include <flockmtl/model_manager/tiktoken.hpp>
#include "flockmtl/registry/registry.hpp"

namespace flockmtl {

class LlmReduce {
public:
    Model& model;
    std::string reduce_query;
    std::string llm_reduce_template;
    int available_tokens;
    AggregateFunctionType function_type;

    int calculateFixedTokens() const {
        int num_tokens_meta_and_reduce_query = 0;
        num_tokens_meta_and_reduce_query += Tiktoken::GetNumTokens(reduce_query);
        num_tokens_meta_and_reduce_query += Tiktoken::GetNumTokens(llm_reduce_template);
        return num_tokens_meta_and_reduce_query;
    }

    LlmReduce(Model& model, std::string& reduce_query) : model(model), reduce_query(reduce_query) {

        function_type = AggregateFunctionType::REDUCE;
        llm_reduce_template = PromptManager::GetTemplate(function_type);
        auto num_tokens_meta_and_reduce_query = calculateFixedTokens();

        auto model_context_size = model.GetModelDetails().context_window;
        if (num_tokens_meta_and_reduce_query > model_context_size) {
            throw std::runtime_error("Fixed tokens exceed model context size");
        }

        available_tokens = model_context_size - num_tokens_meta_and_reduce_query;
    }

    nlohmann::json Reduce(nlohmann::json& tuples) {
        nlohmann::json data;
        auto prompt = PromptManager::Render(reduce_query, tuples, function_type);
        auto response = model.CallComplete(prompt);
        return response["output"];
    };

    nlohmann::json ReduceLoop(std::vector<nlohmann::json>& tuples) {
        auto accumulated_tuples_tokens = 0u;
        auto batch_tuples = nlohmann::json::array();
        int start_index = 0;

        do {
            accumulated_tuples_tokens = Tiktoken::GetNumTokens(batch_tuples.dump());
            accumulated_tuples_tokens +=
                Tiktoken::GetNumTokens(PromptManager::ConstructMarkdownHeader(tuples[start_index]));
            while (accumulated_tuples_tokens < available_tokens && start_index < tuples.size()) {
                auto num_tokens =
                    Tiktoken::GetNumTokens(PromptManager::ConstructMarkdownSingleTuple(tuples[start_index]));
                if (accumulated_tuples_tokens + num_tokens > available_tokens) {
                    break;
                }
                batch_tuples.push_back(tuples[start_index]);
                accumulated_tuples_tokens += num_tokens;
                start_index++;
            }
            auto response = Reduce(batch_tuples);
            batch_tuples.clear();
            batch_tuples.push_back(response);
            accumulated_tuples_tokens = 0;
        } while (start_index < tuples.size());

        return batch_tuples[0];
    }
};

struct LlmReduceOperation {
    static Model& model;
    static std::string reduce_query;
    static std::unordered_map<void*, std::shared_ptr<LlmAggState>> state_map;

    static void Initialize(const duckdb::AggregateFunction&, duckdb::data_ptr_t state_p) {
        auto state_ptr = reinterpret_cast<LlmAggState*>(state_p);

        if (state_map.find(state_ptr) == state_map.end()) {
            auto state = std::make_shared<LlmAggState>();
            state->Initialize();
            state_map[state_ptr] = state;
        }
    }

    static void Operation(duckdb::Vector inputs[], duckdb::AggregateInputData& aggr_input_data, idx_t input_count,
                          duckdb::Vector& states, idx_t count) {
        if (inputs[0].GetType().id() != duckdb::LogicalTypeId::STRUCT) {
            throw std::runtime_error("Expected a struct type for model details");
        }
        auto model_details_json = CastVectorOfStructsToJson(inputs[0], 1)[0];
        LlmReduceOperation::model = Model(model_details_json);

        if (inputs[1].GetType().id() != duckdb::LogicalTypeId::STRUCT) {
            throw std::runtime_error("Expected a struct type for prompt details");
        }
        auto prompt_details_json = CastVectorOfStructsToJson(inputs[1], 1)[0];
        auto connection = Config::GetConnection();
        reduce_query = PromptManager::CreatePromptDetails(prompt_details_json).prompt;

        if (inputs[2].GetType().id() != duckdb::LogicalTypeId::STRUCT) {
            throw std::runtime_error("Expected a struct type for prompt inputs");
        }
        auto tuples = CastVectorOfStructsToJson(inputs[2], count);

        auto states_vector = duckdb::FlatVector::GetData<LlmAggState*>(states);

        for (idx_t i = 0; i < count; i++) {
            auto tuple = tuples[i];
            auto state_ptr = states_vector[i];

            auto state = state_map[state_ptr];
            state->Update(tuple);
        }
    }

    static void Combine(duckdb::Vector& source, duckdb::Vector& target, duckdb::AggregateInputData& aggr_input_data,
                        idx_t count) {
        auto source_vector = duckdb::FlatVector::GetData<LlmAggState*>(source);
        auto target_vector = duckdb::FlatVector::GetData<LlmAggState*>(target);

        for (auto i = 0; i < count; i++) {
            auto source_ptr = source_vector[i];
            auto target_ptr = target_vector[i];

            auto source_state = state_map[source_ptr];
            auto target_state = state_map[target_ptr];

            LlmReduce llm_reduce(LlmReduceOperation::model, reduce_query);
            auto result = llm_reduce.ReduceLoop(source_state->value);
            target_state->Update(result);
        }
    }

    static void Finalize(duckdb::Vector& states, duckdb::AggregateInputData& aggr_input_data, duckdb::Vector& result,
                         idx_t count, idx_t offset) {
        auto states_vector = duckdb::FlatVector::GetData<LlmAggState*>(states);

        for (idx_t i = 0; i < count; i++) {
            auto idx = i + offset;
            auto state_ptr = states_vector[idx];
            auto state = state_map[state_ptr];

            LlmReduce llm_reduce(LlmReduceOperation::model, reduce_query);
            auto response = llm_reduce.ReduceLoop(state->value);
            result.SetValue(idx, response.dump());
        }
    }

    static void SimpleUpdate(duckdb::Vector inputs[], duckdb::AggregateInputData& aggr_input_data, idx_t input_count,
                             duckdb::data_ptr_t state_p, idx_t count) {
        if (inputs[0].GetType().id() != duckdb::LogicalTypeId::STRUCT) {
            throw std::runtime_error("Expected a struct type for model details");
        }
        auto model_details_json = CastVectorOfStructsToJson(inputs[0], 1)[0];
        LlmReduceOperation::model = Model(model_details_json);

        if (inputs[1].GetType().id() != duckdb::LogicalTypeId::STRUCT) {
            throw std::runtime_error("Expected a struct type for prompt details");
        }
        auto prompt_details_json = CastVectorOfStructsToJson(inputs[1], 1)[0];
        auto connection = Config::GetConnection();
        reduce_query = PromptManager::CreatePromptDetails(prompt_details_json).prompt;

        if (inputs[2].GetType().id() != duckdb::LogicalTypeId::STRUCT) {
            throw std::runtime_error("Expected a struct type for prompt inputs");
        }
        auto tuples = CastVectorOfStructsToJson(inputs[2], count);

        auto state_map_p = reinterpret_cast<LlmAggState*>(state_p);

        for (idx_t i = 0; i < count; i++) {
            auto tuple = tuples[i];
            auto state = state_map[state_map_p];
            state->Update(tuple);
        }
    }

    static bool IgnoreNull() { return true; }
};

Model& LlmReduceOperation::model(*(new Model(nlohmann::json())));
std::string LlmReduceOperation::reduce_query;
std::unordered_map<void*, std::shared_ptr<LlmAggState>> LlmReduceOperation::state_map;

void AggregateRegistry::RegisterLlmReduce(duckdb::DatabaseInstance& db) {
    auto string_concat = duckdb::AggregateFunction(
        "llm_reduce", {duckdb::LogicalType::ANY, duckdb::LogicalType::ANY, duckdb::LogicalType::ANY},
        duckdb::LogicalType::VARCHAR, duckdb::AggregateFunction::StateSize<LlmAggState>, LlmReduceOperation::Initialize,
        LlmReduceOperation::Operation, LlmReduceOperation::Combine, LlmReduceOperation::Finalize,
        LlmReduceOperation::SimpleUpdate);

    duckdb::ExtensionUtil::RegisterFunction(db, string_concat);
}

} // namespace flockmtl
