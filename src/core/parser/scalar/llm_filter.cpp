#include <flockmtl/common.hpp>
#include <flockmtl/core/functions/scalar.hpp>
#include <flockmtl/core/parser/scalar.hpp>
#include <flockmtl_extension.hpp>

namespace flockmtl {
namespace core {

void CoreScalarParsers::LlmFilterScalarParser(DataChunk &args) {
    if (args.ColumnCount() != 3) {
        throw std::runtime_error("LlmFilterScalarParser: Invalid number of arguments.");
    }

    if (args.data[0].GetType().id() != LogicalTypeId::STRUCT) {
        throw std::runtime_error("LlmFilterScalarParser: Model details must be a struct.");
    }
    if (args.data[1].GetType().id() != LogicalTypeId::STRUCT) {
        throw std::runtime_error("LlmFilterScalarParser: Prompt details must be a struct.");
    }
    if (args.data[2].GetType().id() != LogicalTypeId::STRUCT) {
        throw std::runtime_error("LlmFilterScalarParser: Inputs must be a struct.");
    }
}

} // namespace core
} // namespace flockmtl