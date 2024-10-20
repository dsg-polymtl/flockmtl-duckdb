#include <large_flock/common.hpp>
#include <large_flock/core/functions/scalar.hpp>
#include <large_flock/core/model_manager/model_manager.hpp>
#include <large_flock/core/parser/llm_response.hpp>
#include <large_flock/core/parser/scalar.hpp>
#include <large_flock_extension.hpp>

namespace large_flock {
namespace core {

static void LlmEmbeddingScalarFunction(DataChunk &args, ExpressionState &state, Vector &result) {
    Connection con(*state.GetContext().db);
    CoreScalarParsers::LlmEmbeddingScalarParser(args);

    auto model = args.data[1].GetValue(0).ToString();
    auto query_result = con.Query(
        "SELECT model, max_tokens FROM lf_config.LARGE_FLOCK_MODEL_INTERNAL_TABLE WHERE model_name = '" + model + "'");

    if (query_result->RowCount() == 0) {
        throw std::runtime_error("Model not found");
    }

    auto model_name = query_result->GetValue(0, 0).ToString();
    auto inputs = CoreScalarParsers::Struct2Json(args.data[0], args.size());

    auto embeddings = nlohmann::json::array();
    for (auto &row : inputs) {
        std::string concat_input;
        for (auto &item : row.items()) {
            concat_input += item.value().get<std::string>() + " ";
        }
        auto element_embedding = ModelManager::CallEmbedding(concat_input, model_name);
        embeddings.push_back(element_embedding);
    }

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
} // namespace large_flock
