#ifndef LLM_MIN_OR_MAX_PROMPT_TEMPLATE_H
#define LLM_MIN_OR_MAX_PROMPT_TEMPLATE_H

#include <string>

constexpr auto llm_min_or_max_prompt_template = R"(
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

enum class MinOrMax {
    MIN,
    MAX,
};

template <MinOrMax option>
std::string GetMinOrMaxPromptTemplate() {
    auto relevance = (option == MinOrMax::MAX) ? std::string("most") : std::string("least");

    auto prompt = std::string(llm_min_or_max_prompt_template);
    auto pos = prompt.find("{relevance}");
    while (pos != std::string::npos) {
        prompt.replace(pos, 10, relevance);
        pos = prompt.find("{relevance}", pos + relevance.size());
    }

    return prompt;
}

#endif // LLM_MIN_OR_MAX_PROMPT_TEMPLATE_H
