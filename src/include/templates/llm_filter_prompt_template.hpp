#ifndef LLM_FILTER_PROMPT_TEMPLATE_H
#define LLM_FILTER_PROMPT_TEMPLATE_H

constexpr auto llm_filter_prompt_template = R"(
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
        // For each tuple in the input tuples check the prompt condition if it's valid. The answers should be in boolean format separated by commas for each tuple.
    ]
}
)";

#endif // LLM_FILTER_PROMPT_TEMPLATE_H
