#include <flockmtl/common.hpp>
#include <flockmtl/core/functions/scalar.hpp>
#include <flockmtl/core/parser/scalar.hpp>
#include <flockmtl_extension.hpp>

namespace flockmtl {
namespace core {

void CoreScalarParsers::LlmFilterScalarParser(DataChunk &args) {
    if (args.ColumnCount() != 3) {
        throw std::runtime_error("LlmCompleteScalarParser: Invalid number of arguments.");
    }

    // check if template is a string
    if (args.data[0].GetType() != LogicalType::VARCHAR) {
        throw std::runtime_error("LlmCompleteScalarParser: Template must be a string.");
    }
    // check if model details is a struct
    if (args.data[1].GetType().id() != LogicalTypeId::STRUCT) {
        throw std::runtime_error("LlmCompleteScalarParser: Model details must be a struct.");
    }

    if (args.data[2].GetType().id() != LogicalTypeId::STRUCT) {
        throw std::runtime_error("LlmCompleteScalarParser: Inputs must be a struct.");
    }
}

} // namespace core
} // namespace flockmtl