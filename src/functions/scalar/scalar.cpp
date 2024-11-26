#include "flockmtl/functions/scalar/scalar.hpp"

namespace flockmtl {

nlohmann::json ScalarFunctionBase::Complete(const nlohmann::json& tuples, const std::string& user_prompt,
                                            ScalarFunctionType function_type, Model& model) {
    nlohmann::json data;
    auto prompt = PromptManager::Render(user_prompt, tuples, function_type);
    auto response = model.CallComplete(prompt);
    return response["tuples"];
};

nlohmann::json ScalarFunctionBase::BatchAndComplete(std::vector<nlohmann::json>& tuples, std::string user_prompt,
                                                    ScalarFunctionType function_type, Model& model) {
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
            accumulated_tuples_tokens +=
                Tiktoken::GetNumTokens(PromptManager::ConstructMarkdownHeader(tuples[start_index]));
            while (accumulated_tuples_tokens < available_tokens && start_index < tuples.size() &&
                   batch_tuples.size() < batch_size) {
                auto num_tokens =
                    Tiktoken::GetNumTokens(PromptManager::ConstructMarkdownSingleTuple(tuples[start_index]));
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
            } catch (const ExceededMaxOutputTokensError& e) {
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

            for (const auto& tuple : response) {
                responses.push_back(tuple);
            }

        } while (start_index < tuples.size());
    }

    return responses;
}

} // namespace flockmtl
