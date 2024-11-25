#include <functional>
#include <iostream>
#include <flockmtl/functions/batch_response_builder.hpp>
#include <flockmtl/core/common.hpp>
#include <flockmtl/model_manager/model.hpp>
#include <flockmtl/model_manager/tiktoken.hpp>
#include <flockmtl/core/config.hpp>
#include <flockmtl/core/parser/scalar.hpp>
#include "flockmtl/prompt_manager/prompt_manager.hpp"
#include <flockmtl_extension.hpp>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include "flockmtl/registry/registry.hpp"

namespace flockmtl {

static void LlmCompleteJsonScalarFunction(duckdb::DataChunk& args, duckdb::ExpressionState& state,
                                          duckdb::Vector& result) {
    duckdb::Connection con(*state.GetContext().db);
    CoreScalarParsers::LlmCompleteJsonScalarParser(args);

    auto model_details_json = CastVectorOfStructsToJson(args.data[0], 1)[0];
    Model model(model_details_json);
    auto prompt_details_json = CastVectorOfStructsToJson(args.data[1], 1)[0];
    auto prompt_details = PromptManager::CreatePromptDetails(prompt_details_json);

    if (args.ColumnCount() == 2) {
        auto template_str = prompt_details.prompt + "\nThe Ouput should be in JSON format.";
        auto response = model.CallComplete(template_str);

        result.SetValue(0, response.dump());
    } else {
        auto tuples = CastVectorOfStructsToJson(args.data[2], args.size());

        auto responses = BatchAndComplete(tuples, con, prompt_details.prompt, ScalarFunctionType::COMPLETE_JSON, model);

        auto index = 0;
        for (const auto& response : responses) {
            result.SetValue(index++, duckdb::Value(response.dump()));
        }
    }
}

void ScalarRegistry::RegisterLlmCompleteJson(duckdb::DatabaseInstance& db) {
    duckdb::ExtensionUtil::RegisterFunction(
        db, duckdb::ScalarFunction("llm_complete_json", {}, duckdb::LogicalType::JSON(), LlmCompleteJsonScalarFunction,
                                   nullptr, nullptr, nullptr, nullptr, duckdb::LogicalType::ANY));
}

} // namespace flockmtl
