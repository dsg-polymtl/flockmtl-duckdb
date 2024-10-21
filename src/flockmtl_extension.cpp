#define DUCKDB_EXTENSION_MAIN

#include "flockmtl_extension.hpp"

#include "duckdb/parser/parser.hpp"
#include "duckdb/parser/statement/extension_statement.hpp"
#include "flockmtl/common.hpp"
#include "flockmtl/core/module.hpp"
#include "flockmtl/core/parser/query_parser.hpp"

#include <flockmtl/core/model_manager/model_manager.hpp>

namespace duckdb {

static void LoadInternal(DatabaseInstance &instance) {
    const auto *key = std::getenv("OPENAI_API_KEY");
    if (!key)
        throw std::runtime_error("FLOCKMTL EXTENSION ERROR: To load and use the extension, an OpenAI API key needs to "
                                 "be defined as an env variable: OPENAI_API_KEY");

    flockmtl::core::ModelManager::OPENAI_API_KEY = key;

    flockmtl::core::CoreModule::Register(instance);

    // Register the custom parser
    auto &config = DBConfig::GetConfig(instance);
    DuckParserExtension duck_parser;
    config.parser_extensions.push_back(duck_parser);
    config.operator_extensions.push_back(make_uniq<DuckOperatorExtension>());
}

ParserExtensionParseResult duck_parse(ParserExtensionInfo *, const std::string &query) {
    flockmtl::core::QueryParser query_parser;

    // Translate and print SQL queries for each input query
    std::string sql_query = query_parser.ParseQuery(query);

    // Parse and return the statement using DuckDB's parser
    Parser parser;
    parser.ParseQuery(sql_query);
    auto statements = std::move(parser.statements);

    return ParserExtensionParseResult(
        make_uniq_base<ParserExtensionParseData, DuckParseData>(std::move(statements[0])));
}

ParserExtensionPlanResult duck_plan(ParserExtensionInfo *, ClientContext &context,
                                    unique_ptr<ParserExtensionParseData> parse_data) {
    auto state = context.registered_state->Get<DuckState>("duck");
    if (state) {
        context.registered_state->Remove("duck");
    }
    context.registered_state->GetOrCreate<DuckState>("duck", std::move(parse_data));
    throw BinderException("Use duck_bind instead");
}

BoundStatement duck_bind(ClientContext &context, Binder &binder, OperatorExtensionInfo *info, SQLStatement &statement) {
    switch (statement.type) {
    case StatementType::EXTENSION_STATEMENT: {
        auto &extension_statement = dynamic_cast<ExtensionStatement &>(statement);
        if (extension_statement.extension.parse_function == duck_parse) {
            auto duck_state = context.registered_state->Get<DuckState>("duck");
            if (duck_state) {
                auto duck_binder = Binder::CreateBinder(context, &binder);
                auto duck_parse_data = dynamic_cast<DuckParseData *>(duck_state->parse_data.get());
                auto statement = duck_binder->Bind(*(duck_parse_data->statement));
                BoundStatement result;
                return statement;
            }
            throw BinderException("Registered state not found");
        }
    }
    default:
        // No-op empty
        return {};
    }
}

void FlockmtlExtension::Load(DuckDB &db) {
    LoadInternal(*db.instance);
}

std::string FlockmtlExtension::Name() {
    return "flockmtl";
}
std::string FlockmtlExtension::Version() const {
#ifdef EXT_VERSION_FLOCKMTL
    return EXT_VERSION_FLOCKMTL;
#else
    return "";
#endif
}

} // namespace duckdb

extern "C" {

DUCKDB_EXTENSION_API void flockmtl_init(DatabaseInstance &db) {
    duckdb::DuckDB db_wrapper(db);
    db_wrapper.LoadExtension<duckdb::FlockmtlExtension>();
}

DUCKDB_EXTENSION_API const char *flockmtl_version() {
    return duckdb::DuckDB::LibraryVersion();
}
}

#ifndef DUCKDB_EXTENSION_MAIN
#error DUCKDB_EXTENSION_MAIN not defined
#endif
