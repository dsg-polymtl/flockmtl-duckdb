#include <flockmtl/common.hpp>
#include <flockmtl/core/functions/aggregate.hpp>
#include <iostream>

namespace flockmtl {
namespace core {

struct StringConcatState {
public:
    bool isInitialized;
    Value value;

    void Initialize() {
        this->isInitialized = false;
    }

    void Combine(const StringConcatState &source) {
        if (!source.isInitialized)
            return;
        if (!this->isInitialized) {
            this->value = source.value;
            this->isInitialized = true;
        } else {
            auto combined = source.value.ToString();
            this->value = source.value;
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
            state.isInitialized = true;
            state.value = Value(input);
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
            finalize_data.result.SetValue(finalize_data.result_idx, state.value.ToString());
            state.Initialize();
        }
    }

    template <class STATE>
    static void SimpleUpdate(Vector inputs[], AggregateInputData &aggr_input_data, idx_t input_count, data_ptr_t state,
                             idx_t count) {
    }

    static bool IgnoreNull() {
        return true;
    }
};

void CoreAggregateFunctions::RegisterStringConcatFunction(DatabaseInstance &db) {
    auto string_concat = AggregateFunction(
        "string_concat", {LogicalType::VARCHAR}, LogicalType::JSON(), AggregateFunction::StateSize<StringConcatState>,
        AggregateFunction::StateInitialize<StringConcatState, StringConcatOperation>,
        AggregateFunction::UnaryScatterUpdate<StringConcatState, string_t, StringConcatOperation>,
        AggregateFunction::StateCombine<StringConcatState, StringConcatOperation>,
        AggregateFunction::StateFinalize<StringConcatState, string_t, StringConcatOperation>,
        FunctionNullHandling::DEFAULT_NULL_HANDLING, StringConcatOperation::SimpleUpdate<StringConcatState>);

    ExtensionUtil::RegisterFunction(db, string_concat);
}

} // namespace core
} // namespace flockmtl