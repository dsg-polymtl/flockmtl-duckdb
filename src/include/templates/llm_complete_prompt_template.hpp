#ifndef LLM_COMPLETE_PROMPT_TEMPLATE_H
#define LLM_COMPLETE_PROMPT_TEMPLATE_H

constexpr auto llm_complete_prompt_template = R"(
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
        // For each tuple in the input tuples answer the prompt in a plain text format.
    ]
}
)";

#endif // LLM_COMPLETE_PROMPT_TEMPLATE_H
