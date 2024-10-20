#ifndef LLM_MAP_PROMPT_TEMPLATE_H
#define LLM_MAP_PROMPT_TEMPLATE_H

constexpr auto llm_map_prompt_template = R"(
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
        // For each row in the input rows add the necessary columns that would ensure that the prompt is correctly answered.
    ]
}
)";

#endif // LLM_MAP_PROMPT_TEMPLATE_H
