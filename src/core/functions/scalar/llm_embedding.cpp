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

static void LlmEmbeddingScalarFunction(DataChunk &args, ExpressionState &state, Vector &result) {
    Connection con(*state.GetContext().db);
    CoreScalarParsers::LlmEmbeddingScalarParser(args);

    auto inputs = CoreScalarParsers::Struct2Json(args.data[0], args.size());
    auto model_details_json = CoreScalarParsers::Struct2Json(args.data[1], 1)[0];
    auto model_details = ModelManager::CreateModelDetails(con, model_details_json);

    vector<string> prepared_inputs;
    for (auto &row : inputs) {
        std::string concat_input;
        for (auto &item : row.items()) {
            concat_input += item.value().get<std::string>() + " ";
        }
        prepared_inputs.push_back(concat_input);
    }

    auto embeddings = ModelManager::CallEmbedding(prepared_inputs, model_details);

    for (size_t index = 0; index < embeddings.size(); index++) {
        vector<Value> embedding;
        for (auto &value : embeddings[index]) {
            embedding.push_back(Value(static_cast<double>(value)));
        }
        result.SetValue(index, Value::LIST(embedding));
    }
}

void CoreScalarFunctions::RegisterLlmEmbeddingScalarFunction(DatabaseInstance &db) {
    ExtensionUtil::RegisterFunction(db, ScalarFunction("llm_embedding", {}, LogicalType::LIST(LogicalType::DOUBLE),
                                                       LlmEmbeddingScalarFunction, nullptr, nullptr, nullptr, nullptr,
                                                       LogicalType::ANY));
}

} // namespace core
} // namespace flockmtl
