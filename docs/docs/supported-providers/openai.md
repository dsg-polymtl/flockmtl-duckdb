---
title: OpenAI
sidebar_position: 2
---

# Configuring and Using OpenAI Services

## 1. Setting Up the OpenAI API Key

Configure your OpenAI API key securely using the DuckDB secret manager with this SQL command:

```sql
CREATE SECRET (
    TYPE OPENAI
    API_KEY 'your-api-key'
);
```

For more information on managing secrets, refer to the [secret management](/docs/resource-management/secret-management) section.

## 2. Defining an OpenAI Model

Create a model for specific tasks, such as:

```sql
CREATE MODEL('description-model', 'gpt-4o', 'openai',
    {"context_window": 128000, "max_output_tokens": 4096});
```

For more information on managing models, refer to the [model management](/docs/resource-management/model-management) section.

## 3. Practical Example

Generate product descriptions using the `description-model` for entries in a `products` table:

```sql
SELECT llm_complete(
    {'model_name': 'description-model'},
    {'prompt': 'Generate a description for the next product based on its product name.'},
    {'product_name': product_name}
) AS product_description
FROM products;
```
