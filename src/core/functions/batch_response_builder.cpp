#include <flockmtl/core/module.hpp>
#include <flockmtl/core/functions/batch_response_builder.hpp>
#include <flockmtl/model_manager/tiktoken.hpp>
#include "flockmtl/prompt_manager/prompt_manager.hpp"

namespace flockmtl {
namespace core {

std::vector<nlohmann::json> CastVectorOfStructsToJson(Vector &struct_vector, int size) {
    vector<nlohmann::json> vector_json;
    for (auto i = 0; i < size; i++) {
        nlohmann::json json;
        for (auto j = 0; j < StructType::GetChildCount(struct_vector.GetType()); j++) {
            auto key = StructType::GetChildName(struct_vector.GetType(), j);
            auto value = StructValue::GetChildren(struct_vector.GetValue(i))[j].ToString();
            json[key] = value;
        }
        vector_json.push_back(json);
    }
    return vector_json;
}

std::string ConstructMarkdownHeader(const nlohmann::json &tuple) {
    std::string header_markdown = "|";
    for (const auto &key : tuple.items()) {
        header_markdown += key.key() + " | ";
    }
    header_markdown += "\n";
    for (const auto &key : tuple.items()) {
        header_markdown += "|---";
    }
    header_markdown += "|\n";
    return header_markdown;
}

std::string ConstructMarkdownSingleTuple(const nlohmann::json &tuple) {
    std::string tuple_markdown = "|";
    for (const auto &key : tuple.items()) {
        tuple_markdown += key.value().dump() + " | ";
    }
    tuple_markdown += "\n";
    return tuple_markdown;
}

std::string ConstructMarkdownArrayTuples(const nlohmann::json &tuples) {
    std::string tuples_markdown = "";
    tuples_markdown += ConstructMarkdownHeader(tuples[0]);
    for (const auto &tuple : tuples) {
        tuples_markdown += ConstructMarkdownSingleTuple(tuple);
    }
    return tuples_markdown;
}

PromptDetails CreatePromptDetails(Connection &con, const nlohmann::json prompt_details_json) {
    PromptDetails prompt_details;

    if (prompt_details_json.contains("prompt_name")) {
        if (!prompt_details_json.contains("version") && prompt_details_json.size() > 1) {
            throw std::runtime_error("The prompt details struct should contain a single key value pair of prompt or "
                                     "prompt_name with prompt version");
        } else if (prompt_details_json.contains("version") && prompt_details_json.size() > 2) {
            throw std::runtime_error("The prompt details struct should contain a single key value pair of prompt or "
                                     "prompt_name with prompt version");
        }
        prompt_details.prompt_name = prompt_details_json["prompt_name"];
        Query query(con);
        auto prompt_details_query = query.select("prompt")
                                        .from(FLOCK_TABLE::PROMPT)
                                        .where("prompt_name = '" + prompt_details.prompt_name + "'");
        auto error_message = "The provided `" + prompt_details.prompt_name + "` prompt ";
        if (prompt_details_json.contains("version")) {
            prompt_details.version = std::stoi(prompt_details_json["version"].get<std::string>());
            prompt_details_query.where("version = " + std::to_string(prompt_details.version));
            error_message += "with version " + std::to_string(prompt_details.version) + " not found";
        } else {
            prompt_details_query.order_by("version DESC").limit("1");
            error_message += "not found";
        }
        auto query_result = prompt_details_query.execute();
        if (query_result.empty()) {
            throw std::runtime_error(error_message);
        }
        prompt_details.prompt = std::any_cast<std::string>(query_result[0][0]);
    } else if (prompt_details_json.contains("prompt")) {
        prompt_details.prompt = prompt_details_json["prompt"];
    } else {
        throw std::runtime_error("The prompt details struct should contain a single key value pair of prompt or "
                                 "prompt_name with prompt version");
    }
    return prompt_details;
}

nlohmann::json Complete(const nlohmann::json &tuples, const std::string &user_prompt, ScalarFunctionType function_type,
                        Model &model) {
    nlohmann::json data;
    auto tuples_markdown = ConstructMarkdownArrayTuples(tuples);
    auto prompt = PromptManager::Render(user_prompt, tuples_markdown, function_type);
    auto response = model.CallComplete(prompt);
    return response["tuples"];
};

nlohmann::json BatchAndComplete(std::vector<nlohmann::json> &tuples, Connection &con, std::string user_prompt,
                                ScalarFunctionType function_type, Model &model) {
    auto llm_template = PromptManager::GetTemplate(function_type);

    int num_tokens_meta_and_user_pormpt = 0;
    num_tokens_meta_and_user_pormpt += Tiktoken::GetNumTokens(user_prompt);
    num_tokens_meta_and_user_pormpt += Tiktoken::GetNumTokens(llm_template);
    int available_tokens = model.GetModelDetails().context_window - num_tokens_meta_and_user_pormpt;

    auto responses = nlohmann::json::array();

    if (available_tokens < 0) {
        throw std::runtime_error("The total number of tokens in the prompt exceeds the model's maximum token limit");
    } else {

        auto accumulated_tuples_tokens = 0u;
        auto batch_tuples = nlohmann::json::array();
        auto batch_size = tuples.size();
        int start_index = 0;

        do {
            accumulated_tuples_tokens += Tiktoken::GetNumTokens(ConstructMarkdownHeader(tuples[start_index]));
            while (accumulated_tuples_tokens < available_tokens && start_index < tuples.size() &&
                   batch_tuples.size() < batch_size) {
                auto num_tokens = Tiktoken::GetNumTokens(ConstructMarkdownSingleTuple(tuples[start_index]));
                if (accumulated_tuples_tokens + num_tokens > available_tokens) {
                    break;
                }
                batch_tuples.push_back(tuples[start_index]);
                accumulated_tuples_tokens += num_tokens;
                start_index++;
            }

            nlohmann::json response;
            try {
                response = Complete(batch_tuples, user_prompt, function_type, model);
            } catch (const LengthExceededError &e) {
                batch_tuples.clear();
                accumulated_tuples_tokens = 0;
                auto new_batch_size = int(batch_size * 0.1);
                batch_size = 1 ? new_batch_size == 0 : new_batch_size;
                accumulated_tuples_tokens = 0;
                start_index = 0;
                continue;
            }
            auto output_tokens_per_tuple = Tiktoken::GetNumTokens(response.dump()) / batch_tuples.size();

            batch_size = model.GetModelDetails().max_output_tokens / output_tokens_per_tuple;
            batch_tuples.clear();
            accumulated_tuples_tokens = 0;

            for (const auto &tuple : response) {
                responses.push_back(tuple);
            }

        } while (start_index < tuples.size());
    }

    return responses;
}

} // namespace core
} // namespace flockmtl