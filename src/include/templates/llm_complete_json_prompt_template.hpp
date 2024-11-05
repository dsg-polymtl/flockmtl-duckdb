#ifndef LLM_MAP_PROMPT_TEMPLATE_H
#define LLM_MAP_PROMPT_TEMPLATE_H

constexpr auto llm_complete_json_prompt_template = R"(
{{user_prompt}}

{
    "tuples": [
        {% for tuple in tuples %}
        {
            {% for key, value in tuple %}
            "{{key}}": "{{value}}"
            {% endfor %}
        }
        {% endfor %}
    ]
}

Respond in a json format as follow:

{
    "tuples":[
        // For each tuple in the input tuples add the necessary columns that would ensure that the prompt is correctly answered.
    ]
}
)";

#endif // LLM_MAP_PROMPT_TEMPLATE_H
