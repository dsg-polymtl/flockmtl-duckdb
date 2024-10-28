#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include "flockmtl/core/module.hpp"
#include "templates/llm_max_prompt_template.hpp"

#include <flockmtl/common.hpp>
#include <flockmtl/core/functions/aggregate.hpp>
#include <flockmtl/core/functions/prompt_builder.hpp>
#include <flockmtl/core/model_manager/model_manager.hpp>
#include <flockmtl_extension.hpp>

namespace flockmtl {
namespace core {

struct LlmMaxState {
public:
    vector<nlohmann::json> value;

    void Initialize() {
    }

    void Update(const nlohmann::json &input) {
        value.push_back(input);
    }

    void Combine(const LlmMaxState &source) {
        value = std::move(source.value);
    }
};

class LlmSelector {
public:
    std::string model;
    int model_context_size;
    std::string user_prompt;
    std::string llm_selector_template;
    int available_tokens;

    // Constructor
    LlmSelector(std::string &model, int model_context_size, std::string &user_prompt,
                std::string &llm_selector_template)
        : model(model), model_context_size(model_context_size), user_prompt(user_prompt),
          llm_selector_template(llm_selector_template) {

        // Calculate fixed token size
        int fixed_tokens = 0;
        fixed_tokens += Tiktoken::GetNumTokens(user_prompt);
        fixed_tokens += Tiktoken::GetNumTokens(llm_selector_template);

        if (fixed_tokens > model_context_size) {
            throw std::runtime_error("Fixed tokens exceed model context size");
        }

        available_tokens = model_context_size - fixed_tokens;
    };

    // Mock function to simulate LLM ranking by returning indices sorted in some order
    nlohmann::json GetElement(const nlohmann::json &tuples) {
        inja::Environment env;
        nlohmann::json data;
        data["tuples"] = tuples;
        data["user_prompt"] = user_prompt;
        auto prompt = env.render(llm_selector_template, data);

        nlohmann::json settings;
        auto response = ModelManager::CallComplete(prompt, model, settings);
        return response["selected"];
    };

    nlohmann::json LlmSelectorCall(nlohmann::json &tuples) {

        if (tuples.size() == 1) {
            return tuples[0]["content"];
        }

        auto num_tuples = tuples.size();
        vector<int> batch_size_list;
        auto used_tokens = 0;
        auto batch_size = 0;
        for (int i = 0; i < num_tuples; i++) {
            used_tokens += Tiktoken::GetNumTokens(tuples[i].dump());
            batch_size++;
            if (used_tokens >= available_tokens) {
                batch_size_list.push_back(batch_size);
                used_tokens = 0;
                batch_size = 0;
            } else if (i == num_tuples - 1) {
                batch_size_list.push_back(batch_size);
            }
        }

        auto responses = nlohmann::json::array();
        for (int i = 0; i < batch_size_list.size(); i++) {
            auto start_index = i * batch_size;
            auto end_index = start_index + batch_size;
            auto batch = nlohmann::json::array();
            for (int j = start_index; j < end_index; j++) {
                batch.push_back(tuples[j]);
            }
            auto ranked_indices = GetElement(batch);
            responses.push_back(batch[ranked_indices.get<int>()]);
        }
        return LlmSelectorCall(responses);
    };
};

struct LlmMaxOperation {

    static std::string model_name;
    static std::string prompt_name;
    static std::unordered_map<void *, std::shared_ptr<LlmMaxState>> state_map;

    static void Initialize(const AggregateFunction &, data_ptr_t state_p) {
        auto state_ptr = reinterpret_cast<LlmMaxState *>(state_p);

        if (state_map.find(state_ptr) == state_map.end()) {
            auto state = std::make_shared<LlmMaxState>();
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

        auto states_vector = FlatVector::GetData<LlmMaxState *>(states);

        for (idx_t i = 0; i < count; i++) {
            auto tuple = tuples[i];
            auto state_ptr = states_vector[i];

            auto state = state_map[state_ptr];
            state->Update(tuple);
        }
    }

    static void Combine(Vector &source, Vector &target, AggregateInputData &aggr_input_data, idx_t count) {
        auto source_vector = FlatVector::GetData<LlmMaxState *>(source);
        auto target_vector = FlatVector::GetData<LlmMaxState *>(target);

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
        auto states_vector = FlatVector::GetData<LlmMaxState *>(states);

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
            auto llm_max_prompt_template_str = string(llm_max_prompt_template);
            LlmSelector llm_selector(model, model_context_size, prompt_name, llm_max_prompt_template_str);

            auto tuples_with_ids = nlohmann::json::array();
            for (auto i = 0; i < state->value.size(); i++) {
                auto tuple_with_id = nlohmann::json::object();
                tuple_with_id["id"] = i;
                tuple_with_id["content"] = state->value[i];
                tuples_with_ids.push_back(tuple_with_id);
            }

            auto response = llm_selector.LlmSelectorCall(tuples_with_ids);
            result.SetValue(idx, response.dump());
        }
    }

    static void SimpleUpdate(Vector inputs[], AggregateInputData &aggr_input_data, idx_t input_count,
                             data_ptr_t state_p, idx_t count) {
        prompt_name = inputs[0].GetValue(0).ToString();
        model_name = inputs[1].GetValue(0).ToString();
        auto tuples = StructToJson(inputs[2], count);

        auto state_map_p = reinterpret_cast<LlmMaxState *>(state_p);

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
std::string LlmMaxOperation::model_name;
std::string LlmMaxOperation::prompt_name;
std::unordered_map<void *, std::shared_ptr<LlmMaxState>> LlmMaxOperation::state_map;

void CoreAggregateFunctions::RegisterLlmMaxFunction(DatabaseInstance &db) {
    auto string_concat = AggregateFunction(
        "llm_max", {LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::ANY}, LogicalType::JSON(),
        AggregateFunction::StateSize<LlmMaxState>, LlmMaxOperation::Initialize, LlmMaxOperation::Operation,
        LlmMaxOperation::Combine, LlmMaxOperation::Finalize, LlmMaxOperation::SimpleUpdate);

    ExtensionUtil::RegisterFunction(db, string_concat);
}

} // namespace core
} // namespace flockmtl