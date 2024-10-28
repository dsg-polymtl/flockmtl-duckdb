#include <nlohmann/json.hpp>
#include <inja/inja.hpp>
#include <flockmtl/core/functions/prompt_builder.hpp>
#include "flockmtl/core/module.hpp"
#include "templates/llm_rerank_prompt_template.hpp"
#include <flockmtl/common.hpp>
#include <flockmtl/core/functions/aggregate.hpp>
#include <flockmtl/core/model_manager/model_manager.hpp>
#include <flockmtl_extension.hpp>
#include <flockmtl/core/model_manager/tiktoken.hpp>

namespace flockmtl {
namespace core {

struct LlmRerankState {
public:
    vector<nlohmann::json> value;

    void Initialize() {
    }

    void Update(const nlohmann::json &input) {
        value.push_back(input);
    }

    void Combine(const LlmRerankState &source) {
        value = std::move(source.value);
    }
};

class LlmReranker {
public:
    std::string model;
    int model_context_size;
    std::string user_prompt;
    std::string llm_reranking_template;
    int batch_size;

    // Constructor
    LlmReranker(std::vector<nlohmann::json>& tuples, std::string& model, int model_context_size, std::string& user_prompt, std::string& llm_reranking_template)
    : model(model), model_context_size(model_context_size), user_prompt(user_prompt), llm_reranking_template(llm_reranking_template) {


        // Calculate fixed token size
        int fixed_tokens = 0;
        fixed_tokens += Tiktoken::GetNumTokens(user_prompt);
        fixed_tokens += Tiktoken::GetNumTokens(llm_reranking_template);

        if (fixed_tokens > model_context_size) {
            throw std::runtime_error("Fixed tokens exceed model context size");
        }

        // Determine the maximum batch size based on available tokens
        auto max_size_row = GetMaxLengthValues(tuples);
        int max_tokens_per_row = Tiktoken::GetNumTokens(max_size_row.dump());

        int available_tokens = model_context_size - fixed_tokens;
        int max_batch_size = available_tokens / max_tokens_per_row;
        batch_size = std::min(max_batch_size, static_cast<int>(tuples.size()));
    };

    // Mock function to simulate LLM ranking by returning indices sorted in some order
    nlohmann::json LlmRerank(const nlohmann::json& tuples) {
        inja::Environment env;
        nlohmann::json data;
        data["tuples"] = tuples;
        data["user_prompt"] = user_prompt;
        auto prompt = env.render(llm_reranking_template, data);

        nlohmann::json settings;
        auto response = ModelManager::CallComplete(prompt, model, settings);
        return response["ranking"];
    };

    // Sliding window re-ranking
    nlohmann::json SlidingWindowRerank(std::vector<nlohmann::json>& tuples) {
        int num_tuples = tuples.size();
        int half_batch = batch_size / 2;
        int start_index = num_tuples - batch_size;

        auto reranked_tuples = nlohmann::json::array();
        auto window_tuples = nlohmann::json::array();

        // Initialize the first window with the last `batch_size` elements
        nlohmann::json tuple;
        for (int i = start_index; i < start_index + batch_size; i++) {
            tuple["id"] = i;
            tuple["content"] = tuples[i];
            window_tuples.push_back(tuple);
        }

        // Process each sliding window
        while (start_index >= 0) {
            // Step 1: Rank the tuples in the current window
            nlohmann::json ranked_indices = LlmRerank(window_tuples);

            // Step 2: Append the last `m/2` ranked tuples to the output
            for (int i = batch_size - half_batch; i < batch_size; ++i) {
                int idx = ranked_indices[i];
                reranked_tuples.push_back(tuples[idx]);
            }

            // Step 3: Prepare the first `m/2` ranked tuples for the next window
            auto next_window = nlohmann::json::array();
            for (int i = 0; i < half_batch; ++i) {
                tuple["id"] = ranked_indices[i];
                tuple["content"] = tuples[ranked_indices[i]];
                next_window.push_back(tuple);
            }

            // Step 4: Add the next `m/2` elements from the input after the last window
            int next_start = start_index - half_batch;
            for (int i = next_start; i < next_start + half_batch && i >= 0; ++i) {
                tuple["id"] = i;
                tuple["content"] = tuples[i];
                next_window.push_back(tuple);
            }

            // Update for the next loop iteration
            window_tuples = std::move(next_window);
            start_index -= half_batch;
        }

        return reranked_tuples;
    }
};

struct LlmRerankOperation {

    static std::string model_name;
    static std::string prompt_name;
    static std::unordered_map<void*, std::shared_ptr<LlmRerankState>> state_map;

