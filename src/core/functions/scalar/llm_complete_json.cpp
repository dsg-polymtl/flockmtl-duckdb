#include <algorithm>
#include <cmath>
#include <flockmtl/common.hpp>
#include <flockmtl/core/functions/scalar.hpp>
#include <flockmtl/core/model_manager/model_manager.hpp>
#include <flockmtl/core/model_manager/openai.hpp>
#include <flockmtl/core/model_manager/tiktoken.hpp>
#include <flockmtl/core/parser/llm_response.hpp>
#include <flockmtl/core/parser/scalar.hpp>
#include <flockmtl_extension.hpp>
#include <functional>
#include <inja/inja.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <templates/llm_complete_json_prompt_template.hpp>

namespace flockmtl {
namespace core {

template <typename T>
std::string ToString(const T &value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

nlohmann::json GetMaxLengthValues(const std::vector<nlohmann::json> &params) {
    nlohmann::json attr_to_max_token_length;

    for (const auto &json_obj : params) {
        for (const auto &item : json_obj.items()) {
            auto key = item.key();
            auto value_str = ToString(item.value());
            int length = value_str.length();

            if (attr_to_max_token_length.contains(key)) {
                auto current_max_value_str = ToString(attr_to_max_token_length[key]);
                if (current_max_value_str.length() < length) {
                    attr_to_max_token_length[key] = item.value();
                }
            } else {
                attr_to_max_token_length[key] = item.value();
            }
        }
    }

    return attr_to_max_token_length;
}

std::string combine_values(const nlohmann::json &json_obj) {
    std::string combined;
    for (const auto &item : json_obj.items()) {
        combined += ToString(item.value()) + " ";
    }

    if (!combined.empty()) {
        combined.pop_back();
    }
    return combined;
}

inline std::vector<std::string> ConstructPrompts(std::vector<nlohmann::json> &unique_rows, Connection &con,
                                                 std::string prompt_name, int model_max_tokens = 4096) {
    inja::Environment env;

    auto query_result = con.Query(
        "SELECT prompt FROM flockmtl_config.FLOCKMTL_PROMPT_INTERNAL_TABLE WHERE prompt_name = '" + prompt_name + "'");

    if (query_result->RowCount() == 0) {
        throw std::runtime_error("Prompt not found");
    }

    auto template_str = query_result->GetValue(0, 0).ToString();
    auto row_tokens = Tiktoken::GetNumTokens(template_str);
    auto max_length_values = GetMaxLengthValues(unique_rows);
    auto combined_values = combine_values(max_length_values);
    row_tokens += Tiktoken::GetNumTokens(combined_values);

    std::vector<std::string> prompts;

    if (row_tokens > model_max_tokens) {
        throw std::runtime_error("The total number of tokens in the prompt exceeds the model's maximum token limit");
    } else {
        auto template_tokens = Tiktoken::GetNumTokens(llm_complete_json_prompt_template);
        auto max_tokens_for_rows = model_max_tokens - template_tokens;
        auto max_chunk_size = max_tokens_for_rows / row_tokens;
#undef min
        auto chunk_size = std::min(max_chunk_size, static_cast<int>(unique_rows.size()));
        auto num_chunks = static_cast<int>(std::ceil(static_cast<double>(unique_rows.size()) / chunk_size));

        for (int i = 0; i < num_chunks; ++i) {
            nlohmann::json data;
            data["prompts"] = template_str;

            for (int j = 0; j < chunk_size; ++j) {
                data["rows"].push_back(unique_rows[i + j]);
            }

            auto prompt = env.render(llm_complete_json_prompt_template, data);
            prompts.push_back(prompt);
        }
    }

    return prompts;
}

static void LlmCompleteJsonScalarFunction(DataChunk &args, ExpressionState &state, Vector &result) {
    Connection con(*state.GetContext().db);
    CoreScalarParsers::LlmCompleteJsonScalarParser(args);

    auto model = args.data[1].GetValue(0).ToString();
    auto query_result =
        con.Query("SELECT model, max_tokens FROM flockmtl_config.FLOCKMTL_MODEL_INTERNAL_TABLE WHERE model_name = '" +
                  model + "'");

    if (query_result->RowCount() == 0) {
        throw std::runtime_error("Model not found");
    }

    auto model_name = query_result->GetValue(0, 0).ToString();
    auto model_max_tokens = query_result->GetValue(1, 0).GetValue<int32_t>();

    if (args.ColumnCount() == 2) {
        auto query_result =
            con.Query("SELECT prompt FROM flockmtl_config.FLOCKMTL_PROMPT_INTERNAL_TABLE WHERE prompt_name = '" +
                      args.data[0].GetValue(0).ToString() + "'");

        if (query_result->RowCount() == 0) {
            throw std::runtime_error("Prompt not found");
        }

        auto template_str = query_result->GetValue(0, 0).ToString() + "\nThe Ouput should be in JSON format.";
        nlohmann::json settings;
        auto response = ModelManager::CallComplete(template_str, model_name, settings);

        result.SetValue(0, response.dump());
    } else {
        auto tuples = CoreScalarParsers::Struct2Json(args.data[2], args.size());

        auto prompts = ConstructPrompts(tuples, con, args.data[0].GetValue(0).ToString(), model_max_tokens);

        nlohmann::json settings;
        if (args.ColumnCount() == 4) {
            settings = CoreScalarParsers::Struct2Json(args.data[3], 1)[0];
        }

        auto responses = nlohmann::json::array();
        for (const auto &prompt : prompts) {
            // Call ModelManager::CallComplete and get the rows
            auto response = ModelManager::CallComplete(prompt, model_name, settings);

            // Check if the result contains the 'rows' field and push it to the main 'rows'
            if (response.contains("rows")) {
                for (const auto &row : response["rows"]) {
                    responses.push_back(row);
                }
            }
        }

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
