#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include <flockmtl/common.hpp>
#include <flockmtl/core/functions/aggregate.hpp>
#include <flockmtl/core/functions/aggregate/llm_min_or_max.hpp>

namespace flockmtl {
namespace core {

void CoreAggregateFunctions::RegisterLlmMinFunction(DatabaseInstance &db) {
    auto string_concat = AggregateFunction(
        "llm_min", {LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::ANY}, LogicalType::JSON(),
        AggregateFunction::StateSize<LlmMinOrMaxState>, LlmMinOrMaxOperation::Initialize, LlmMinOrMaxOperation::Operation,
        LlmMinOrMaxOperation::Combine, LlmMinOrMaxOperation::Finalize<0>, LlmMinOrMaxOperation::SimpleUpdate);

    ExtensionUtil::RegisterFunction(db, string_concat);
}

} // namespace core
} // namespace flockmtl