    static void Initialize(const AggregateFunction &, data_ptr_t state_p) {
        auto state_ptr = reinterpret_cast<LlmRerankState *>(state_p);

        if (state_map.find(state_ptr) == state_map.end()) {
            auto state = std::make_shared<LlmRerankState>();
            state->Initialize();
            state_map[state_ptr] = state;
        }
    }

    static void Operation(Vector inputs[], AggregateInputData &aggr_input_data, idx_t input_count, Vector &states,
                          idx_t count) {
        prompt_name = inputs[0].GetValue(0).ToString();
        model_name = inputs[1].GetValue(0).ToString();

        if (inputs[2].GetType().id() != LogicalTypeId::STRUCT) {
            throw std::runtime_error("Expected a struct type for prompt inputs");
        }
        auto tuples = StructToJson(inputs[2], count);

        auto states_vector = FlatVector::GetData<LlmRerankState *>(states);

        for (idx_t i = 0; i < count; i++) {
            auto tuple = tuples[i];
            auto state_ptr = states_vector[i];

            auto state = state_map[state_ptr];
            state->Update(tuple);
        }
    }

    static void Combine(Vector &source, Vector &target, AggregateInputData &aggr_input_data, idx_t count) {
        auto source_vector = FlatVector::GetData<LlmRerankState *>(source);
        auto target_vector = FlatVector::GetData<LlmRerankState *>(target);

        for (idx_t i = 0; i < count; i++) {
            auto source_ptr = source_vector[i];
            auto target_ptr = target_vector[i];

            auto source_state = state_map[source_ptr];
            auto target_state = state_map[target_ptr];

            target_state->Combine(*source_state);
        }
    }

    static void Finalize(Vector &states, AggregateInputData &aggr_input_data, Vector &result, idx_t count,
                         idx_t offset) {
        auto states_vector = FlatVector::GetData<LlmRerankState *>(states);

        for (idx_t i = 0; i < count; i++) {
            auto idx = i + offset;
            auto state_ptr = states_vector[idx];
            auto state = state_map[state_ptr];

            auto query_result = CoreModule::GetConnection().Query(
                "SELECT model, max_tokens FROM flockmtl_config.FLOCKMTL_MODEL_INTERNAL_TABLE WHERE model_name = '" +
                model_name + "'");

            if (query_result->RowCount() == 0) {
                throw std::runtime_error("Model not found");
            }

            auto model = query_result->GetValue(0, 0).ToString();
            auto model_context_size = query_result->GetValue(1, 0).GetValue<int>();

            query_result = CoreModule::GetConnection().Query("SELECT prompt FROM flockmtl_config.FLOCKMTL_PROMPT_INTERNAL_TABLE WHERE prompt_name = '" +
                  prompt_name + "'");

            if (query_result->RowCount() == 0) {
                throw std::runtime_error("Prompt not found");
            }

            auto user_prompt = query_result->GetValue(0, 0).ToString();
            auto llm_rerank_prompt_template_str = std::string(llm_rerank_prompt_template);
            LlmReranker llm_reranker(state->value, model, model_context_size, user_prompt, llm_rerank_prompt_template_str);
            auto reranked_tuples = llm_reranker.SlidingWindowRerank(state->value);
            result.SetValue(idx, reranked_tuples.dump());
        }
    }

    static void SimpleUpdate(Vector inputs[], AggregateInputData &aggr_input_data, idx_t input_count,
                             data_ptr_t state_p, idx_t count) {
        prompt_name = inputs[0].GetValue(0).ToString();
        model_name = inputs[1].GetValue(0).ToString();
        auto tuples = StructToJson(inputs[2], count);

        auto state_map_p = reinterpret_cast<LlmRerankState *>(state_p);

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

// Static member initialization
std::string LlmRerankOperation::model_name;
std::string LlmRerankOperation::prompt_name;
std::unordered_map<void*, std::shared_ptr<LlmRerankState>> LlmRerankOperation::state_map;

void CoreAggregateFunctions::RegisterLlmRerankFunction(DatabaseInstance &db) {
    auto string_concat = AggregateFunction(
        "llm_rerank", {LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::ANY},
        LogicalType::JSON(), AggregateFunction::StateSize<LlmRerankState>, LlmRerankOperation::Initialize,
        LlmRerankOperation::Operation, LlmRerankOperation::Combine, LlmRerankOperation::Finalize, LlmRerankOperation::SimpleUpdate);

    ExtensionUtil::RegisterFunction(db, string_concat);
}

} // namespace core
} // namespace flockmtl