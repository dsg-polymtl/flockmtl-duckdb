---
title: Prompt Management
sidebar_position: 2
---

# Prompt Management

The **Prompt Management** section provides guidance on how to manage and configure prompts for **analytics and semantic analysis tasks** within FlockMTL. Prompts guide models in generating specific outputs for tasks like content generation, summarization, and ranking. Each database is configured with its own prompt management table during the initial load.

### Prompt Table Structure

| **Column Name** | **Description**                   |
| --------------- | --------------------------------- |
| **prompt_name** | Unique identifier for the prompt  |
| **prompt**      | Instruction content for the model |
| **updated_at**  | Timestamp of the last update      |
| **version**     | Version number of the prompt      |

## 1. Introduction to Global and Local Prompts

FlockMTL supports two types of prompts:

*   **Global Prompts**: Created using `CREATE GLOBAL PROMPT`. These are shared across different databases.
*   **Local Prompts**: Created using `CREATE LOCAL PROMPT` or `CREATE PROMPT` (default if no type is specified). These are limited to a single database.

## 2. Management Commands

### Create Prompts

*   Create a global prompt:

```sql
CREATE GLOBAL PROMPT('prompt_name', 'prompt');
```

- Create a local prompt (default if no type is specified):

```sql
CREATE LOCAL PROMPT('prompt_name', 'prompt');
CREATE PROMPT('prompt_name', 'prompt');
```

### Retrieve Prompts

- Retrieve all available prompts:

```sql
GET PROMPTS;
```

- Retrieve details of a specific prompt:

```sql
GET PROMPT 'prompt_name';
```

### Update Prompts

- Update an existing prompt:

```sql
UPDATE PROMPT('prompt_name', 'prompt');
```

- Toggle a prompt's state between global and local:

```sql
UPDATE PROMPT 'prompt_name' TO GLOBAL;
UPDATE PROMPT 'prompt_name' TO LOCAL;
```

- Remove a prompt

```sql
DELETE PROMPT 'prompt_name';
```

## 3. SQL Query Examples

### Semantic Text Completion

Generate a description for products using a predefined prompt:

```sql
SELECT llm_complete(
    {'model_name': 'gpt-4'},
    {'prompt_name': 'product-description'},
    {'input_text': product_description}
) AS generated_description
FROM products;
```

### Review Summary

Generate a summary for customer reviews using a specific prompt version:

```sql
SELECT llm_complete(
    {'model_name': 'semantic_search_model'},
    {'prompt_name': 'customer-review-summary', 'version': 3},
    {'customer_review': review_text}
) AS review_summary
FROM reviews;
```
