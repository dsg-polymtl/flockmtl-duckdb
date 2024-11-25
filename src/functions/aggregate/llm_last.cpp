#include <nlohmann/json.hpp>
#include <flockmtl/core/common.hpp>
#include "flockmtl/registry/registry.hpp"
#include <flockmtl/functions/aggregate/llm_agg.hpp>

namespace flockmtl {

void AggregateRegistry::RegisterLlmLast(duckdb::DatabaseInstance& db) {
    auto string_concat = duckdb::AggregateFunction(
        "llm_last", {duckdb::LogicalType::ANY, duckdb::LogicalType::ANY, duckdb::LogicalType::ANY},
        duckdb::LogicalType::JSON(), duckdb::AggregateFunction::StateSize<LlmAggState>, LlmAggOperation::Initialize,
        LlmAggOperation::Operation, LlmAggOperation::Combine,
        LlmAggOperation::FirstOrLastFinalize<AggregateFunctionType::LAST>, LlmAggOperation::SimpleUpdate);

    duckdb::ExtensionUtil::RegisterFunction(db, string_concat);
}

} // namespace flockmtl
