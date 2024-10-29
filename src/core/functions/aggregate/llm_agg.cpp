#include <flockmtl/core/functions/aggregate/llm_agg.hpp>
#include <flockmtl/core/functions/prompt_builder.hpp>
#include "flockmtl/core/module.hpp"
#include "flockmtl/core/model_manager/model_manager.hpp"
#include "flockmtl/core/model_manager/tiktoken.hpp"

namespace flockmtl {
namespace core {

void LlmAggState::Initialize() {
}

void LlmAggState::Update(const nlohmann::json &input) {
    value.push_back(input);
}

void LlmAggState::Combine(const LlmAggState &source) {
    for (auto &value: source.value){
        Update(std::move(source.value));
    }
}

LlmFirstOrLast::LlmFirstOrLast(std::string &model, int model_context_size, std::string &search_query,
                         std::string &llm_first_or_last_template)
    : model(model), model_context_size(model_context_size), search_query(search_query),
      llm_first_or_last_template(llm_first_or_last_template) {

    auto num_tokens_meta_and_search_query = calculateFixedTokens();

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

nlohmann::json LlmFirstOrLast::GetFirstOrLastTupleId(const nlohmann::json &tuples) {
    inja::Environment env;
    nlohmann::json data;
    data["tuples"] = tuples;
    data["search_query"] = search_query;
    auto prompt = env.render(llm_first_or_last_template, data);

    nlohmann::json settings;
    auto response = ModelManager::CallComplete(prompt, model, "", settings);
    return response["selected"];
}

nlohmann::json LlmFirstOrLast::Evaluate(nlohmann::json &tuples) {
    int num_tuples;
    vector<int> num_tuples_per_batch;
    int num_used_tokens;
    int batch_size;
    int batch_index;

    while (tuples.size() > 1) {
        num_tuples = tuples.size();
        num_used_tokens = 0;
        batch_size = 0;
        batch_index = 0;

        for (int i = 0; i < num_tuples; i++) {
            num_used_tokens += Tiktoken::GetNumTokens(tuples[i].dump());
            batch_size++;

            if (num_used_tokens >= available_tokens) {
                num_tuples_per_batch.push_back(batch_size);
                batch_index++;
                num_used_tokens = 0;
                batch_size = 0;
            } else if (i == num_tuples - 1) {
                num_tuples_per_batch.push_back(batch_size);
                batch_index++;
            }
        }

        auto responses = nlohmann::json::array();
        auto num_batches = batch_index;

        for (auto i = 0; i < num_batches; i++) {
            auto start_index = i * num_tuples_per_batch[i];
            auto end_index = start_index + num_tuples_per_batch[i];
            auto batch = nlohmann::json::array();

            for (auto j = start_index; j < end_index; j++) {
                batch.push_back(tuples[j]);
            }

            auto ranked_indices = GetFirstOrLastTupleId(batch);
            responses.push_back(batch[ranked_indices.get<int>()]);
        }

        tuples = responses;
    };

    return tuples[0]["content"];
}

// Static member initialization
std::string LlmAggOperation::model_name;
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
    search_query = inputs[0].GetValue(0).ToString();
    model_name = inputs[1].GetValue(0).ToString();

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
                     idx_t offset, string llm_prompt_template) {
    auto states_vector = FlatVector::GetData<LlmAggState *>(states);

    for (idx_t i = 0; i < count; i++) {
        auto idx = i + offset;
        auto state_ptr = states_vector[idx];
        auto state = state_map[state_ptr];

        /*
        auto query_result = CoreModule::GetConnection().Query(
            "SELECT model, max_tokens FROM flockmtl_config.FLOCKMTL_MODEL_INTERNAL_TABLE WHERE model_name = '" +
            model_name + "'");

        if (query_result->RowCount() == 0) {
            throw std::runtime_error("Model not found");
        }

        auto model = query_result->GetValue(0, 0).ToString();
        auto model_context_size = query_result->GetValue(1, 0).GetValue<int>();
        */


        auto model_query_result = ModelManager::GetQueriedModel (CoreModule::GetConnection(), model_name, "");
        auto model = model_query_result.first;
        auto model_context_size = model_query_result.second;

        auto tuples_with_ids = nlohmann::json::array();
        for (auto i = 0; i < state->value.size(); i++) {
            auto tuple_with_id = nlohmann::json::object();
            tuple_with_id["id"] = i;
            tuple_with_id["content"] = state->value[i];
            tuples_with_ids.push_back(tuple_with_id);
        }

        LlmFirstOrLast llm_first_or_last(model, model_context_size, search_query, llm_prompt_template);
        auto response = llm_first_or_last.Evaluate(tuples_with_ids);
        result.SetValue(idx, response.dump());
    }
}

template <>
void LlmAggOperation::FirstOrLastFinalize<FirstOrLast::LAST>(Vector &states, AggregateInputData &aggr_input_data, Vector &result, idx_t count,
                     idx_t offset) {
    FinalizeResults(states, aggr_input_data, result, count, offset, GetFirstOrLastPromptTemplate<FirstOrLast::LAST>());
};

template <>
void LlmAggOperation::FirstOrLastFinalize<FirstOrLast::FIRST>(Vector &states, AggregateInputData &aggr_input_data, Vector &result, idx_t count,
                     idx_t offset) {
    FinalizeResults(states, aggr_input_data, result, count, offset, GetFirstOrLastPromptTemplate<FirstOrLast::FIRST>());
};

void LlmAggOperation::SimpleUpdate(Vector inputs[], AggregateInputData &aggr_input_data, idx_t input_count,
                         data_ptr_t state_p, idx_t count) {
    search_query = inputs[0].GetValue(0).ToString();
    model_name = inputs[1].GetValue(0).ToString();
    auto tuples = CastVectorOfStructsToJson(inputs[2], count);

    auto state_map_p = reinterpret_cast<LlmAggState *>(state_p);

    for (idx_t i = 0; i < count; i++) {
        auto tuple = tuples[i];
        auto state = state_map[state_map_p];
        state->Update(tuple);
    }
}

bool LlmAggOperation::IgnoreNull() {
    return true;
}

} // namespace core
} // namespace flockmtl
