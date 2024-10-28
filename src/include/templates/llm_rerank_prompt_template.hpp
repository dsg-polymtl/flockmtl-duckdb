#ifndef LLM_RERANK_PROMPT_TEMPLATE_H
#define LLM_RERANK_PROMPT_TEMPLATE_H

constexpr auto llm_rerank_prompt_template = R"(
You are RankLLM, an intelligent assistant that can rank passages based on their relevancy to the query.

I will provide you with some tuples, each indicated by a numerical identifier []. Rank the passages based on their relevance to the search query:

Tuples:

{% for tuple in tuples %}
[{{tuple.id}}] {{tuple.content}}
{% endfor %}

Search Query: {{user_prompt}}

Rank the tuples above based on their relevance to the search query. All the passages should be included and listed using identifiers, in descending order of relevance. The output format should be in JSON list [id_1, ..., id_n], e.g., [22, 33, ..., 3], Only respond with the ranking results, do not say any word or explain.

Response Format:

{
  "ranking": [id_1, ..., id_n]
}

)";

#endif // LLM_RERANK_PROMPT_TEMPLATE_H
