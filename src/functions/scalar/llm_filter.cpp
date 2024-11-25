#include <functional>
#include <iostream>
#include <flockmtl/functions/batch_response_builder.hpp>
#include <flockmtl/core/common.hpp>
#include <flockmtl/model_manager/model.hpp>
#include <flockmtl/model_manager/tiktoken.hpp>
#include "flockmtl/prompt_manager/prompt_manager.hpp"
#include <flockmtl/core/parser/scalar.hpp>
#include <flockmtl/core/config.hpp>
#include <flockmtl_extension.hpp>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include "flockmtl/registry/registry.hpp"

namespace flockmtl {

static void LlmFilterScalarFunction(duckdb::DataChunk& args, duckdb::ExpressionState& state, duckdb::Vector& result) {
    duckdb::Connection con(*state.GetContext().db);
    CoreScalarParsers::LlmFilterScalarParser(args);

    auto model_details_json = CastVectorOfStructsToJson(args.data[0], 1)[0];
    Model model(model_details_json);
    auto prompt_details_json = CastVectorOfStructsToJson(args.data[1], 1)[0];
    auto prompt_details = PromptManager::CreatePromptDetails(prompt_details_json);

    auto tuples = CastVectorOfStructsToJson(args.data[2], args.size());

    auto responses = BatchAndComplete(tuples, con, prompt_details.prompt, ScalarFunctionType::FILTER, model);

    auto index = 0;
    duckdb::Vector vec(duckdb::LogicalType::VARCHAR, args.size());
    duckdb::UnaryExecutor::Execute<duckdb::string_t, duckdb::string_t>(
        vec, result, args.size(),
        [&](duckdb::string_t _) { return duckdb::StringVector::AddString(result, responses[index++].dump()); });
}

void ScalarRegistry::RegisterLlmFilter(duckdb::DatabaseInstance& db) {
    duckdb::ExtensionUtil::RegisterFunction(db, duckdb::ScalarFunction("llm_filter", {}, duckdb::LogicalType::VARCHAR,
                                                                       LlmFilterScalarFunction, nullptr, nullptr,
                                                                       nullptr, nullptr, duckdb::LogicalType::ANY));
}

} // namespace flockmtl
