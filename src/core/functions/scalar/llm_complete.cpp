#include <functional>
#include <iostream>
#include <flockmtl/core/functions/prompt_builder.hpp>
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

    auto model_details_json = CoreScalarParsers::Struct2Json(args.data[1], 1)[0];
    auto model_details = ModelManager::CreateModelDetails(con, model_details_json);

    if (args.ColumnCount() == 2) {
        auto query_result =
            con.Query("SELECT prompt FROM flockmtl_config.FLOCKMTL_PROMPT_INTERNAL_TABLE WHERE prompt_name = '" +
                      args.data[0].GetValue(0).ToString() + "'");

        if (query_result->RowCount() == 0) {
            throw std::runtime_error("Prompt not found");
        }

        auto template_str = query_result->GetValue(0, 0).ToString();
        auto response = ModelManager::CallComplete(template_str, model_details, false);

        result.SetValue(0, response.dump());
    } else {
        auto tuples = CoreScalarParsers::Struct2Json(args.data[2], args.size());

        auto prompts = ConstructPrompts(tuples, con, args.data[0].GetValue(0).ToString(), llm_complete_prompt_template,
                                        Config::default_max_tokens);
        nlohmann::json settings;
        if (args.ColumnCount() == 4) {
            settings = CoreScalarParsers::Struct2Json(args.data[3], 1)[0];
        }

        auto responses = nlohmann::json::array();
        for (const auto &prompt : prompts) {
            auto response = ModelManager::CallComplete(prompt, model_details, false);

            // Check if the result contains the 'rows' field and push it to the main 'rows'
            if (response.contains("rows")) {
                for (const auto &row : response["rows"]) {
                    responses.push_back(row);
                }
            }
        }

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
