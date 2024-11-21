#pragma once

#include <string>

namespace flockmtl {

enum class PromptSection { USER_PROMPT, TUPLES, RESPONSE_FORMAT, INSTRUCTIONS };

enum class AggregateFunctionType { REDUCE, FIRST, LAST, RERANK };

enum class ScalarFunctionType {
    COMPLETE_JSON,
    COMPLETE,
    FILTER,
};

class PromptManager {
public:
    template <typename FunctionType>
    static std::string Render(const std::string &user_prompt, const std::string &tuples, const FunctionType option);

    template <typename FunctionType>
    static std::string GetTemplate(const FunctionType option);
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