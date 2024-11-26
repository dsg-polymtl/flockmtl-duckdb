#pragma once

#include <tuple>
#include <nlohmann/json.hpp>

#include "flockmtl/core/common.hpp"
#include "flockmtl/model_manager/model.hpp"
#include "flockmtl/functions/batch_response_builder.hpp"

namespace flockmtl {

class AggregateFunctionState {
public:
    std::vector<nlohmann::json> value;

    void Initialize();
    void Update(const nlohmann::json& input);
    void Combine(const AggregateFunctionState& source);
};

class AggregateFunctionBase {
public:
    Model model;
    std::string user_query;
    std::unordered_map<void*, std::shared_ptr<AggregateFunctionState>> state_map;

public:
    explicit AggregateFunctionBase() : model(std::move(Model())), user_query("") {};

public:
    static void ValidateArguments(duckdb::Vector inputs[], idx_t input_count);
    static std::tuple<nlohmann::json, nlohmann::json, std::vector<nlohmann::json>>
    CastInputsToJson(duckdb::Vector inputs[], idx_t count);

    static bool IgnoreNull() { return true; };

    template <class Derived>
    static void Initialize(const duckdb::AggregateFunction&, duckdb::data_ptr_t state_p) {
        auto state_ptr = reinterpret_cast<AggregateFunctionState*>(state_p);
        auto function_instance = GetInstance<Derived>();
        if (function_instance->state_map.find(state_ptr) == function_instance->state_map.end()) {
            auto state = std::make_shared<AggregateFunctionState>();
            state->Initialize();
            function_instance->state_map[state_ptr] = state;
        }
    }

    template <class Derived>
    static void Operation(duckdb::Vector inputs[], duckdb::AggregateInputData& aggr_input_data, idx_t input_count,
                          duckdb::Vector& states, idx_t count) {
        ValidateArguments(inputs, input_count);

        auto [model_details, prompt_details, tuples] = CastInputsToJson(inputs, count);
        auto function_instance = GetInstance<Derived>();
        function_instance->model = Model(model_details);
        function_instance->user_query = PromptManager::CreatePromptDetails(prompt_details).prompt;

        auto states_vector = duckdb::FlatVector::GetData<AggregateFunctionState*>(states);
        for (idx_t i = 0; i < count; i++) {
            auto tuple = tuples[i];
            auto state_ptr = states_vector[i];

            auto state = function_instance->state_map[state_ptr];
            state->Update(tuple);
        }
    }

    template <class Derived>
    static void SimpleUpdate(duckdb::Vector inputs[], duckdb::AggregateInputData& aggr_input_data, idx_t input_count,
                             duckdb::data_ptr_t state_p, idx_t count) {
        ValidateArguments(inputs, input_count);

        auto [model_details, prompt_details, tuples] = CastInputsToJson(inputs, count);
        auto function_instance = GetInstance<Derived>();
        function_instance->model = Model(model_details);
        function_instance->user_query = PromptManager::CreatePromptDetails(prompt_details).prompt;

        auto state_map_p = reinterpret_cast<AggregateFunctionState*>(state_p);
        for (idx_t i = 0; i < count; i++) {
            auto tuple = tuples[i];
            auto state = function_instance->state_map[state_map_p];
            state->Update(tuple);
        }
    }

    template <class Derived>
    static void Combine(duckdb::Vector& source, duckdb::Vector& target, duckdb::AggregateInputData& aggr_input_data,
                        idx_t count) {
        auto source_vector = duckdb::FlatVector::GetData<AggregateFunctionState*>(source);
        auto target_vector = duckdb::FlatVector::GetData<AggregateFunctionState*>(target);

        auto function_instance = GetInstance<Derived>();
        for (auto i = 0; i < count; i++) {
            auto source_ptr = source_vector[i];
            auto target_ptr = target_vector[i];

            auto source_state = function_instance->state_map[source_ptr];
            auto target_state = function_instance->state_map[target_ptr];

            target_state->Combine(*source_state);
        }
    }

    static void Finalize(duckdb::Vector& states, duckdb::AggregateInputData& aggr_input_data, duckdb::Vector& result,
                         idx_t count, idx_t offset);

    // Static method to get or create an instance of the derived class
    template <typename Derived>
    static Derived* GetInstance() {
        if (instance == nullptr) {
            instance = new Derived();
        }
        return static_cast<Derived*>(instance);
    }

private:
    static AggregateFunctionBase* instance;
};

} // namespace flockmtl
