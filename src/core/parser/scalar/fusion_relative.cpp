#include <flockmtl/common.hpp>
#include <flockmtl/core/functions/scalar.hpp>
#include <flockmtl/core/parser/scalar.hpp>
#include <flockmtl_extension.hpp>

namespace flockmtl {
namespace core {

void CoreScalarParsers::FusionRelativeScalarParser(DataChunk &args) {
    for (int i = 0; i < args.ColumnCount(); i++) {
        if (args.data[i].GetType() != LogicalType::DOUBLE) {
            throw std::runtime_error("fusion_relative: argument must be a double");
        }
    }
}

} // namespace core
} // namespace flockmtl