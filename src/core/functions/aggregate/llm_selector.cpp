#include <flockmtl/core/functions/aggregate/llm_selector.hpp>

namespace flockmtl {
namespace core {

void LlmSelectorState::Initialize() {
}

void LlmSelectorState::Update(const nlohmann::json &input) {
    value.push_back(input);
}

void LlmSelectorState::Combine(const LlmSelectorState &source) {
    value = std::move(source.value);
}

LlmSelector::LlmSelector(std::string &model, int model_context_size, std::string &user_prompt,
                         std::string &llm_selector_template)
    : model(model), model_context_size(model_context_size), user_prompt(user_prompt),
      llm_selector_template(llm_selector_template) {

    int fixed_tokens = calculateFixedTokens();

    if (fixed_tokens > model_context_size) {
        throw std::runtime_error("Fixed tokens exceed model context size");
    }

    available_tokens = model_context_size - fixed_tokens;
}

int LlmSelector::calculateFixedTokens() const {
    int fixed_tokens = 0;
    fixed_tokens += Tiktoken::GetNumTokens(user_prompt);
    fixed_tokens += Tiktoken::GetNumTokens(llm_selector_template);
    return fixed_tokens;
}

nlohmann::json LlmSelector::GetElement(const nlohmann::json &tuples) {
    inja::Environment env;
    nlohmann::json data;
    data["tuples"] = tuples;
    data["user_prompt"] = user_prompt;
    auto prompt = env.render(llm_selector_template, data);

    nlohmann::json settings;
    auto response = ModelManager::CallComplete(prompt, model, settings);
    return response["selected"];
}

nlohmann::json LlmSelector::LlmSelectorCall(nlohmann::json &tuples) {

    if (tuples.size() == 1) {
        return tuples[0]["content"];
    }

    auto num_tuples = tuples.size();
    std::vector<int> batch_size_list;
    auto used_tokens = 0;
    auto batch_size = 0;
    for (int i = 0; i < num_tuples; i++) {
        used_tokens += Tiktoken::GetNumTokens(tuples[i].dump());
        batch_size++;
        if (used_tokens >= available_tokens) {
            batch_size_list.push_back(batch_size);
            used_tokens = 0;
            batch_size = 0;
        } else if (i == num_tuples - 1) {
            batch_size_list.push_back(batch_size);
        }
    }

    auto responses = nlohmann::json::array();
    for (int i = 0; i < batch_size_list.size(); i++) {
        auto start_index = i * batch_size;
        auto end_index = start_index + batch_size;
        auto batch = nlohmann::json::array();
        for (int j = start_index; j < end_index; j++) {
            batch.push_back(tuples[j]);
        }
        auto ranked_indices = GetElement(batch);
        responses.push_back(batch[ranked_indices.get<int>()]);
    }
    return LlmSelectorCall(responses);
}

} // namespace core
} // namespace flockmtl
