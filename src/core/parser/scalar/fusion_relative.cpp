#include <flockmtl/core/common.hpp>
#include <flockmtl/core/parser/scalar.hpp>
#include <flockmtl_extension.hpp>

namespace flockmtl {

void CoreScalarParsers::FusionRelativeScalarParser(duckdb::DataChunk& args) {
    for (int i = 0; i < args.ColumnCount(); i++) {
        if (args.data[i].GetType() != duckdb::LogicalType::DOUBLE) {
            throw std::runtime_error("fusion_relative: argument must be a double");
        }
    }
}

} // namespace flockmtl
