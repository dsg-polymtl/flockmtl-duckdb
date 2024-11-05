#include <functional>
#include <iostream>
#include <flockmtl/core/functions/batch_response_builder.hpp>
#include <flockmtl/common.hpp>
#include <flockmtl/core/functions/scalar.hpp>
#include <flockmtl/core/model_manager/model_manager.hpp>
#include <flockmtl/core/model_manager/openai.hpp>
#include <flockmtl/core/model_manager/tiktoken.hpp>
#include <flockmtl/core/parser/llm_response.hpp>
#include <flockmtl/core/config/config.hpp>
#include <flockmtl/core/parser/scalar.hpp>
#include <flockmtl_extension.hpp>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <templates/llm_complete_json_prompt_template.hpp>

namespace flockmtl {
namespace core {

static void LlmCompleteJsonScalarFunction(DataChunk &args, ExpressionState &state, Vector &result) {
    Connection con(*state.GetContext().db);
    CoreScalarParsers::LlmCompleteJsonScalarParser(args);

    auto model_details_json = CoreScalarParsers::Struct2Json(args.data[1], 1)[0];
    auto model_details = ModelManager::CreateModelDetails(con, model_details_json);

    if (args.ColumnCount() == 2) {
        auto query_result =
            con.Query("SELECT prompt FROM flockmtl_config.FLOCKMTL_PROMPT_INTERNAL_TABLE WHERE prompt_name = '" +
                      args.data[0].GetValue(0).ToString() + "'");

        if (query_result->RowCount() == 0) {
            throw std::runtime_error("Prompt not found");
        }

        auto template_str = query_result->GetValue(0, 0).ToString() + "\nThe Ouput should be in JSON format.";
        auto response = ModelManager::CallComplete(template_str, model_details);

        result.SetValue(0, response.dump());
    } else {
        auto tuples = CoreScalarParsers::Struct2Json(args.data[2], args.size());

        auto responses = BatchAndComplete(tuples, con, args.data[0].GetValue(0).ToString(),
                                          llm_complete_json_prompt_template, model_details);

        auto index = 0;
        for (const auto &response : responses) {
            result.SetValue(index++, Value(response.dump()));
        }
    }
}

void CoreScalarFunctions::RegisterLlmCompleteJsonScalarFunction(DatabaseInstance &db) {
    ExtensionUtil::RegisterFunction(db, ScalarFunction("llm_complete_json", {}, LogicalType::JSON(),
                                                       LlmCompleteJsonScalarFunction, nullptr, nullptr, nullptr,
                                                       nullptr, LogicalType::ANY));
}

} // namespace core
} // namespace flockmtl
