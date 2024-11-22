#pragma once

#include <string>
#include <flockmtl/prompt_manager/repository.hpp>

namespace flockmtl {

class PromptManager {
public:
    template <typename FunctionType>
    static std::string GetTemplate(FunctionType option) {
        auto prompt_template =
            PromptManager::ReplaceSection(META_PROMPT, PromptSection::INSTRUCTIONS, INSTRUCTIONS::Get(option));
        auto response_format = RESPONSE_FORMAT::Get(option);
        prompt_template =
            PromptManager::ReplaceSection(prompt_template, PromptSection::RESPONSE_FORMAT, response_format);
        return prompt_template;
    };

    template <typename FunctionType>
    static std::string Render(const std::string &user_prompt, const std::string &tuples, FunctionType option) {
        auto prompt = PromptManager::GetTemplate(option);
        prompt = PromptManager::ReplaceSection(prompt, PromptSection::USER_PROMPT, user_prompt);
        prompt = PromptManager::ReplaceSection(prompt, PromptSection::TUPLES, tuples);
        return prompt;
    };
    // Replace the predefined sections in the meta prompt with the provided content
    static std::string ReplaceSection(const std::string &prompt_template, const PromptSection section,
                                      const std::string &section_content);
    // Replace the defined string in the meta prompt with the provided content
    static std::string ReplaceSection(const std::string &prompt_template, const std::string &replace_string,
                                      const std::string &section_content);

    template <typename T>
    static std::string ToString(const T element);
    template <typename T>
    static T FromString(const std::string &element);
};

} // namespace flockmtl