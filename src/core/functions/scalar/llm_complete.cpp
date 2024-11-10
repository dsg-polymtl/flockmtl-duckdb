#include <functional>
#include <iostream>
#include <flockmtl/core/functions/batch_response_builder.hpp>
#include <flockmtl/common.hpp>
#include <flockmtl/core/functions/scalar.hpp>
#include <flockmtl/core/model_manager/model_manager.hpp>
#include <flockmtl/core/parser/llm_response.hpp>
#include <flockmtl/core/parser/scalar.hpp>
#include <flockmtl/core/config/config.hpp>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <templates/llm_complete_prompt_template.hpp>

namespace flockmtl {
namespace core {

static void LlmCompleteScalarFunction(DataChunk &args, ExpressionState &state, Vector &result) {
    Connection con(*state.GetContext().db);
    CoreScalarParsers::LlmCompleteScalarParser(args);

    auto model_details_json = CoreScalarParsers::Struct2Json(args.data[0], 1)[0];
    auto model_details = ModelManager::CreateModelDetails(con, model_details_json);
    auto prompt_details_json = CoreScalarParsers::Struct2Json(args.data[1], 1)[0];
    auto prompt_details = CreatePromptDetails(con, prompt_details_json);

    if (args.ColumnCount() == 2) {
        auto template_str = prompt_details.prompt;
        auto response = ModelManager::CallComplete(template_str, model_details, false);

        result.SetValue(0, response.dump());
    } else {
        auto tuples = CoreScalarParsers::Struct2Json(args.data[2], args.size());

        auto responses =
            BatchAndComplete(tuples, con, prompt_details.prompt, llm_complete_prompt_template, model_details);

        auto index = 0;
        Vector vec(LogicalType::VARCHAR, args.size());
        UnaryExecutor::Execute<string_t, string_t>(vec, result, args.size(), [&](string_t _) {
            return StringVector::AddString(result, responses[index++].dump());
        });
    }
}

void CoreScalarFunctions::RegisterLlmCompleteScalarFunction(DatabaseInstance &db) {
    ExtensionUtil::RegisterFunction(db,
                                    ScalarFunction("llm_complete", {}, LogicalType::VARCHAR, LlmCompleteScalarFunction,
                                                   nullptr, nullptr, nullptr, nullptr, LogicalType::ANY));
}

} // namespace core
} // namespace flockmtl
