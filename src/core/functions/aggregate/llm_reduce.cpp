#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include <flockmtl/common.hpp>
#include <flockmtl/core/config/config.hpp>
#include <flockmtl/core/functions/aggregate.hpp>
#include <flockmtl/core/functions/aggregate/llm_agg.hpp>
#include <flockmtl/core/functions/prompt_builder.hpp>
#include <templates/llm_reduce_prompt_template.hpp>

namespace flockmtl {
namespace core {

class LlmReduce {
public:
    std::string model;
    int model_context_size;
    std::string reduce_query;
    std::string llm_reduce_template;
    int available_tokens;
    ModelDetails model_details;

    int calculateFixedTokens() const {
        int num_tokens_meta_and_reduce_query = 0;
        num_tokens_meta_and_reduce_query += Tiktoken::GetNumTokens(reduce_query);
        num_tokens_meta_and_reduce_query += Tiktoken::GetNumTokens(llm_reduce_template);
        return num_tokens_meta_and_reduce_query;
    }

    LlmReduce(std::string &model, int model_context_size, std::string &reduce_query, std::string &llm_reduce_template,
              ModelDetails model_details)
        : model(model), model_context_size(model_context_size), reduce_query(reduce_query),
          llm_reduce_template(llm_reduce_template), model_details(model_details) {

        auto num_tokens_meta_and_reduce_query = calculateFixedTokens();

        if (num_tokens_meta_and_reduce_query > model_context_size) {
            throw std::runtime_error("Fixed tokens exceed model context size");
        }

        available_tokens = model_context_size - num_tokens_meta_and_reduce_query;
    }

    nlohmann::json Reduce(nlohmann::json &tuples) {
        inja::Environment env;
        nlohmann::json data;
        data["tuples"] = tuples;
        data["reduce_query"] = reduce_query;
        auto prompt = env.render(llm_reduce_template, data);

        auto response = ModelManager::CallComplete(prompt, model_details);

        return response["output"];
    };

    nlohmann::json ReduceLoop(vector<nlohmann::json> &tuples) {
        auto accumulated_rows_tokens = 0u;
        auto window_tuples = nlohmann::json::array();
        int signed start_index = tuples.size() - 1;

        do {
            accumulated_rows_tokens = Tiktoken::GetNumTokens(window_tuples.dump());
            while (available_tokens - accumulated_rows_tokens > 0 && start_index >= 0) {
                auto num_tokens = Tiktoken::GetNumTokens(tuples[start_index].dump());
                if (accumulated_rows_tokens + num_tokens > available_tokens) {
                    break;
                }
                window_tuples.push_back(tuples[start_index]);
                accumulated_rows_tokens += num_tokens;
                start_index--;
            }
            auto response = Reduce(window_tuples);
            window_tuples.clear();
            window_tuples.push_back(response);
        } while (start_index >= 0);

        return window_tuples[0];
    }
};

struct LlmReduceOperation {
    static ModelDetails model_details;
    static std::string reduce_query;
    static std::unordered_map<void *, std::shared_ptr<LlmAggState>> state_map;

    static void Initialize(const AggregateFunction &, data_ptr_t state_p) {
        auto state_ptr = reinterpret_cast<LlmAggState *>(state_p);

        if (state_map.find(state_ptr) == state_map.end()) {
            auto state = std::make_shared<LlmAggState>();
            state->Initialize();
            state_map[state_ptr] = state;
        }
    }

