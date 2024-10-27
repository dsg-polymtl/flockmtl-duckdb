#ifndef LLM_MAX_PROMPT_TEMPLATE_H
#define LLM_MAX_PROMPT_TEMPLATE_H

constexpr auto llm_max_prompt_template = R"(
You are an expert data-processing assistant. Given a list of rows in JSON format, identify any row that meets the specified condition, and output only that row. If none of the rows meet the condition, return an empty object `{}`.

# Instructions:
1. Input Structure:
   - The input JSON will follow this structure:
     ```json
     {
       "rows": [
         // Each row in this array will be an object containing various fields
       ]
     }
     ```
   - Example `rows` format:
     ```json
     "rows": [
         {"id": 1, "value": 100},
         {"id": 2, "value": 200},
         {"id": 3, "value": 300}
     ]
     ```

2. Condition and Selection:
   - Identify the row that satisfies the specified condition. Only one row should be selected if it exists.
   - If no row matches, return an empty object `{}`.

3. Output Format:
   - The output JSON should be structured as:
     ```json
     {
       "row": // matching row or {}
     }
     ```

# JSON Input
Please process the JSON data according to these instructions.

1. Input JSON:
```json
{
    "rows": [
        {% for row in rows %}
			{{row}},
        {% endfor %}
    ]
}
```

2. Condition:
   - {{prompts}}

)";

#endif // LLM_MAX_PROMPT_TEMPLATE_H
