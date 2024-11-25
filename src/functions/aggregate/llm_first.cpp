#include <nlohmann/json.hpp>
#include "flockmtl/core/common.hpp"
#include "flockmtl/functions/aggregate/llm_agg.hpp"

namespace flockmtl {

void AggregateRegistry::RegisterLlmFirst(duckdb::DatabaseInstance& db) {
    auto string_concat = duckdb::AggregateFunction(
        "llm_first", {duckdb::LogicalType::ANY, duckdb::LogicalType::ANY, duckdb::LogicalType::ANY},
        duckdb::LogicalType::JSON(), duckdb::AggregateFunction::StateSize<LlmAggState>, LlmAggOperation::Initialize,
        LlmAggOperation::Operation, LlmAggOperation::Combine,
        LlmAggOperation::FirstOrLastFinalize<AggregateFunctionType::FIRST>, LlmAggOperation::SimpleUpdate);

    duckdb::ExtensionUtil::RegisterFunction(db, string_concat);
}

} // namespace flockmtl
