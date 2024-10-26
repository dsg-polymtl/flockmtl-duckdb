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
    bool isInitialized;
    shared_ptr<Value> value;

    LlmMaxState() : isInitialized(false), value(make_shared_ptr<Value>(std::move(Value("")))) {
    }

    void Initialize() {
        if (isInitialized) {
            return;
        }
        isInitialized = true;
        value = make_shared_ptr<Value>(std::move(Value("")));
    }

    void Update(const Value &input) {
        if (!isInitialized) {
            Initialize();
        } else {
            // Dynamically allocate larger storage if necessary
            auto input_copy = input;
            value = make_shared_ptr<Value>(std::move(input_copy));
        }
    }

    void Combine(const LlmMaxState &source) {
        if (!source.isInitialized) {
            return;
        }
        if (!isInitialized) {
            Initialize();
        } else {
            // Dynamically allocate larger storage if necessary
            auto str_value = source.value->ToString();
            value = make_shared_ptr<Value>(std::move(Value(str_value)));
        }
    }
};

std::string MergeTupleToRows(const std::string &rows, const std::string &tuple) {
    nlohmann::json rows_json;
    if (rows.empty()) {
        rows_json["rows"] = nlohmann::json::array();
    } else {
        rows_json = nlohmann::json::parse(rows);
    }
    rows_json["rows"].push_back(tuple);
    return rows_json.dump();
}

struct LlmMaxOperation {

    static std::string model_name;
    static std::string prompt_name;

    static void Initialize(const AggregateFunction &, data_ptr_t state_p) {
        auto &state = *reinterpret_cast<LlmMaxState *>(state_p);
        state.Initialize();
    }

    static void Operation(Vector inputs[], AggregateInputData &aggr_input_data, idx_t input_count, Vector &states,
                          idx_t count) {
        prompt_name = inputs[0].GetValue(0).ToString();
        model_name = inputs[1].GetValue(0).ToString();
        auto tuples = inputs[2];

        auto states_vector = FlatVector::GetData<LlmMaxState *>(states);

        for (idx_t i = 0; i < count; i++) {
            auto tuple = tuples.GetValue(i);
            auto state = states_vector[i];
            if (!state->isInitialized) {
                state->Initialize();
            } else {
                state->Update(std::move(Value(MergeTupleToRows(state->value->ToString(), tuple.ToString()))));
            }
        }
    }

    static void Combine(Vector &source, Vector &target, AggregateInputData &aggr_input_data, idx_t count) {
        auto source_vector = FlatVector::GetData<LlmMaxState *>(source);
        auto target_vector = FlatVector::GetData<LlmMaxState *>(target);
        for (idx_t i = 0; i < count; i++) {
            auto source_state = source_vector[i];
            auto target_state = target_vector[i];
            target_state->Combine(*source_state);
        }
    }

    static void Finalize(Vector &states, AggregateInputData &aggr_input_data, Vector &result, idx_t count,
                         idx_t offset) {
        auto states_vector = FlatVector::GetData<LlmMaxState *>(states);
        for (idx_t i = 0; i < count; i++) {
            auto idx = i + offset;
            auto state = states_vector[idx];
            vector<nlohmann::json> rows_vector;
            auto rows = nlohmann::json::parse(state->value->ToString())["rows"];
            for (auto &row : rows) {
                rows_vector.push_back(row);
            }
            auto prompts =
                ConstructPrompts(rows_vector, CoreModule::GetConnection(), prompt_name, llm_max_prompt_template, 2048);
            auto query_result = CoreModule::GetConnection().Query(
                "SELECT model, max_tokens FROM flockmtl_config.FLOCKMTL_MODEL_INTERNAL_TABLE WHERE model_name = '" +
                model_name + "'");

            if (query_result->RowCount() == 0) {
                throw std::runtime_error("Model not found");
            }

            auto model = query_result->GetValue(0, 0).ToString();
            auto responses = nlohmann::json::array();
            nlohmann::json settings;
            for (const auto &prompt : prompts) {
                auto response = ModelManager::CallComplete(prompt, model, settings);
                if (response.contains("row")) {
                    responses.push_back(response["row"]);
                }
            }
            result.SetValue(idx, responses.dump());
        }
    }

    static void SimpleUpdate(Vector inputs[], AggregateInputData &aggr_input_data, idx_t input_count,
                             data_ptr_t state_p, idx_t count) {
        prompt_name = inputs[0].GetValue(0).ToString();
        model_name = inputs[1].GetValue(0).ToString();
        auto tuples = inputs[2];

        auto state = reinterpret_cast<LlmMaxState *>(state_p);

        for (idx_t i = 0; i < count; i++) {
            auto tuple = tuples.GetValue(i);
            if (!state->isInitialized) {
                state->Initialize();
            } else {
                state->Update(std::move(Value(MergeTupleToRows(state->value->ToString(), tuple.ToString()))));
            }
        }
    }

    static bool IgnoreNull() {
        return true;
    }
};

std::string LlmMaxOperation::model_name;
std::string LlmMaxOperation::prompt_name;

void CoreAggregateFunctions::RegisterLlmMaxFunction(DatabaseInstance &db) {
    auto string_concat = AggregateFunction(
        "llm_max", {LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::LIST(LogicalType::ANY)},
        LogicalType::JSON(), AggregateFunction::StateSize<LlmMaxState>, LlmMaxOperation::Initialize,
        LlmMaxOperation::Operation, LlmMaxOperation::Combine, LlmMaxOperation::Finalize, LlmMaxOperation::SimpleUpdate);

    ExtensionUtil::RegisterFunction(db, string_concat);
}

} // namespace core
} // namespace flockmtl