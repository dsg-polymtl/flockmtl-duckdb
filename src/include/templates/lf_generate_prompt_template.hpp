#ifndef LF_GENERATE_PROMPT_TEMPLATE_H
#define LF_GENERATE_PROMPT_TEMPLATE_H

constexpr auto lf_generate_prompt_template = R"(
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

#endif // LF_GENERATE_PROMPT_TEMPLATE_H
