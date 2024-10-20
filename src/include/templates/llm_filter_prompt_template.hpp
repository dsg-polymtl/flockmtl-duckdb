#ifndef LLM_FILTER_PROMPT_TEMPLATE_H
#define LLM_FILTER_PROMPT_TEMPLATE_H

constexpr auto llm_filter_prompt_template = R"(
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
        // For each row in the input rows check the prompt condition if it's valid. The answers should be in boolean format separated by commas for each row.
    ]
}
)";

#endif // LLM_FILTER_PROMPT_TEMPLATE_H
