#include <templates/llm_scalar_prompt_template.hpp>

std::string ScalarPromptTemplate::GetResponseFormat(ScalarFunctionType option) {
    switch (option) {
    case ScalarFunctionType::COMPLETE_JSON:
        return llm_complete_json_reponse_format;
    case ScalarFunctionType::COMPLETE:
        return llm_complete_response_format;
    case ScalarFunctionType::FILTER:
        return llm_filter_response_format;
    }
    return "";
}

std::string ScalarPromptTemplate::GetPromptTemplate(ScalarFunctionType option) {
    auto response_format = GetResponseFormat(option);
    auto prompt_template = LlmPromptTemplate::ReplaceSection(
        llm_prompt_template, LlmPromptTemplate::LlmPromptSection::INSTRUCTIONS, scalar_function_instructions);
    prompt_template = LlmPromptTemplate::ReplaceSection(
        prompt_template, LlmPromptTemplate::LlmPromptSection::RESPONSE_FORMAT, response_format);
    return prompt_template;
}

std::string ScalarPromptTemplate::GetPrompt(std::string user_prompt, std::string &tuples, ScalarFunctionType option) {
    auto prompt = GetPromptTemplate(option);
    prompt = LlmPromptTemplate::ReplaceSection(prompt, LlmPromptTemplate::LlmPromptSection::USER_PROMPT, user_prompt);
    prompt = LlmPromptTemplate::ReplaceSection(prompt, LlmPromptTemplate::LlmPromptSection::TUPLES, tuples);
    return prompt;
}