#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include <flockmtl/common.hpp>
#include <flockmtl/core/functions/aggregate.hpp>
#include <flockmtl/core/functions/aggregate/llm_agg.hpp>

namespace flockmtl {
namespace core {

void CoreAggregateFunctions::RegisterLlmFirstFunction(DatabaseInstance &db) {
    auto string_concat = AggregateFunction(
        "llm_first", {LogicalType::VARCHAR, LogicalType::ANY, LogicalType::ANY}, LogicalType::JSON(),
        AggregateFunction::StateSize<LlmAggState>, LlmAggOperation::Initialize, LlmAggOperation::Operation,
        LlmAggOperation::Combine, LlmAggOperation::FirstOrLastFinalize<FirstOrLast::FIRST>, LlmAggOperation::SimpleUpdate);

    ExtensionUtil::RegisterFunction(db, string_concat);
}

} // namespace core
} // namespace flockmtl