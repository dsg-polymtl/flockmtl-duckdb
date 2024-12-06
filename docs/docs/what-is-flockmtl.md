---
sidebar_position: 1
---

# What is FlockMTL?

**FlockMTL** is an extension for DuckDB designed to bring analytics & semantic analysis capabilities directly into your SQL queries. It deeply integrates capabilities of language models and retrieval-augmented generation using a set of map and reduce functions such as:

- [`llm_complete`](/docs/scalar-map-functions/llm-complete)
- [`llm_complete_json`](/docs/scalar-map-functions/llm-complete-json)
- [`llm_filter`](/docs/scalar-map-functions/llm-filter)
- [`llm_embedding`](/docs/scalar-map-functions/llm-embedding)
- [`fusion_relative`](/docs/scalar-map-functions/)
- [`llm_reduce`](/docs/aggregate-reduce-functions/llm-reduce)
- [`llm_rerank`](/docs/aggregate-reduce-functions/llm-rerank)
- [`llm_first`](/docs/aggregate-reduce-functions/llm-first)
- [`llm_last`](/docs/aggregate-reduce-functions/llm-last)

This allows users to perform tasks such as text generation, summarization, classification, filtering, fusion, and embedding generation and even end-to-end RAG pipelines within DuckDB.

We believe that relational DBMSs and LLMs are a match made in heaven. We are leaning on the tradition of declarative interfaces to unburden users from lower-level implementation details. Users can query both structured and unstructred datasets while combining analytics and semantic analysis directly within SQL.
