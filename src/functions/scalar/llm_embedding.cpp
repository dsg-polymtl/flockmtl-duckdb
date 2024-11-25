#include <flockmtl/core/common.hpp>
#include <flockmtl/model_manager/model.hpp>
#include "flockmtl/functions/batch_response_builder.hpp"
#include <flockmtl/core/parser/scalar.hpp>
#include <flockmtl_extension.hpp>
#include <string>
#include <nlohmann/json.hpp>
#include "flockmtl/registry/registry.hpp"

namespace flockmtl {

static void LlmEmbeddingScalarFunction(duckdb::DataChunk& args, duckdb::ExpressionState& state,
                                       duckdb::Vector& result) {
    duckdb::Connection con(*state.GetContext().db);
    CoreScalarParsers::LlmEmbeddingScalarParser(args);

    auto inputs = CastVectorOfStructsToJson(args.data[1], args.size());
    auto model_details_json = CastVectorOfStructsToJson(args.data[0], 1)[0];
    Model model(model_details_json);

    std::vector<std::string> prepared_inputs;
    for (auto& row : inputs) {
        std::string concat_input;
        for (auto& item : row.items()) {
            concat_input += item.value().get<std::string>() + " ";
        }
        prepared_inputs.push_back(concat_input);
    }

    auto embeddings = model.CallEmbedding(prepared_inputs);

    for (size_t index = 0; index < embeddings.size(); index++) {
        duckdb::vector<duckdb::Value> embedding;
        for (auto& value : embeddings[index]) {
            embedding.push_back(duckdb::Value(static_cast<double>(value)));
        }
        result.SetValue(index, duckdb::Value::LIST(embedding));
    }
}

void ScalarRegistry::RegisterLlmEmbedding(duckdb::DatabaseInstance& db) {
    duckdb::ExtensionUtil::RegisterFunction(
        db, duckdb::ScalarFunction("llm_embedding", {}, duckdb::LogicalType::LIST(duckdb::LogicalType::DOUBLE),
                                   LlmEmbeddingScalarFunction, nullptr, nullptr, nullptr, nullptr,
                                   duckdb::LogicalType::ANY));
}

} // namespace flockmtl