    static void Operation(Vector inputs[], AggregateInputData &aggr_input_data, idx_t input_count, Vector &states,
                          idx_t count) {
        reduce_query = inputs[0].GetValue(0).ToString();

        if (inputs[1].GetType().id() != LogicalTypeId::STRUCT) {
            throw std::runtime_error("Expected a struct type for model details");
        }

        auto model_details_json = CastVectorOfStructsToJson(inputs[1], 1)[0];
        LlmReduceOperation::model_details =
            ModelManager::CreateModelDetails(CoreModule::GetConnection(), model_details_json);

        if (inputs[2].GetType().id() != LogicalTypeId::STRUCT) {
            throw std::runtime_error("Expected a struct type for prompt inputs");
        }
        auto tuples = CastVectorOfStructsToJson(inputs[2], count);

        auto states_vector = FlatVector::GetData<LlmAggState *>(states);

        for (idx_t i = 0; i < count; i++) {
            auto tuple = tuples[i];
            auto state_ptr = states_vector[i];

            auto state = state_map[state_ptr];
            state->Update(tuple);
        }
    }

    static void Combine(Vector &source, Vector &target, AggregateInputData &aggr_input_data, idx_t count) {
        auto source_vector = FlatVector::GetData<LlmAggState *>(source);
        auto target_vector = FlatVector::GetData<LlmAggState *>(target);

        for (auto i = 0; i < count; i++) {
            auto source_ptr = source_vector[i];
            auto target_ptr = target_vector[i];

            auto source_state = state_map[source_ptr];
            auto target_state = state_map[target_ptr];

            auto template_str = string(llm_reduce_prompt_template);
            LlmReduce llm_reduce(LlmReduceOperation::model_details.model, Config::default_context_window, reduce_query,
                                 template_str, LlmReduceOperation::model_details);
            auto result = llm_reduce.ReduceLoop(source_state->value);
            target_state->Update(result);
        }
    }

    static void Finalize(Vector &states, AggregateInputData &aggr_input_data, Vector &result, idx_t count,
                         idx_t offset) {
        auto states_vector = FlatVector::GetData<LlmAggState *>(states);

        for (idx_t i = 0; i < count; i++) {
            auto idx = i + offset;
            auto state_ptr = states_vector[idx];
            auto state = state_map[state_ptr];

            auto template_str = string(llm_reduce_prompt_template);
            LlmReduce llm_reduce(LlmReduceOperation::model_details.model, Config::default_context_window, reduce_query,
                                 template_str, LlmReduceOperation::model_details);
            auto response = llm_reduce.ReduceLoop(state->value);
            result.SetValue(idx, response.dump());
        }
    }

    static void SimpleUpdate(Vector inputs[], AggregateInputData &aggr_input_data, idx_t input_count,
                             data_ptr_t state_p, idx_t count) {
        reduce_query = inputs[0].GetValue(0).ToString();
        auto model_details_json = CastVectorOfStructsToJson(inputs[1], 1)[0];
        LlmReduceOperation::model_details =
            ModelManager::CreateModelDetails(CoreModule::GetConnection(), model_details_json);

        auto tuples = CastVectorOfStructsToJson(inputs[2], count);

        auto state_map_p = reinterpret_cast<LlmAggState *>(state_p);

        for (idx_t i = 0; i < count; i++) {
            auto tuple = tuples[i];
            auto state = state_map[state_map_p];
            state->Update(tuple);
        }
    }

    static bool IgnoreNull() {
        return true;
    }
};

ModelDetails LlmReduceOperation::model_details;
std::string LlmReduceOperation::reduce_query;
std::unordered_map<void *, std::shared_ptr<LlmAggState>> LlmReduceOperation::state_map;

void CoreAggregateFunctions::RegisterLlmReduceFunction(DatabaseInstance &db) {
    auto string_concat = AggregateFunction(
        "llm_reduce", {LogicalType::VARCHAR, LogicalType::ANY, LogicalType::ANY}, LogicalType::VARCHAR,
        AggregateFunction::StateSize<LlmAggState>, LlmReduceOperation::Initialize, LlmReduceOperation::Operation,
        LlmReduceOperation::Combine, LlmReduceOperation::Finalize, LlmReduceOperation::SimpleUpdate);

    ExtensionUtil::RegisterFunction(db, string_concat);
}

} // namespace core
} // namespace flockmtl