#ifndef LLM_COMPLETE_PROMPT_TEMPLATE_H
#define LLM_COMPLETE_PROMPT_TEMPLATE_H

constexpr auto llm_complete_prompt_template = R"(
{{prompts}}

{
    "rows": [
        {% for row in rows %}
        {
            {% for key, value in row %}
            "{{key}}": "{{value}}"
            {% endfor %}
        }
        {% endfor %}
    ]
}

Respond in a json format as follow:

{
    "rows":[
        // For each row in the input rows answer the prompt in a plain text format.
    ]
}
)";

#endif // LLM_COMPLETE_PROMPT_TEMPLATE_H
