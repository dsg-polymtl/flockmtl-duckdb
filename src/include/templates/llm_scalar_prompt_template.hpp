#ifndef LLM_SCALAR_PROMPT_TEMPLATES_HPP
#define LLM_SCALAR_PROMPT_TEMPLATES_HPP

#include <string>
#include <templates/llm_prompt_template.hpp>

constexpr auto scalar_function_instructions =
    R"(- The response should be directly relevant to each tuple without additional formatting, purely answering the prompt as if each tuple were a standalone entity.
- Use clear, context-relevant language to generate a meaningful and concise answer for each tuple.)";

constexpr auto llm_complete_json_reponse_format =
    R"(The system should interpret database tuples and provide a response to the user's prompt for each tuple in a JSON format that contains the necessary columns for the answer.

The tool should respond in JSON format as follows:

```json
{
    "tuples": [
        {<response 1>},
        {<response 2>},
             ...
        {<response n>}
    ]
}
```)";
constexpr auto llm_complete_response_format =
    R"(The system should interpret database tuples and provide a response to the user's prompt for each tuple in plain text.

The tool should respond in JSON format as follows:

```json
{
    "tuples": [
        "<response 1>",
        "<response 2>",
             ...
        "<response n>"
    ]
}
```)";

constexpr auto llm_filter_response_format =
    R"(The system should interpret database tuples and provide a response to the user's prompt for each tuple in a BOOL format that would be true/false.

The tool should respond in JSON format as follows:

```json
{
    "tuples": [
        <bool response 1>,
        <bool response 2>,
             ...
        <bool response n>
    ]
}
```)";

enum class ScalarFunctionType {
    COMPLETE_JSON,
    COMPLETE,
    FILTER,
};

class ScalarPromptTemplate {
public:
    static std::string GetResponseFormat(ScalarFunctionType option);
    static std::string GetPromptTemplate(ScalarFunctionType option);
    static std::string GetPrompt(std::string user_prompt, std::string &tuples, ScalarFunctionType option);
};

#endif // LLM_SCALAR_PROMPT_TEMPLATES_HPP
