#include <functional>
#include <iostream>
#include <flockmtl/core/functions/prompt_builder.hpp>
#include <flockmtl/common.hpp>
#include <flockmtl/core/functions/scalar.hpp>
#include <flockmtl/core/model_manager/model_manager.hpp>
#include <flockmtl/core/parser/llm_response.hpp>
#include <flockmtl/core/parser/scalar.hpp>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <templates/llm_complete_prompt_template.hpp>


namespace flockmtl {
namespace core {

static void LlmCompleteScalarFunction(DataChunk &args, ExpressionState &state, Vector &result) {
    Connection con(*state.GetContext().db);
    CoreScalarParsers::LlmCompleteScalarParser(args);

    nlohmann::json settings;
    if (args.ColumnCount() == 4) {
        settings = CoreScalarParsers::Struct2Json(args.data[3], 1)[0];
    }

    std::string provider_name = settings.contains("provider") ? settings.at("provider").get<std::string>() : "";
    //bool provider_available = get_provider_name_from_settings (settings, provider_name);

    auto model_name = args.data[1].GetValue(0).ToString();
    /*
    auto query_result = provider_available ? con.Query("SELECT model, max_tokens FROM flockmtl_config.FLOCKMTL_MODEL_USER_DEFINED_INTERNAL_TABLE WHERE model_name = '" +
                                                         model + "'" + " AND provider_name = '" + provider_name + "'") :
                                           con.Query("SELECT model, max_tokens FROM flockmtl_config.FLOCKMTL_MODEL_USER_DEFINED_INTERNAL_TABLE WHERE model_name = '" +
                                                         model + "'");

    if (query_result->RowCount() == 0) {
        query_result = con.Query("SELECT model, max_tokens FROM flockmtl_config.FLOCKMTL_MODEL_DEFAULT_INTERNAL_TABLE WHERE model_name = '" +
                  model + "'");

        if (query_result->RowCount() == 0) {
            throw std::runtime_error("Model not found");
        }
    }
    */
    auto model_query_result = ModelManager::GetQueriedModel (con, model_name, provider_name);
//    auto model = query_result->GetValue(0, 0).ToString();
//    auto model_max_tokens = query_result->GetValue(1, 0).GetValue<int32_t>();

    auto model = model_query_result.first;
    auto model_max_tokens = model_query_result.second;

    if (args.ColumnCount() == 2) {
        auto query_result =
            con.Query("SELECT prompt FROM flockmtl_config.FLOCKMTL_PROMPT_INTERNAL_TABLE WHERE prompt_name = '" +
                      args.data[0].GetValue(0).ToString() + "'");

        if (query_result->RowCount() == 0) {
            throw std::runtime_error("Prompt not found");
        }

        auto template_str = query_result->GetValue(0, 0).ToString();
        //auto response = ModelManager::CallComplete(template_str, model_name, settings, false);
        auto response = ModelManager::CallComplete(template_str, model, provider_name, settings, false);

        result.SetValue(0, response.dump());
    } else {
        auto tuples = CoreScalarParsers::Struct2Json(args.data[2], args.size());

        auto prompts = ConstructPrompts(tuples, con, args.data[0].GetValue(0).ToString(), llm_complete_prompt_template,
                                        model_max_tokens);
        nlohmann::json settings;
        if (args.ColumnCount() == 4) {
            settings = CoreScalarParsers::Struct2Json(args.data[3], 1)[0];
        }

        auto responses = nlohmann::json::array();
        for (const auto &prompt : prompts) {
            // Call ModelManager::CallComplete and get the rows
            //auto response = provider_available ? ModelManager::CallComplete(prompt, model, provider_name, settings, false) :
            //                                 ModelManager::CallComplete(prompt, model, settings, false);

            auto response = ModelManager::CallComplete(prompt, model, provider_name, settings, false);

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
