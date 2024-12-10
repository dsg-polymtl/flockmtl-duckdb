---
title: Model Management
sidebar_position: 1
---

# Model Management

The **Model Management** section provides guidance on how to manage and configure models for **analytics and semantic analysis tasks** within FlockMTL. These tasks involve processing and analyzing text, embeddings, and other data types using pre-configured models, either system-defined or user-defined, based on specific use cases. Each database is configured with its own model management table during the initial load.

## 1. Model Configuration

Models are stored in a table with the following structure:

| **Column Name**     | **Description**                                                                     |
| ------------------- | ----------------------------------------------------------------------------------- |
| **Model Name**      | Unique identifier for the model                                                     |
| **Model Type**      | Specific model type (e.g., `gpt-4`, `llama3`)                                       |
| **Provider**        | Source of the model (e.g., `openai`, `azure`, `ollama`)                             |
| **Model Arguments** | JSON configuration parameters such as `context_window` size and `max_output_tokens` |

## 2. Management Commands

- Retrieve all available models

```sql
GET MODELS;
```

- Retrieve details of a specific model

```sql
GET MODEL 'model_name';
```

- Create a new user-defined model

```sql
CREATE MODEL('model_name', 'model', 'provider', {'context_window': 128000, 'max_output_tokens': 8000})
```

- Modify an existing user-defined model

```sql
UPDATE MODEL('model_name', 'model', 'provider', {'context_window': 128000, 'max_output_tokens': 8000});
```

- Remove a user-defined model

```sql
DELETE MODEL 'model_name';
```

## 3. SQL Query Examples

### Semantic Text Completion

```sql
SELECT llm_complete(
    {'model_name': 'gpt-4'},
    {'prompt_name': 'product-description'},
    {'input_text': product_description}
) AS generated_description
FROM products;
```

### Semantic Search

```sql
SELECT llm_complete(
    {'model_name': 'semantic_search_model'},
    {'prompt_name': 'search-query'},
    {'search_query': query}
) AS search_results
FROM search_data;
```
