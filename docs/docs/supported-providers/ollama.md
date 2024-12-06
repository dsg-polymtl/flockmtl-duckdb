---
title: Ollama
sidebar_position: 1
---

# Setting Up and Using Ollama

## 1. Prerequisites

1. **Install Ollama**: Ensure Ollama is installed. If not, download it from the [Ollama download page](https://ollama.com/download).
2. **Download the Model**: Confirm that the required model is available on your system.

Here’s the modified section for configuring the Ollama URL:

## 2. Configuring Your Ollama URL

Specify the Ollama URL using the DuckDB secret manager. Use the following query:

```sql
CREATE SECRET (
    TYPE OLLAMA,
    API_URL '127.0.0.1:11434'
);
```

In this example, Ollama is running locally, and the default URL is `127.0.0.1:11434`. For more details, refer to the [secret management](/docs/resource-management/secret-management) section.

## 3. Adding an Ollama Model

Register your Ollama model in the model manager with this query:

```sql
CREATE MODEL('llama-model', 'llama3.2', 'ollama', {"context_window": 128000, "max_output_tokens": 2048});
```

For more information on managing models, refer to the [model management](/docs/resource-management/model-management) section.

## 4. Practical Example

Generate product descriptions using the `llama-model` for entries in a `products` table:

```sql
SELECT llm_complete(
    {'model_name': 'llama-model'},
    {'prompt': 'Generate a description for the next product based on its product name.'},
    {'product_name': product_name}
) AS product_description
FROM products;
```
