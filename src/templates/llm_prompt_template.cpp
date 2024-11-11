#include <templates/llm_prompt_template.hpp>

std::string LlmPromptTemplate::SectionToString(LlmPromptSection section) {
    switch (section) {
    case LlmPromptSection::USER_PROMPT:
        return "{{USER_PROMPT}}";
    case LlmPromptSection::TUPLES:
        return "{{TUPLES}}";
    case LlmPromptSection::RESPONSE_FORMAT:
        return "{{RESPONSE_FORMAT}}";
    case LlmPromptSection::INSTRUCTIONS:
        return "{{INSTRUCTIONS}}";
    }
    return "";
}

std::string LlmPromptTemplate::ReplaceSection(std::string prompt_template, LlmPromptSection section,
                                              std::string section_content) {
    auto replace_string = SectionToString(section);
    return ReplaceString(prompt_template, replace_string, section_content);
}

std::string LlmPromptTemplate::ReplaceString(std::string prompt_template, std::string replace_string,
                                             std::string section_content) {
    auto prompt = prompt_template;
    auto replace_string_size = replace_string.size();
    auto replace_pos = prompt.find(replace_string);

    while (replace_pos != std::string::npos) {
        prompt.replace(replace_pos, replace_string_size, section_content);
        replace_pos = prompt.find(replace_string, replace_pos + section_content.size());
    }

    return prompt;
}