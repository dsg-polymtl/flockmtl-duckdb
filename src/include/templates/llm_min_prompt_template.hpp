#ifndef LLM_MIN_PROMPT_TEMPLATE_H
#define LLM_MIN_PROMPT_TEMPLATE_H

constexpr auto llm_min_prompt_template = R"(
You are RankLLM, an intelligent assistant that ranks tuples based on their relevance to the search query.

I will provide you with some tuples, each indicated by a numerical identifier []. Identify and return the less relevant tuple to the search query.

Tuples:

{% for tuple in tuples %}
[{{tuple.id}}] {{tuple.content}}
{% endfor %}

Search Query: {{user_prompt}}

Select the **single less relevant tuples** from the tuples provided. The output format should be a JSON object with the identifier, e.g., `{"selected": id}`. Only respond with the result, do not explain or add additional words.

Response Format:

```json
{
  "selected": id
}
```
)";

#endif // LLM_MIN_PROMPT_TEMPLATE_H
