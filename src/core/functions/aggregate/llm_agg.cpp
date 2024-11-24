#include <flockmtl/core/functions/aggregate/llm_agg.hpp>
#include <flockmtl/core/functions/batch_response_builder.hpp>
#include "flockmtl/core/module.hpp"
#include "flockmtl/model_manager/model.hpp"
#include "flockmtl/model_manager/tiktoken.hpp"
#include <flockmtl/core/config/config.hpp>
#include <vector>

namespace flockmtl {
namespace core {

void LlmAggState::Initialize() {}

void LlmAggState::Update(const nlohmann::json &input) { value.push_back(input); }

void LlmAggState::Combine(const LlmAggState &source) {
    for (auto &input : source.value) {
        Update(std::move(input));
    }
}

LlmFirstOrLast::LlmFirstOrLast(Model &model, std::string &search_query, AggregateFunctionType function_type)
    : model(model), search_query(search_query), function_type(function_type) {

    llm_first_or_last_template = PromptManager::GetTemplate(function_type);
    auto num_tokens_meta_and_search_query = calculateFixedTokens();

    auto model_context_size = model.model_details_.context_window;
    if (num_tokens_meta_and_search_query > model_context_size) {
        throw std::runtime_error("Fixed tokens exceed model context size");
    }

    available_tokens = model_context_size - num_tokens_meta_and_search_query;
}

int LlmFirstOrLast::calculateFixedTokens() const {
    int num_tokens_meta_and_search_query = 0;
    num_tokens_meta_and_search_query += Tiktoken::GetNumTokens(search_query);
    num_tokens_meta_and_search_query += Tiktoken::GetNumTokens(llm_first_or_last_template);
    return num_tokens_meta_and_search_query;
}

int LlmFirstOrLast::GetFirstOrLastTupleId(const nlohmann::json &tuples) {
    nlohmann::json data;
    auto markdown_tuples = ConstructMarkdownArrayTuples(tuples);
    auto prompt = PromptManager::Render(search_query, markdown_tuples, function_type);
    auto response = model.CallComplete(prompt);
    return response["selected"].get<int>();
}

nlohmann::json LlmFirstOrLast::Evaluate(nlohmann::json &tuples) {

    auto accumulated_tuples_tokens = 0u;
    auto batch_tuples = nlohmann::json::array();
    int start_index = 0;

    do {
        accumulated_tuples_tokens = Tiktoken::GetNumTokens(batch_tuples.dump());
        accumulated_tuples_tokens += Tiktoken::GetNumTokens(ConstructMarkdownHeader(tuples[start_index]));
        while (accumulated_tuples_tokens < available_tokens && start_index < tuples.size()) {
            auto num_tokens = Tiktoken::GetNumTokens(ConstructMarkdownSingleTuple(tuples[start_index]));
            if (accumulated_tuples_tokens + num_tokens > available_tokens) {
                break;
            }
            batch_tuples.push_back(tuples[start_index]);
            accumulated_tuples_tokens += num_tokens;
            start_index++;
        }
        auto result_idx = GetFirstOrLastTupleId(batch_tuples);
        batch_tuples.clear();
        batch_tuples.push_back(tuples[result_idx]);
    } while (start_index < tuples.size());

    batch_tuples[0].erase("flockmtl_tuple_id");

    return batch_tuples[0];
}

// Static member initialization
Model &LlmAggOperation::model(*(new Model(nlohmann::json())));
std::string LlmAggOperation::search_query;

std::unordered_map<void *, std::shared_ptr<LlmAggState>> LlmAggOperation::state_map;

void LlmAggOperation::Initialize(const AggregateFunction &, data_ptr_t state_p) {
    auto state_ptr = reinterpret_cast<LlmAggState *>(state_p);

    if (state_map.find(state_ptr) == state_map.end()) {
        auto state = std::make_shared<LlmAggState>();
        state->Initialize();
        state_map[state_ptr] = state;
    }
}

void LlmAggOperation::Operation(Vector inputs[], AggregateInputData &aggr_input_data, idx_t input_count, Vector &states,
                                idx_t count) {
    if (inputs[0].GetType().id() != LogicalTypeId::STRUCT) {
        throw std::runtime_error("Expected a struct type for model details");
    }
    auto model_details_json = CastVectorOfStructsToJson(inputs[0], 1)[0];
    LlmAggOperation::model = Model(model_details_json);

    if (inputs[1].GetType().id() != LogicalTypeId::STRUCT) {
        throw std::runtime_error("Expected a struct type for prompt details");
    }
    auto prompt_details_json = CastVectorOfStructsToJson(inputs[1], 1)[0];
    auto connection = CoreModule::GetConnection();
    search_query = CreatePromptDetails(connection, prompt_details_json).prompt;

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

void LlmAggOperation::Combine(Vector &source, Vector &target, AggregateInputData &aggr_input_data, idx_t count) {
    auto source_vector = FlatVector::GetData<LlmAggState *>(source);
    auto target_vector = FlatVector::GetData<LlmAggState *>(target);

    for (auto i = 0; i < count; i++) {
        auto source_ptr = source_vector[i];
        auto target_ptr = target_vector[i];

        auto source_state = state_map[source_ptr];
        auto target_state = state_map[target_ptr];

        target_state->Combine(*source_state);
    }
}

void LlmAggOperation::FinalizeResults(Vector &states, AggregateInputData &aggr_input_data, Vector &result, idx_t count,
                                      idx_t offset, AggregateFunctionType function_type) {
    auto states_vector = FlatVector::GetData<LlmAggState *>(states);

    for (idx_t i = 0; i < count; i++) {
        auto idx = i + offset;
        auto state_ptr = states_vector[idx];
        auto state = state_map[state_ptr];

        auto tuples_with_ids = nlohmann::json::array();
        for (auto i = 0; i < state->value.size(); i++) {
            auto tuple_with_id = state->value[i];
            tuple_with_id["flockmtl_tuple_id"] = i;
            tuples_with_ids.push_back(tuple_with_id);
        }

        LlmFirstOrLast llm_first_or_last(LlmAggOperation::model, search_query, function_type);
        auto response = llm_first_or_last.Evaluate(tuples_with_ids);
        result.SetValue(idx, response.dump());
    }
}

template <>
void LlmAggOperation::FirstOrLastFinalize<AggregateFunctionType::LAST>(Vector &states,
                                                                       AggregateInputData &aggr_input_data,
                                                                       Vector &result, idx_t count, idx_t offset) {
    FinalizeResults(states, aggr_input_data, result, count, offset, AggregateFunctionType::LAST);
};

template <>
void LlmAggOperation::FirstOrLastFinalize<AggregateFunctionType::FIRST>(Vector &states,
                                                                        AggregateInputData &aggr_input_data,
                                                                        Vector &result, idx_t count, idx_t offset) {
    FinalizeResults(states, aggr_input_data, result, count, offset, AggregateFunctionType::FIRST);
};

void LlmAggOperation::SimpleUpdate(Vector inputs[], AggregateInputData &aggr_input_data, idx_t input_count,
                                   data_ptr_t state_p, idx_t count) {
    if (inputs[0].GetType().id() != LogicalTypeId::STRUCT) {
        throw std::runtime_error("Expected a struct type for model details");
    }
    auto model_details_json = CastVectorOfStructsToJson(inputs[0], 1)[0];
    LlmAggOperation::model = Model(model_details_json);

    if (inputs[1].GetType().id() != LogicalTypeId::STRUCT) {
        throw std::runtime_error("Expected a struct type for prompt details");
    }
    auto prompt_details_json = CastVectorOfStructsToJson(inputs[1], 1)[0];
    auto connection = CoreModule::GetConnection();
    search_query = CreatePromptDetails(connection, prompt_details_json).prompt;

    if (inputs[2].GetType().id() != LogicalTypeId::STRUCT) {
        throw std::runtime_error("Expected a struct type for prompt inputs");
    }
    auto tuples = CastVectorOfStructsToJson(inputs[2], count);

    auto state_map_p = reinterpret_cast<LlmAggState *>(state_p);

    for (idx_t i = 0; i < count; i++) {
        auto tuple = tuples[i];
        auto state = state_map[state_map_p];
        state->Update(tuple);
    }
}

bool LlmAggOperation::IgnoreNull() { return true; }

} // namespace core
} // namespace flockmtl
