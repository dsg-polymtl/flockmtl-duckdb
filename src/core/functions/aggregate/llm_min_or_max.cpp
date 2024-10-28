#include <flockmtl/core/functions/aggregate/llm_min_or_max.hpp>
#include <flockmtl/core/functions/prompt_builder.hpp>
#include <templates/llm_min_or_max_prompt_template.hpp>
#include "flockmtl/core/module.hpp"
#include "flockmtl/core/model_manager/model_manager.hpp"
#include "flockmtl/core/model_manager/tiktoken.hpp"

namespace flockmtl {
namespace core {

void LlmMinOrMaxState::Initialize() {
}

void LlmMinOrMaxState::Update(const nlohmann::json &input) {
    value.push_back(input);
}

void LlmMinOrMaxState::Combine(const LlmMinOrMaxState &source) {
    value = std::move(source.value);
}

LlmMinOrMax::LlmMinOrMax(std::string &model, int model_context_size, std::string &user_prompt,
                         std::string &llm_min_or_max_template)
    : model(model), model_context_size(model_context_size), user_prompt(user_prompt),
      llm_min_or_max_template(llm_min_or_max_template) {

    int fixed_tokens = calculateFixedTokens();

    if (fixed_tokens > model_context_size) {
        throw std::runtime_error("Fixed tokens exceed model context size");
    }

    available_tokens = model_context_size - fixed_tokens;
}

int LlmMinOrMax::calculateFixedTokens() const {
    int fixed_tokens = 0;
    fixed_tokens += Tiktoken::GetNumTokens(user_prompt);
    fixed_tokens += Tiktoken::GetNumTokens(llm_min_or_max_template);
    return fixed_tokens;
}

nlohmann::json LlmMinOrMax::GetElement(const nlohmann::json &tuples) {
    inja::Environment env;
    nlohmann::json data;
    data["tuples"] = tuples;
    data["user_prompt"] = user_prompt;
    auto prompt = env.render(llm_min_or_max_template, data);

    nlohmann::json settings;
    auto response = ModelManager::CallComplete(prompt, model, settings);
    return response["selected"];
}

nlohmann::json LlmMinOrMax::LlmMinOrMaxCall(nlohmann::json &tuples) {

    if (tuples.size() == 1) {
        return tuples[0]["content"];
    }

    auto num_tuples = tuples.size();
    std::vector<int> batch_size_list;
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
    return LlmMinOrMaxCall(responses);
}

// Static member initialization
std::string LlmMinOrMaxOperation::model_name;
std::string LlmMinOrMaxOperation::prompt_name;
std::unordered_map<void *, std::shared_ptr<LlmMinOrMaxState>> LlmMinOrMaxOperation::state_map;


void LlmMinOrMaxOperation::Initialize(const AggregateFunction &, data_ptr_t state_p) {
    auto state_ptr = reinterpret_cast<LlmMinOrMaxState *>(state_p);

    if (state_map.find(state_ptr) == state_map.end()) {
        auto state = std::make_shared<LlmMinOrMaxState>();
        state->Initialize();
        state_map[state_ptr] = state;
    }
}

void LlmMinOrMaxOperation::Operation(Vector inputs[], AggregateInputData &aggr_input_data, idx_t input_count, Vector &states,
                      idx_t count) {
    prompt_name = inputs[0].GetValue(0).ToString();
    model_name = inputs[1].GetValue(0).ToString();

    if (inputs[2].GetType().id() != LogicalTypeId::STRUCT) {
        throw std::runtime_error("Expected a struct type for prompt inputs");
    }
    auto tuples = StructToJson(inputs[2], count);

    auto states_vector = FlatVector::GetData<LlmMinOrMaxState *>(states);

    for (idx_t i = 0; i < count; i++) {
        auto tuple = tuples[i];
        auto state_ptr = states_vector[i];

        auto state = state_map[state_ptr];
        state->Update(tuple);
    }
}

void LlmMinOrMaxOperation::Combine(Vector &source, Vector &target, AggregateInputData &aggr_input_data, idx_t count) {
    auto source_vector = FlatVector::GetData<LlmMinOrMaxState *>(source);
    auto target_vector = FlatVector::GetData<LlmMinOrMaxState *>(target);

    for (idx_t i = 0; i < count; i++) {
        auto source_ptr = source_vector[i];
        auto target_ptr = target_vector[i];

        auto source_state = state_map[source_ptr];
        auto target_state = state_map[target_ptr];

        target_state->Combine(*source_state);
    }
}

void LlmMinOrMaxOperation::FinalizeResults(Vector &states, AggregateInputData &aggr_input_data, Vector &result, idx_t count,
                     idx_t offset, string llm_prompt_template) {
    auto states_vector = FlatVector::GetData<LlmMinOrMaxState *>(states);

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

        LlmMinOrMax llm_min_or_max(model, model_context_size, prompt_name, llm_prompt_template);

        auto tuples_with_ids = nlohmann::json::array();
        for (auto i = 0; i < state->value.size(); i++) {
            auto tuple_with_id = nlohmann::json::object();
            tuple_with_id["id"] = i;
            tuple_with_id["content"] = state->value[i];
            tuples_with_ids.push_back(tuple_with_id);
        }

        auto response = llm_min_or_max.LlmMinOrMaxCall(tuples_with_ids);
        result.SetValue(idx, response.dump());
    }
}

template <>
void LlmMinOrMaxOperation::Finalize<0>(Vector &states, AggregateInputData &aggr_input_data, Vector &result, idx_t count,
                     idx_t offset) {
    FinalizeResults(states, aggr_input_data, result, count, offset, GetMinOrMaxPromptTemplate<0>());
};

template <>
void LlmMinOrMaxOperation::Finalize<1>(Vector &states, AggregateInputData &aggr_input_data, Vector &result, idx_t count,
                     idx_t offset) {
    FinalizeResults(states, aggr_input_data, result, count, offset, GetMinOrMaxPromptTemplate<1>());
};

void LlmMinOrMaxOperation::SimpleUpdate(Vector inputs[], AggregateInputData &aggr_input_data, idx_t input_count,
                         data_ptr_t state_p, idx_t count) {
    prompt_name = inputs[0].GetValue(0).ToString();
    model_name = inputs[1].GetValue(0).ToString();
    auto tuples = StructToJson(inputs[2], count);

    auto state_map_p = reinterpret_cast<LlmMinOrMaxState *>(state_p);

    for (idx_t i = 0; i < count; i++) {
        auto tuple = tuples[i];
        auto state = state_map[state_map_p];
        state->Update(tuple);
    }
}

bool LlmMinOrMaxOperation::IgnoreNull() {
    return true;
}

} // namespace core
} // namespace flockmtl
