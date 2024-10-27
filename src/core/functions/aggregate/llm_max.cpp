#include <nlohmann/json.hpp>
#include <flockmtl/core/functions/prompt_builder.hpp>
#include "flockmtl/core/module.hpp"
#include "templates/llm_max_prompt_template.hpp"
#include <flockmtl/common.hpp>
#include <flockmtl/core/functions/aggregate.hpp>
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

struct LlmMaxOperation {

    static std::string model_name;
    static std::string prompt_name;
    static std::unordered_map<void*, std::shared_ptr<LlmMaxState>> state_map;

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

    static vector<nlohmann::json> LlmAggregate(vector<nlohmann::json> &tuples, std::string model) {

        if (tuples.size() == 1) {
            return tuples;
        }

        auto prompts =
                ConstructPrompts(tuples, CoreModule::GetConnection(), prompt_name, llm_max_prompt_template, 2048);

        vector<nlohmann::json> responses;
        nlohmann::json settings;
        for (const auto &prompt : prompts) {
            auto response = ModelManager::CallComplete(prompt, model, settings);
            if (response.contains("row")) {
                responses.push_back(response["row"]);
            }
        }
        return LlmAggregate(responses, model);
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

            auto response = LlmAggregate(state->value, model)[0];
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
std::unordered_map<void*, std::shared_ptr<LlmMaxState>> LlmMaxOperation::state_map;

void CoreAggregateFunctions::RegisterLlmMaxFunction(DatabaseInstance &db) {
    auto string_concat = AggregateFunction(
        "llm_max", {LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::ANY},
        LogicalType::JSON(), AggregateFunction::StateSize<LlmMaxState>, LlmMaxOperation::Initialize,
        LlmMaxOperation::Operation, LlmMaxOperation::Combine, LlmMaxOperation::Finalize, LlmMaxOperation::SimpleUpdate);

    ExtensionUtil::RegisterFunction(db, string_concat);
}

} // namespace core
} // namespace flockmtl