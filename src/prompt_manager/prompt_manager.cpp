#include "flockmtl/prompt_manager/prompt_manager.hpp"
#include "flockmtl/prompt_manager/repository.hpp"

#include <string>

namespace flockmtl {

template <>
std::string PromptManager::ToString<PromptSection>(PromptSection section) {
    switch (section) {
    case PromptSection::USER_PROMPT:
        return "{{USER_PROMPT}}";
    case PromptSection::TUPLES:
        return "{{TUPLES}}";
    case PromptSection::RESPONSE_FORMAT:
        return "{{RESPONSE_FORMAT}}";
    case PromptSection::INSTRUCTIONS:
        return "{{INSTRUCTIONS}}";
    default:
        return "";
    }
}

std::string PromptManager::ReplaceSection(const std::string &prompt_template, const PromptSection section,
                                          const std::string &section_content) {
    auto replace_string = PromptManager::ToString(section);
    return PromptManager::ReplaceSection(prompt_template, replace_string, section_content);
}

std::string PromptManager::ReplaceSection(const std::string &prompt_template, const std::string &replace_string,
                                          const std::string &section_content) {
    auto prompt = prompt_template;
    auto replace_string_size = replace_string.size();
    auto replace_pos = prompt.find(replace_string);

    while (replace_pos != std::string::npos) {
        prompt.replace(replace_pos, replace_string_size, section_content);
        replace_pos = prompt.find(replace_string, replace_pos + section_content.size());
    }

    return prompt;
}

} // namespace flockmtl