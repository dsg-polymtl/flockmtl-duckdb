#include <string>
#include <nlohmann/json.hpp>

#include "flockmtl_extension.hpp"
#include "flockmtl/core/common.hpp"
#include "flockmtl/core/parser/scalar.hpp"
#include "flockmtl/registry/registry.hpp"
#include "flockmtl/model_manager/model.hpp"

namespace flockmtl {

static void FusionRelativeScalarFunction(duckdb::DataChunk& args, duckdb::ExpressionState& state,
                                         duckdb::Vector& result) {
    CoreScalarParsers::FusionRelativeScalarParser(args);

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

        result.SetValue(i, max);
    }
}

void ScalarRegistry::RegisterFusionRelative(duckdb::DatabaseInstance& db) {
    duckdb::ExtensionUtil::RegisterFunction(
        db, duckdb::ScalarFunction("fusion_relative", {}, duckdb::LogicalType::DOUBLE, FusionRelativeScalarFunction,
                                   nullptr, nullptr, nullptr, nullptr, duckdb::LogicalType::ANY));
}

} // namespace flockmtl
