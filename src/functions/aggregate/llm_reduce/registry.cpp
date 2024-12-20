#include "flockmtl/functions/aggregate/llm_reduce.hpp"
#include "flockmtl/registry/registry.hpp"

namespace flockmtl {

void AggregateRegistry::RegisterLlmReduce(duckdb::DatabaseInstance& db) {
    auto string_concat = duckdb::AggregateFunction(
        "llm_reduce", {duckdb::LogicalType::ANY, duckdb::LogicalType::ANY, duckdb::LogicalType::ANY},
        duckdb::LogicalType::VARCHAR, duckdb::AggregateFunction::StateSize<AggregateFunctionState>,
        LlmReduce::Initialize, LlmReduce::Operation, LlmReduce::Combine,
        LlmReduce::Finalize<AggregateFunctionType::REDUCE>, LlmReduce::SimpleUpdate);

    duckdb::ExtensionUtil::RegisterFunction(db, string_concat);
}

void AggregateRegistry::RegisterLlmReduceJson(duckdb::DatabaseInstance& db) {
    auto string_concat = duckdb::AggregateFunction(
        "llm_reduce_json", {duckdb::LogicalType::ANY, duckdb::LogicalType::ANY, duckdb::LogicalType::ANY},
        duckdb::LogicalType::VARCHAR, duckdb::AggregateFunction::StateSize<AggregateFunctionState>,
        LlmReduce::Initialize, LlmReduce::Operation, LlmReduce::Combine,
        LlmReduce::Finalize<AggregateFunctionType::REDUCE_JSON>, LlmReduce::SimpleUpdate);

    duckdb::ExtensionUtil::RegisterFunction(db, string_concat);
}

} // namespace flockmtl