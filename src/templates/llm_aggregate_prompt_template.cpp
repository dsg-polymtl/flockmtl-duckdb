#include <templates/llm_aggregate_prompt_template.hpp>

std::string AggregatePromptTemplate::GetResponseFormat(AggregateFunctionType option) {
    switch (option) {
    case AggregateFunctionType::REDUCE:
        return llm_reduce_reponse_format;
    case AggregateFunctionType::FIRST:
    case AggregateFunctionType::LAST: {
        auto relevance = option == AggregateFunctionType::FIRST ? "most" : "least";
        return LlmPromptTemplate::ReplaceString(llm_first_or_last_response_format, "{{RELEVANCE}}", relevance);
    }
    case AggregateFunctionType::RERANK:
        return llm_rerank_response_format;
    }
    return "";
}

std::string AggregatePromptTemplate::GetPromptTemplate(AggregateFunctionType option) {
    auto response_format = GetResponseFormat(option);
    auto prompt_template = LlmPromptTemplate::ReplaceSection(
        llm_prompt_template, LlmPromptTemplate::LlmPromptSection::INSTRUCTIONS, aggregate_function_instructions);
    prompt_template = LlmPromptTemplate::ReplaceSection(
        prompt_template, LlmPromptTemplate::LlmPromptSection::RESPONSE_FORMAT, response_format);
    return prompt_template;
}

std::string AggregatePromptTemplate::GetPrompt(std::string user_prompt, std::string &tuples,
                                               AggregateFunctionType option) {
    auto prompt = GetPromptTemplate(option);
    prompt = LlmPromptTemplate::ReplaceSection(prompt, LlmPromptTemplate::LlmPromptSection::USER_PROMPT, user_prompt);
    prompt = LlmPromptTemplate::ReplaceSection(prompt, LlmPromptTemplate::LlmPromptSection::TUPLES, tuples);
    return prompt;
}