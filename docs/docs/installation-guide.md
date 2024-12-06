---
sidebar_position: 5
---

# Installation Guide

Follow the steps below to install and set up FlockMTL on your system.

### Prerequisites

Before installing FlockMTL, ensure that you have DuckDB installed. You can install DuckDB by following the official [DuckDB installation guide](https://duckdb.org/docs/installation).

Additionally, make sure that you have access to an OpenAI API key to use text generation and embedding features.

### Installation Steps

1. **Install the FlockMTL Extension**

   To install FlockMTL, run the following SQL commands in your DuckDB instance:

   ```sql
   INSTALL flockmtl FROM community;
   LOAD flockmtl;
   ```

   This will install the FlockMTL extension and load it into your DuckDB environment.

2. **Set Up the OpenAI API Key**

   To use functions that rely on LLM models (such as `llm_complete` or `llm_embedding`), you need to create a secret using DuckDB's secrets manager. Refer to the [DuckDB secrets manager guide](https://duckdb.org/docs/configuration/secrets_manager.html) for more details.

   To create an OpenAI API key as a secret, use the following query:

   ```sql
   CREATE SECRET (
       TYPE OPENAI
       API_KEY 'your-api-key'
   );
   ```

   This will create a default, temporary OpenAI API key stored in memory.

   If you want the key to persist across sessions, use the following query:

   ```sql
   CREATE PERSISTENT SECRET (
       TYPE OPENAI
       API_KEY 'your-api-key'
   );
   ```

3. **Verify Installation**

   To verify that the extension is installed correctly, you can run a simple query:

   ```sql
   SELECT
        product_name,
        llm_complete(
            {'model_name': 'gpt-4'},
            {'prompt': 'Generate a description for each product based on its product name.'},
            {'product_name': product_name})
        AS product_description
    FROM products;
   ```

   If everything is set up correctly, you should see the generated output for the product description.

   - **Output Example**

    <br/> 
    | product_name       | product_description                                                                 |
    |--------------------|--------------------------------------------------------------------------------------|
    | Running Shoes      | "These lightweight running shoes offer superior comfort and breathability for long-distance runners. Featuring a cushioned sole and durable upper, they ensure maximum performance on various terrains." |
    | Wireless Headphones| "These wireless headphones provide exceptional sound quality and comfort, featuring noise cancellation and a long battery life perfect for travel or daily use." |
    | Smart Watch        | "This smart watch tracks your fitness, heart rate, and sleep patterns while providing smart notifications, making it the perfect companion for a healthy and connected lifestyle." |
    | Leather Jacket     | "Crafted from premium leather, this jacket combines timeless style with durability, providing a sophisticated look for any occasion." |
    | Coffee Maker       | "Brew your perfect cup every time with this easy-to-use coffee maker, featuring programmable settings and a compact design ideal for any kitchen." |
