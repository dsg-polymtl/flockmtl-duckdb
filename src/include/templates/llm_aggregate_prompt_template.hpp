#ifndef LLM_AGGREGATE_PROMPT_TEMPLATES_HPP
#define LLM_AGGREGATE_PROMPT_TEMPLATES_HPP

#include <string>
#include <templates/llm_prompt_template.hpp>

constexpr auto aggregate_function_instructions =
    R"(- For each tuple in the provided data, evaluate the relevant attribute(s) based on the user prompt.
- After evaluating each tuple, aggregate the results according to the user's query. This could involve different operations depending on the prompt.
- The aggregation should summarize the responses of all tuples into a single final result that answers the user's question as a whole.
- The final result should be returned according to the expected response format.
)";

constexpr auto llm_reduce_reponse_format =
    R"(Return a single, coherent output that synthesizes the most relevant information from the tuples provided. Respond in the following JSON format. **Do not add explanations or additional words beyond the summarized content.**

Response Format:

```json
{
  "output": <summarized content here>
}
```)";

constexpr auto llm_first_or_last_response_format =
    R"(Identify the {{RELEVANCE}} relevant tuple based on the provided user prompt from the list of tuples. Output the index of this single tuple as a JSON object in the following format:

```json
{
  "selected": flockmtl_tuple_id
}
```)";

constexpr auto llm_rerank_response_format =
    R"(Rank the tuples in descending order of relevance according to the user prompt. Output the response strictly as a JSON object in the following format:

```json
{
  "ranking": [flockmtl_tuple_id1, flockmtl_tuple_id2, flockmtl_tuple_id3, ...]
}
```

Include all tuple IDs in the response, listed without any additional text or explanation.)";

enum class AggregateFunctionType { REDUCE, FIRST, LAST, RERANK };

class AggregatePromptTemplate {
public:
    static std::string GetResponseFormat(AggregateFunctionType option);
    static std::string GetPromptTemplate(AggregateFunctionType option);
    static std::string GetPrompt(std::string user_prompt, std::string &tuples, AggregateFunctionType option);
};

#endif // LLM_AGGREGATE_PROMPT_TEMPLATES_HPP
