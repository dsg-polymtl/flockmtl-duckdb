#pragma once

#include <string>

namespace flockmtl {

constexpr auto META_PROMPT =
    "You are a semantic analysis tool for DBMS. The tool will analyze each tuple in the provided data and respond to "
    "user requests based on this context.\n\nUser Prompt:\n\n- {{USER_PROMPT}}\n\nTuples "
    "Table:\n\n{{TUPLES}}\n\nInstructions:\n\n{{INSTRUCTIONS}}\n\nExpected Response Format:\n\n{{RESPONSE_FORMAT}}";

class INSTRUCTIONS {
public:
    static constexpr auto SCALAR_FUNCTION =
        "- The response should be directly relevant to each tuple without additional formatting, purely answering the "
        "prompt as if each tuple were a standalone entity.\n- Use clear, context-relevant language to generate a "
        "meaningful and concise answer for each tuple.";
    static constexpr auto AGGREGATE_FUNCTION =
        "- For each tuple in the provided data, evaluate the relevant attribute(s) based on the user prompt.\n- After "
        "evaluating each tuple, aggregate the results according to the user's query. This could involve different "
        "operations depending on the prompt.\n- The aggregation should summarize the responses of all tuples into a "
        "single final result that answers the user's question as a whole.\n- The final result should be returned "
        "according to the expected response format.";

    template <typename FunctionType>
    static std::string Get(const FunctionType option);
};

class RESPONSE_FORMAT {
public:
    // Scalar Functions
    static constexpr auto COMPLETE_JSON =
        "Return a single, coherent output that synthesizes the most relevant information from the tuples provided. "
        "Respond in the following JSON format. **Do not add explanations or additional words beyond the summarized "
        "content.**\n\nResponse Format:\n\n```json\n{\n  \"output\": <summarized content here>\n}\n```";
    static constexpr auto COMPLETE =
        "Return a single, coherent output that synthesizes the most relevant information from the tuples provided. "
        "**Do not add explanations or additional words beyond the summarized content.**";
    static constexpr auto FILTER =
        "Return a list of all tuples that meet the criteria specified in the user prompt. Respond in the following "
        "JSON format. **Do not add explanations or additional words beyond the summarized content.**\n\nResponse "
        "Format:\n\n```json\n{\n  \"output\": [flockmtl_tuple_id1, flockmtl_tuple_id2, flockmtl_tuple_id3, "
        "...]\n}\n```";

    // Aggregate Functions

    static constexpr auto REDUCE =
        "Return a single, coherent output that synthesizes the most relevant information from the tuples provided. "
        "Respond in the following JSON format. **Do not add explanations or additional words beyond the summarized "
        "content.**\n\nResponse Format:\n\n```json\n{\n  \"output\": <summarized content here>\n}\n```";
    static constexpr auto FIRST_OR_LAST =
        "Identify the {{RELEVANCE}} relevant tuple based on the provided user prompt from the list of tuples. Output "
        "the index of this single tuple as a JSON object in the following format:\n\n```json\n{\n  \"selected\": "
        "flockmtl_tuple_id\n}\n```";
    static constexpr auto RERANK =
        "Rank the tuples in descending order of relevance according to the user prompt. Output the response strictly "
        "as a JSON object in the following format:\n\n```json\n{\n  \"ranking\": [flockmtl_tuple_id1, "
        "flockmtl_tuple_id2, flockmtl_tuple_id3, ...]\n}\n```\n\nInclude all tuple IDs in the response, listed without "
        "any additional text or explanation.";

    template <typename FunctionType>
    static std::string Get(const FunctionType option);
};

} // namespace flockmtl
