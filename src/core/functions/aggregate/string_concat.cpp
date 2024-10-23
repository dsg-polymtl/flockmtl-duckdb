#include "duckdb.hpp"
#include "duckdb/common/types/string_type.hpp"
#include "duckdb/function/function_set.hpp"
#include "duckdb/planner/expression.hpp"

#include <flockmtl/common.hpp>
#include <flockmtl/core/functions/aggregate.hpp>
#include <iostream>
#include <memory>

namespace flockmtl {
namespace core {

struct StringConcatState {
public:
    bool isInitialized;
    Value value;

    void Initialize() {
        this->isInitialized = false;
        this->value = Value("");
    }

    void Combine(const StringConcatState &source) {
        if (!source.isInitialized)
            return;
        if (!this->isInitialized) {
            this->value = source.value;
            this->isInitialized = true;
        } else {
            auto combined = this->value.ToString() + " " + source.value.ToString();
            this->value = Value(combined);
        }
    }
};

struct StringConcatOperation {
    template <class STATE>
    static void Initialize(STATE &state) {
        state.Initialize();
    }

    template <class INPUT_TYPE, class STATE, class OP>
    static void ConstantOperation(STATE &state, const INPUT_TYPE &input, AggregateUnaryInput &unary_input,
                                  idx_t count) {
        for (idx_t i = 0; i < count; i++) {
            Operation<INPUT_TYPE, STATE, OP>(state, input, unary_input);
        }
    }

    template <class INPUT_TYPE, class STATE, class OP>
    static void Operation(STATE &state, const INPUT_TYPE &input, AggregateUnaryInput &unary_input) {
        if (input.Empty()) {
            return;
        }

        std::string input_str = input.GetString();

        if (!state.isInitialized) {
            state.value = Value(input_str);
            state.isInitialized = true;
        } else {
            std::string combined = state.value.ToString() + " " + input_str;
            state.value = Value(combined);
        }
    }

    template <class STATE, class OP>
    static void Combine(const STATE &source, STATE &target, AggregateInputData &) {
        target.Combine(source);
    }

    template <class TARGET_TYPE, class STATE>
    static void Finalize(STATE &state, TARGET_TYPE &target, AggregateFinalizeData &finalize_data) {
        if (state.isInitialized) {
            std::string final_string = state.value.ToString();
            finalize_data.result.SetValue(finalize_data.result_idx, Value(final_string));
        } else {
            target = "";
        }
    }

    static bool IgnoreNull() {
        return true;
    }
};

void CoreAggregateFunctions::RegisterStringConcatFunction(DatabaseInstance &db) {
    auto string_concat = AggregateFunction(
        "string_concat", {LogicalType::VARCHAR}, LogicalType::VARCHAR, AggregateFunction::StateSize<StringConcatState>,
        AggregateFunction::StateInitialize<StringConcatState, StringConcatOperation>,
        AggregateFunction::UnaryScatterUpdate<StringConcatState, string_t, StringConcatOperation>,
        AggregateFunction::StateCombine<StringConcatState, StringConcatOperation>,
        AggregateFunction::StateFinalize<StringConcatState, string_t, StringConcatOperation>,
        FunctionNullHandling::DEFAULT_NULL_HANDLING,
        AggregateFunction::UnaryUpdate<StringConcatState, string_t, StringConcatOperation>);

    ExtensionUtil::RegisterFunction(db, string_concat);
}

} // namespace core
} // namespace flockmtl