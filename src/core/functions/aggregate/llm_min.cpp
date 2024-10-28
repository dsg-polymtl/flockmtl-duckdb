#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include "flockmtl/core/module.hpp"
#include "templates/llm_min_prompt_template.hpp"

#include <flockmtl/common.hpp>
#include <flockmtl/core/functions/aggregate.hpp>
#include <flockmtl/core/functions/aggregate/llm_selector.hpp>
#include <flockmtl/core/functions/prompt_builder.hpp>
#include <flockmtl/core/model_manager/model_manager.hpp>
#include <flockmtl_extension.hpp>

namespace flockmtl {
namespace core {

struct LlmMinOperation {

    static std::string model_name;
    static std::string prompt_name;
    static std::unordered_map<void *, std::shared_ptr<LlmSelectorState>> state_map;

    static void Initialize(const AggregateFunction &, data_ptr_t state_p) {
        auto state_ptr = reinterpret_cast<LlmSelectorState *>(state_p);

        if (state_map.find(state_ptr) == state_map.end()) {
            auto state = std::make_shared<LlmSelectorState>();
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

        auto states_vector = FlatVector::GetData<LlmSelectorState *>(states);

        for (idx_t i = 0; i < count; i++) {
            auto tuple = tuples[i];
            auto state_ptr = states_vector[i];

            auto state = state_map[state_ptr];
            state->Update(tuple);
        }
    }

    static void Combine(Vector &source, Vector &target, AggregateInputData &aggr_input_data, idx_t count) {
        auto source_vector = FlatVector::GetData<LlmSelectorState *>(source);
        auto target_vector = FlatVector::GetData<LlmSelectorState *>(target);

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
        auto states_vector = FlatVector::GetData<LlmSelectorState *>(states);

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
            auto llm_min_prompt_template_str = string(llm_min_prompt_template);
            LlmSelector llm_selector(model, model_context_size, prompt_name, llm_min_prompt_template_str);

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

        auto state_map_p = reinterpret_cast<LlmSelectorState *>(state_p);

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
std::string LlmMinOperation::model_name;
std::string LlmMinOperation::prompt_name;
std::unordered_map<void *, std::shared_ptr<LlmSelectorState>> LlmMinOperation::state_map;

void CoreAggregateFunctions::RegisterLlmMinFunction(DatabaseInstance &db) {
    auto string_concat = AggregateFunction(
        "llm_min", {LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::ANY}, LogicalType::JSON(),
        AggregateFunction::StateSize<LlmSelectorState>, LlmMinOperation::Initialize, LlmMinOperation::Operation,
        LlmMinOperation::Combine, LlmMinOperation::Finalize, LlmMinOperation::SimpleUpdate);

    ExtensionUtil::RegisterFunction(db, string_concat);
}

} // namespace core
} // namespace flockmtl