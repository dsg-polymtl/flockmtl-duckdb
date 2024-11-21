#include "flockmtl/prompt_manager/repository.hpp"
#include "flockmtl/prompt_manager/prompt_manager.hpp"

namespace flockmtl {

template <>
std::string RESPONSE_FORMAT::Get(const ScalarFunctionType option) {
    switch (option) {
    case ScalarFunctionType::COMPLETE_JSON:
        return RESPONSE_FORMAT::COMPLETE_JSON;
    case ScalarFunctionType::COMPLETE:
        return RESPONSE_FORMAT::COMPLETE;
    case ScalarFunctionType::FILTER:
        return RESPONSE_FORMAT::FILTER;
    }
};

template <>
std::string RESPONSE_FORMAT::Get(const AggregateFunctionType option) {
    switch (option) {
    case AggregateFunctionType::REDUCE:
        return RESPONSE_FORMAT::REDUCE;
    case AggregateFunctionType::FIRST:
    case AggregateFunctionType::LAST: {
        return PromptManager::ReplaceSection(RESPONSE_FORMAT::FIRST_OR_LAST, "{{RELEVANCE}}",
                                             option == AggregateFunctionType::FIRST ? "most" : "least");
    case AggregateFunctionType::RERANK:
        return RESPONSE_FORMAT::RERANK;
    }
    };

    template <typename FunctionType>
    std::string INSTRUCTIONS::Get(const FunctionType option) {
        switch (option) {
        case ScalarFunctionType::COMPLETE_JSON:
        case ScalarFunctionType::COMPLETE:
        case ScalarFunctionType::FILTER:
            return INSTRUCTIONS::SCALAR_FUNCTION;
        case AggregateFunctionType::REDUCE:
        case AggregateFunctionType::FIRST:
        case AggregateFunctionType::LAST:
        case AggregateFunctionType::RERANK:
            return INSTRUCTIONS::AGGREGATE_FUNCTION;
        }
    };

} // namespace flockmtl