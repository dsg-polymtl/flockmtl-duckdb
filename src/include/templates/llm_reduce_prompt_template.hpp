#ifndef LLM_REDUCE_PROMPT_TEMPLATE_H
#define LLM_REDUCE_PROMPT_TEMPLATE_H

#include <string>

constexpr auto llm_reduce_prompt_template = R"(
You are ReduceLLM, an intelligent assistant that synthesizes information from multiple tuples to create a single unified response, concise output based on the reduce query.

I will provide you with a list of tuples. Using the reduce query, reduce and combine relevant information from all tuples into a single unified response.

**Tuples**:

{% for tuple in tuples %}
    - {{tuple}}
{% endfor %}

**Reduce Query**: {{reduce_query}}

Return a single, coherent output that synthesizes the most relevant information from the tuples provided. Respond in the following JSON format. **Do not add explanations or additional words beyond the summarized content.**

Response Format:

```json
{
  "output": <summarized content here>
}
```
)";

#endif // LLM_REDUCE_PROMPT_TEMPLATE_H
