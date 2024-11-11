#ifndef LLM_SCALAR_PROMPT_TEMPLATES_HPP
#define LLM_SCALAR_PROMPT_TEMPLATES_HPP

#include <string>
#include <stdexcept>

enum class ScalarPromptType {
    COMPLETE_JSON,
    COMPLETE,
    FILTER,
};

class ScalarPromptTemplate {
public:
    static std::string GetCustomizedSection(const ScalarPromptType option);
    static std::string GetScalarPromptTemplate(ScalarPromptType option);
};

constexpr auto llm_scalar_prompt_template =
    R"(You are a semantic analysis tool for DBMS. The tool will analyze each tuple in the provided data and respond to user requests based on this context.

User Prompt:

{{user_prompt}}

Tuples Table:

{{tuples}}

Expected Response Format

{{RESPONSE_FORMAT}}

Instructions
- The response should be directly relevant to each tuple without additional formatting, purely answering the prompt as if each tuple were a standalone entity.
- Use clear, context-relevant language to generate a meaningful and concise answer for each tuple.)";

#endif // LLM_SCALAR_PROMPT_TEMPLATES_HPP
