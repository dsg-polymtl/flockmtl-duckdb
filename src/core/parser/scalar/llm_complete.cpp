#include <large_flock/common.hpp>
#include <large_flock/core/functions/scalar.hpp>
#include <large_flock/core/parser/scalar.hpp>
#include <large_flock_extension.hpp>

namespace large_flock {
namespace core {

void CoreScalarParsers::LlmCompleteScalarParser(DataChunk &args) {
    if (args.ColumnCount() < 2 || args.ColumnCount() > 3) {
        throw std::runtime_error("LlmCompleteScalarParser: Invalid number of arguments.");
    }

    // check if template and model are strings
    if (args.data[0].GetType() != LogicalType::VARCHAR) {
        throw std::runtime_error("LlmCompleteScalarParser: Template must be a string.");
    }
    if (args.data[1].GetType() != LogicalType::VARCHAR) {
        throw std::runtime_error("LlmCompleteScalarParser: Model must be a string.");
    }
    if (args.ColumnCount() == 3) {
        if (args.data[2].GetType().id() != LogicalTypeId::STRUCT) {
            throw std::runtime_error("LlmCompleteScalarParser: Settings must be a struct.");
        }
    }
}

} // namespace core
} // namespace large_flock