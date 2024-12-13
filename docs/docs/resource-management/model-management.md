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

## 2. Introduction to Global and Local Models

FlockMTL supports two types of models:

*   **Global Models**: Created using `CREATE GLOBAL MODEL`. These are shared across different databases.
*   **Local Models**: Created using `CREATE LOCAL MODEL` or `CREATE MODEL` (default if no type is specified). These are limited to a single database.

## 3. Management Commands

### Create Models

- Create a global model:

```sql
CREATE GLOBAL MODEL('model_name', 'model_type', 'provider', {'context_window': 128000, 'max_output_tokens': 8000})
```

- Create a local model (default if no type is specified):

```sql
CREATE LOCAL MODEL('model_name', 'model_type', 'provider', {'context_window': 128000, 'max_output_tokens': 8000})
CREATE MODEL('model_name', 'model_type', 'provider', {'context_window': 128000, 'max_output_tokens': 8000})
```

### Retrieve Models

- Retrieve all available models:

```sql
GET MODELS;
```

- Retrieve details of a specific model:

```sql
GET MODEL 'model_name';
```

### Update Models

- Update an existing model:

```sql
UPDATE MODEL('model_name', 'model_type', 'provider', {'context_window': 128000, 'max_output_tokens': 8000});
```

- Toggle a model's state between global and local:

```sql
UPDATE MODEL 'model_name' TO GLOBAL;
UPDATE MODEL 'model_name' TO LOCAL;
```
- Remove a user-defined model

```sql
DELETE MODEL 'model_name';
```

## 4. SQL Query Examples

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
