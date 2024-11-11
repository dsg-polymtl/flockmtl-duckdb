#ifndef LLM_PROMPT_TEMPLATES_HPP
#define LLM_PROMPT_TEMPLATES_HPP

#include <string>

constexpr auto llm_prompt_template =
    R"(You are a semantic analysis tool for DBMS. The tool will analyze each tuple in the provided data and respond to user requests based on this context.

User Prompt:

- {{USER_PROMPT}}

Tuples Table:

{{TUPLES}}

Instructions:

{{INSTRUCTIONS}}

Expected Response Format:

{{RESPONSE_FORMAT}}
)";

class LlmPromptTemplate {
public:
    enum class LlmPromptSection { USER_PROMPT, TUPLES, RESPONSE_FORMAT, INSTRUCTIONS };

    static std::string SectionToString(LlmPromptSection section);

    static std::string ReplaceSection(std::string prompt_template, LlmPromptSection section,
                                      std::string section_content);

    static std::string ReplaceString(std::string prompt_template, std::string replace_string,
                                     std::string section_content);
};

#endif // LLM_PROMPT_TEMPLATES_HPP
