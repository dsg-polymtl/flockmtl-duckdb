#include <inja/inja.hpp>
#include <flockmtl/core/module.hpp>
#include <flockmtl/core/functions/batch_response_builder.hpp>
#include <flockmtl/core/model_manager/tiktoken.hpp>

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
        tuple_markdown += key.value().get<std::string>() + " | ";
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
    if (prompt_details_json.size() != 1) {
        throw std::runtime_error(
            "The prompt details struct should contain a single key value pair of prompt_name or prompt");
    }

    if (prompt_details_json.contains("prompt_name")) {
        prompt_details.prompt_name = prompt_details_json["prompt_name"];
        auto query_result =
            con.Query("SELECT prompt FROM flockmtl_config.FLOCKMTL_PROMPT_INTERNAL_TABLE WHERE prompt_name = '" +
                      prompt_details.prompt_name + "'");
        if (query_result->RowCount() == 0) {
            throw std::runtime_error("The provided `" + prompt_details.prompt_name + "` prompt not found");
        }
        prompt_details.prompt = query_result->GetValue(0, 0).ToString();
    } else if (prompt_details_json.contains("prompt")) {
        prompt_details.prompt = prompt_details_json["prompt"];
    } else {
        throw std::runtime_error(
            "The prompt details struct should contain a single key value pair of prompt_name or prompt");
    }
    return prompt_details;
}

nlohmann::json Complete(const std::string &tuples, const std::string &user_prompt, const std::string &llm_template,
                        const ModelDetails &model_details) {
    inja::Environment env;
    nlohmann::json data;
    data["user_prompt"] = user_prompt;
    data["tuples"] = tuples;
    auto prompt = env.render(llm_template, data);

    std::cout << prompt << std::endl;

    auto response = ModelManager::CallComplete(prompt, model_details);

    std::cout << response.dump() << std::endl;

    return response["tuples"];
};

nlohmann::json BatchAndComplete(std::vector<nlohmann::json> &tuples, Connection &con, std::string user_prompt,
                                ScalarPromptType prompt_type, const ModelDetails &model_details) {
    auto llm_template = ScalarPromptTemplate::GetScalarPromptTemplate(prompt_type);

    int num_tokens_meta_and_user_pormpt = 0;
    num_tokens_meta_and_user_pormpt += Tiktoken::GetNumTokens(user_prompt);
    num_tokens_meta_and_user_pormpt += Tiktoken::GetNumTokens(llm_template);
    int available_tokens = model_details.context_window - num_tokens_meta_and_user_pormpt;

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
                response =
                    Complete(ConstructMarkdownArrayTuples(batch_tuples), user_prompt, llm_template, model_details);
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

            batch_size = model_details.max_output_tokens / output_tokens_per_tuple;
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