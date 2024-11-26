#include "flockmtl/functions/scalar/fusion_relative.hpp"

namespace flockmtl {

void FusionRelative::ValidateArguments(duckdb::DataChunk& args) {
    for (int i = 0; i < args.ColumnCount(); i++) {
        if (args.data[i].GetType() != duckdb::LogicalType::DOUBLE) {
            throw std::runtime_error("fusion_relative: argument must be a double");
        }
    }
}

std::vector<double> FusionRelative::Operation(duckdb::DataChunk& args) {
    FusionRelative::ValidateArguments(args);

    std::vector<double> results;
    for (auto i = 0; i < args.size(); i++) {
        auto max = 0.0;
        for (auto j = 0; j < args.ColumnCount(); j++) {
            auto valueWrapper = args.data[j].GetValue(i);
            if (!valueWrapper.IsNull()) {
                auto value = valueWrapper.GetValue<double>();
                if (value > max) {
                    max = value;
                }
            }
        }

        results.push_back(max);
    }

    return results;
}

void FusionRelative::Execute(duckdb::DataChunk& args, duckdb::ExpressionState& state, duckdb::Vector& result) {
    auto results = FusionRelative::Operation(args);

    auto index = 0;
    for (const auto& res : results) {
        result.SetValue(index++, duckdb::Value(res));
    }
}

} // namespace flockmtl
