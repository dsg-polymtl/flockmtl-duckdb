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
    }
};

std::string PromptManager::ReplaceSection(const std::string &prompt_template, const PromptSection section,
                                          const std::string &section_content) {
    auto replace_string = PromptManager::ToString(section);
    return PromptManager::ReplaceSection(prompt_template, replace_string, section_content);
};

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
};

template <typename FunctionType>
std::string PromptManager::GetTemplate(FunctionType option) {
    auto prompt_template =
        PromptManager::ReplaceSection(META_PROMPT, PromptSection::INSTRUCTIONS, INSTRUCTIONS::Get(option));
    auto response_format = RESPONSE_FORMAT::Get(option);
    prompt_template = PromptManager::ReplaceSection(prompt_template, PromptSection::RESPONSE_FORMAT, response_format);
    return prompt_template;
};

template <typename FunctionType>
std::string PromptManager::Render(const std::string &user_prompt, const std::string &tuples, FunctionType option) {
    auto prompt = PromptManager::GetTemplate(option);
    prompt = PromptManager::ReplaceSection(prompt, PromptSection::USER_PROMPT, user_prompt);
    prompt = PromptManager::ReplaceSection(prompt, PromptSection::TUPLES, tuples);
    return prompt;
};

} // namespace flockmtl