#include <flockmtl/common.hpp>
#include <flockmtl/core/functions/scalar.hpp>
#include <flockmtl/model_manager/model.hpp>
#include <flockmtl/core/parser/llm_response.hpp>
#include <flockmtl/core/parser/scalar.hpp>
#include <flockmtl_extension.hpp>
#include <string>
#include <nlohmann/json.hpp>

namespace flockmtl {
namespace core {

static void FusionRelativeScalarFunction(DataChunk &args, ExpressionState &state, Vector &result) {
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

void CoreScalarFunctions::RegisterFusionRelativeScalarFunction(DatabaseInstance &db) {
    ExtensionUtil::RegisterFunction(db, ScalarFunction("fusion_relative", {}, LogicalType::DOUBLE,
                                                       FusionRelativeScalarFunction, nullptr, nullptr, nullptr, nullptr,
                                                       LogicalType::ANY));
}

} // namespace core
} // namespace flockmtl
