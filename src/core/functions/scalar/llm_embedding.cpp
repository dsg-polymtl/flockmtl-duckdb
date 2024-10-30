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

    /*
    nlohmann::json settings;
    if (args.ColumnCount() == 3) {
        settings = CoreScalarParsers::Struct2Json(args.data[2], 1)[0];
    }

    auto model_details = CoreScalarParsers::Struct2Json(args.data[1], 1)[0];
    auto provider_name = model_details.contains("provider") ? model_details.at("provider").get<std::string>() : "";
    auto model_name = model_details.contains("model_name") ? model_details.at("model_name").get<std::string>() : "";
    auto model = ModelManager::GetQueriedModel (con, model_name, provider_name).first;
    auto inputs = CoreScalarParsers::Struct2Json(args.data[0], args.size());
     */
    auto inputs = CoreScalarParsers::Struct2Json(args.data[0], args.size());
    auto model_details_json = CoreScalarParsers::Struct2Json(args.data[1], 1)[0];
    auto model_details = ModelManager::CreateModelDetails (con, model_details_json);

    auto embeddings = nlohmann::json::array();
    for (auto &row : inputs) {
        std::string concat_input;
        for (auto &item : row.items()) {
            concat_input += item.value().get<std::string>() + " ";
        }

        //auto element_embedding = ModelManager::CallEmbedding(concat_input, model, provider_name);
        auto element_embedding = ModelManager::CallEmbedding(concat_input, model_details);

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
} // namespace flockmtl
