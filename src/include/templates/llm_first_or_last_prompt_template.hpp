#ifndef LLM_FIRST_OR_LAST_PROMPT_TEMPLATE_H
#define LLM_FIRST_OR_LAST_PROMPT_TEMPLATE_H

#include <string>

constexpr auto llm_first_or_last_prompt_template = R"(
You are RankLLM, an intelligent assistant that ranks tuples based on their relevance to the search query.

I will provide you with some tuples, each indicated by a numerical identifier []. Identify and return the {relevance} relevant tuple to the search query.

Tuples:

{% for tuple in tuples %}
[{{tuple.id}}] {{tuple.content}}
{% endfor %}

Search Query: {{search_query}}

Select the **single {relevance} relevant tuple** from the tuples provided.The output format should be a JSON object. Only respond with the result, do not explain or add additional words.

Response Format:

```json
{
  "selected": id
}
```
)";

enum class FirstOrLast {
    LAST,
    FIRST,
};

template <FirstOrLast option>
std::string GetFirstOrLastPromptTemplate() {
    auto relevance = (option == FirstOrLast::FIRST) ? std::string("most") : std::string("least");

    auto prompt = std::string(llm_first_or_last_prompt_template);
    auto pos = prompt.find("{relevance}");
    while (pos != std::string::npos) {
        prompt.replace(pos, 10, relevance);
        pos = prompt.find("{relevance}", pos + relevance.size());
    }

    return prompt;
}

#endif // LLM_FIRST_OR_LAST_PROMPT_TEMPLATE_H
