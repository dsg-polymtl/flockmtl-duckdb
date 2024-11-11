#include <templates/llm_scalar_prompt_templates.hpp>

std::string ScalarPromptTemplate::GetCustomizedSection(const ScalarPromptType option) {
    switch (option) {
    case ScalarPromptType::COMPLETE_JSON:
        return R"(The system should interpret database tuples and provide a response to the user's prompt for each tuple in a JSON format that contains the necessary columns for the answer.

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
    case ScalarPromptType::COMPLETE:
        return R"(The system should interpret database tuples and provide a response to the user's prompt for each tuple in plain text.

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
    case ScalarPromptType::FILTER:
        return R"(The system should interpret database tuples and provide a response to the user's prompt for each tuple in a BOOL format that would be true/false.

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
    }
    return "";
}

std::string ScalarPromptTemplate::GetScalarPromptTemplate(ScalarPromptType option) {
    auto customized_section = GetCustomizedSection(option);
    auto prompt = std::string(llm_scalar_prompt_template);
    auto replace_string = std::string("{{RESPONSE_FORMAT}}");
    auto replace_string_size = replace_string.size();
    auto replace_pos = prompt.find(replace_string);

    while (replace_pos != std::string::npos) {
        prompt.replace(replace_pos, replace_string_size, customized_section);
        replace_pos = prompt.find(replace_string, replace_pos + customized_section.size());
    }

    return prompt;
}