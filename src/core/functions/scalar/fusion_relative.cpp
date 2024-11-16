#include <flockmtl/common.hpp>
#include <flockmtl/core/functions/scalar.hpp>
#include <flockmtl/core/model_manager/model_manager.hpp>
#include <flockmtl/core/parser/llm_response.hpp>
#include <flockmtl/core/parser/scalar.hpp>
#include <flockmtl_extension.hpp>
#include <string>
#include <nlohmann/json.hpp>

namespace flockmtl {
namespace core {

std::vector<double> normalize(const std::vector<double> &scores) {

    double min_val = *std::min_element(scores.begin(), scores.end());
    double max_val = *std::max_element(scores.begin(), scores.end());

    std::vector<double> normalized;
    if (max_val == min_val) {
        normalized.resize(scores.size(), 0.0);
    } else {
        for (double score : scores) {
            normalized.push_back((score - min_val) / (max_val - min_val));
        }
    }

    return normalized;
}

std::vector<int> fusion_relative(DataChunk &args) {
    size_t column_size = args.ColumnCount();
    size_t tuple_size = args.size();

    std::vector<std::vector<double>> normalized_columns;
    for (size_t i = 0; i < column_size; i++) {
        std::vector<double> scores;
        for (size_t j = 0; j < tuple_size; j++) {
            scores.push_back(args.data[i].GetValue(j).GetValue<double>());
        }
        std::vector<double> normalized_scores = normalize(scores);
        normalized_columns.push_back(normalized_scores);
    }

    std::vector<int> result_idx;
    for (size_t i = 0; i < tuple_size; i++) {
        double max_score = 0.0;
        int max_idx = 0;
        for (size_t j = 0; j < column_size; j++) {
            if (normalized_columns[j][i] > max_score) {
                max_score = normalized_columns[j][i];
                max_idx = j;
            }
        }
        result_idx.push_back(max_idx);
    }

    return result_idx;
}

static void FusionRelativeScalarFunction(DataChunk &args, ExpressionState &state, Vector &result) {
    CoreScalarParsers::FusionRelativeScalarParser(args);

    std::vector<std::vector<double>> score_vectors;

    std::vector<int> result_idx = fusion_relative(args);
    for (auto i = 0; i < result_idx.size(); i++) {
        result.SetValue(i, args.data[result_idx[i]].GetValue(i));
    }
}

void CoreScalarFunctions::RegisterFusionRelativeScalarFunction(DatabaseInstance &db) {
    ExtensionUtil::RegisterFunction(db, ScalarFunction("fusion_relative", {}, LogicalType::DOUBLE,
                                                       FusionRelativeScalarFunction, nullptr, nullptr, nullptr, nullptr,
                                                       LogicalType::ANY));
}

} // namespace core
} // namespace flockmtl
