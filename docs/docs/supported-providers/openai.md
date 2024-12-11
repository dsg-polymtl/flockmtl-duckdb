---
title: OpenAI  
sidebar_position: 2  
---

# Configuring and Using OpenAI Services  

## 1. Setting Up the OpenAI API Key  

Configure your OpenAI API key securely using the DuckDB secret manager with one of the following SQL commands:  

### 1.1. Standard OpenAI Configuration  

```sql  
CREATE SECRET (
    TYPE OPENAI,
    API_KEY 'your-api-key'
);  
```  

### 1.2. OpenAI-Compatible Providers  

For providers with OpenAI-compatible APIs, specify the `BASE_URL` along with your API key:  

```sql  
CREATE SECRET (
    TYPE OPENAI,
    BASE_URL 'your-provider-url',
    API_KEY 'your-api-key'
);  
```  

### 1.3. Example: GROQ Provider  

To configure the GROQ provider using the base URL `https://api.groq.com/openai/v1/`, use the following command:  

```sql  
CREATE SECRET groq (
    TYPE OPENAI,
    BASE_URL 'https://api.groq.com/openai/v1/',
    API_KEY 'your-api-key'
);  
```  

For more information on managing secrets, refer to the [secret management](/docs/resource-management/secret-management) section.  

## 2. Defining a Model  

Define a model for specific tasks, specifying its name, provider, and parameters.  

### 2.1. Example: OpenAI GPT-4 Model  

```sql  
CREATE MODEL(
    'description-model',
    'gpt-4o',
    'openai',
    {"context_window": 128000, "max_output_tokens": 4096}
);  
```  

### 2.2. Example: GROQ-Compatible Model  

For a GROQ-compatible model (`llama3.3`, `llama-3.3-70b-versatile`):  

```sql  
CREATE MODEL(
    'description-model',
    'llama-3.3-70b-versatile',
    'openai',
    {"context_window": 128000, "max_output_tokens": 32000}
);  
```  

For more information on managing models, refer to the [model management](/docs/resource-management/model-management) section.

## 3. Practical Examples  

### 3.1. Using OpenAI Services

Generate product descriptions using the `description-model` with OpenAI services:  

```sql  
SELECT llm_complete(
    {'model_name': 'description-model'},
    {'prompt': 'Generate a description for the next product based on its product name.'},
    {'product_name': product_name}
) AS product_description  
FROM products;  
```  

### 3.2. Using GROQ Provider  

Generate product descriptions using the `description-model` and the GROQ provider:  

```sql  
SELECT llm_complete(
    {'model_name': 'description-model', 'secret_name': 'groq' },
    {'prompt': 'Generate a description for the next product based on its product name.'},
    {'product_name': product_name}
) AS product_description  
FROM products;  
```  
