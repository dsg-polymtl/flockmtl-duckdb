#include <functional>
#include <iostream>
#include <flockmtl/core/functions/batch_response_builder.hpp>
#include <flockmtl/common.hpp>
#include <flockmtl/core/functions/scalar.hpp>
#include <flockmtl/core/model_manager/model_manager.hpp>
#include <flockmtl/core/model_manager/tiktoken.hpp>
#include <flockmtl/core/parser/llm_response.hpp>
#include <flockmtl/core/parser/scalar.hpp>
#include <flockmtl/core/config/config.hpp>
#include <flockmtl_extension.hpp>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>

namespace flockmtl {
namespace core {

static void LlmFilterScalarFunction(DataChunk &args, ExpressionState &state, Vector &result) {
    Connection con(*state.GetContext().db);
    CoreScalarParsers::LlmFilterScalarParser(args);

    auto model_details_json = CoreScalarParsers::Struct2Json(args.data[0], 1)[0];
    auto model_details = ModelManager::CreateModelDetails(con, model_details_json);
    auto prompt_details_json = CoreScalarParsers::Struct2Json(args.data[1], 1)[0];
    auto prompt_details = CreatePromptDetails(con, prompt_details_json);

    auto tuples = CoreScalarParsers::Struct2Json(args.data[2], args.size());

    auto responses = BatchAndComplete(tuples, con, prompt_details.prompt, ScalarFunctionType::FILTER, model_details);

    auto index = 0;
    Vector vec(LogicalType::VARCHAR, args.size());
    UnaryExecutor::Execute<string_t, string_t>(vec, result, args.size(), [&](string_t _) {
        return StringVector::AddString(result, responses[index++].dump());
    });
}

void CoreScalarFunctions::RegisterLlmFilterScalarFunction(DatabaseInstance &db) {
    ExtensionUtil::RegisterFunction(db, ScalarFunction("llm_filter", {}, LogicalType::VARCHAR, LlmFilterScalarFunction,
                                                       nullptr, nullptr, nullptr, nullptr, LogicalType::ANY));
}

} // namespace core
} // namespace flockmtl
