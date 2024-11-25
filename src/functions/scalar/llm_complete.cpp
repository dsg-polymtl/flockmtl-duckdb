#include <string>
#include <sstream>
#include <iostream>
#include <functional>
#include <nlohmann/json.hpp>

#include "flockmtl/core/common.hpp"
#include "flockmtl/core/parser/scalar.hpp"
#include "flockmtl/core/config.hpp"
#include "flockmtl/registry/registry.hpp"
#include "flockmtl/model_manager/model.hpp"
#include "flockmtl/functions/batch_response_builder.hpp"
#include "flockmtl/prompt_manager/prompt_manager.hpp"

namespace flockmtl {

static void LlmCompleteScalarFunction(duckdb::DataChunk& args, duckdb::ExpressionState& state, duckdb::Vector& result) {
    duckdb::Connection con(*state.GetContext().db);
    CoreScalarParsers::LlmCompleteScalarParser(args);

    auto model_details_json = CastVectorOfStructsToJson(args.data[0], 1)[0];
    Model model(model_details_json);
    auto prompt_details_json = CastVectorOfStructsToJson(args.data[1], 1)[0];
    auto prompt_details = PromptManager::CreatePromptDetails(prompt_details_json);

    if (args.ColumnCount() == 2) {
        auto template_str = prompt_details.prompt;
        auto response = model.CallComplete(template_str, false);

        result.SetValue(0, response.dump());
    } else {
        auto tuples = CastVectorOfStructsToJson(args.data[2], args.size());

        auto responses = BatchAndComplete(tuples, con, prompt_details.prompt, ScalarFunctionType::COMPLETE, model);

        auto index = 0;
        duckdb::Vector vec(duckdb::LogicalType::VARCHAR, args.size());
        duckdb::UnaryExecutor::Execute<duckdb::string_t, duckdb::string_t>(
            vec, result, args.size(),
            [&](duckdb::string_t _) { return duckdb::StringVector::AddString(result, responses[index++].dump()); });
    }
}

void ScalarRegistry::RegisterLlmComplete(duckdb::DatabaseInstance& db) {
    duckdb::ExtensionUtil::RegisterFunction(db, duckdb::ScalarFunction("llm_complete", {}, duckdb::LogicalType::VARCHAR,
                                                                       LlmCompleteScalarFunction, nullptr, nullptr,
                                                                       nullptr, nullptr, duckdb::LogicalType::ANY));
}

} // namespace flockmtl
