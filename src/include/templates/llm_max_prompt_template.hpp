#ifndef LLM_MAX_PROMPT_TEMPLATE_H
#define LLM_MAX_PROMPT_TEMPLATE_H

constexpr auto llm_max_prompt_template = R"(
{{prompts}}

{
    "rows": [
        {% for row in rows %}
            {{row}},
        {% endfor %}
    ]
}

Give me the response in JSON format where it represents the row value that satisfies the condition in json if none return an empty object:

{
    "row": // response
}
)";

#endif // LLM_MAX_PROMPT_TEMPLATE_H
