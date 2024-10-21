# FlockMTL Extension

FlockMTL is a DuckDB extension that integrates language model (LLM) capabilities directly into your queries and workflows. This experimental extension enables DuckDB users to add semantic analysis (classification, filtering, completion, all w/ structured output) and embeddings using GPT models—all from within SQL commands. Following the tradition of declarativity, we introduce an administrative view of `MODEL`(s) and `PROMPT`(s) akin to `TABLE`(s).

---

## Table of Contents

- [FlockMTL Extension](#flockmtl-extension)
  - [Table of Contents](#table-of-contents)
    - [Installation](#installation)
    - [Documentation](#documentation)
      - [**1. Text Generation with `llm_complete`**](#1-text-generation-with-llm_complete)
      - [**2. JSON Output with `llm_complete_json`**](#2-json-output-with-llm_complete_json)
      - [**3. Filtering with `llm_filter`**](#3-filtering-with-llm_filter)
      - [**4. Text Embedding with `llm_embedding`**](#4-text-embedding-with-llm_embedding)
      - [**5. Prompt Management**](#5-prompt-management)
      - [**6. Model Management**](#6-model-management)

---

### Installation

Install the extension as a [Community Extension](https://community-extensions.duckdb.org/).

Run:
  ```sql
  INSTALL flockmtl FROM community;
  LOAD flockmtl;
  ```

---

### Documentation

Next, we demonstrate how to use the **DuckDB LLM Extension** and an example we would like to analyze product reviews. Specifically, we’ll focus on generating text, classifying reviews as positive or negative, and working with text embeddings using a `product_reviews` table with attributes `review_text`, `review_id`, and `customer_name`.

#### **1. Text Generation with `llm_complete`**

`llm_complete` generates text based on a given prompt and LLM model. The following examples show how this functionality can be used in real-world scenarios.

- **Basic Text Generation – Product Description**:  
  Imagine you have a list of product names, and you want to automatically generate product descriptions based on the product name.
  ```sql
  SELECT product_name, llm_complete('generate_product_description', 'default', {'product_name': product_name}, {'max_tokens': 150}) AS product_description 
  FROM products;
  ```
  **Result**:
  | product_name | product_description |
  |--------------|---------------------|
  | Running Shoes | "These lightweight running shoes offer superior comfort and breathability for long-distance runners. Featuring a cushioned sole and durable upper, they ensure maximum performance on various terrains." |

- **Example: Sentiment Classification – Complex Review**:  
  Classify whether a review is positive based on more than just the review text, such as review length and other metadata. This could help you get a more nuanced classification.
  ```sql
  SELECT review_id, llm_complete('is_positive', 'default', {'text': review_text, 'length': LENGTH(review_text)}, {'max_tokens': 100}) AS results 
  FROM product_reviews;
  ```
  **Result**:
  | review_id | results |
  |-----------|---------|
  | 12345     | "Positive - Great in-depth feedback, long review indicating high engagement." |

- **Example: Generate Responses to Negative Reviews**:  
  Automatically generate personalized responses to negative reviews, allowing your team to respond quickly and professionally.
  ```sql
  SELECT review_id, llm_complete('generate_response', 'default', {'review': review_text, 'sentiment': 'negative'}, {'max_tokens': 100}) AS response 
  FROM product_reviews
  WHERE llm_filter('is_negative', 'default', {'text': review_text});
  ```
  **Result**:
  | review_id | response |
  |-----------|----------|
  | 54321     | "We're sorry to hear about your experience. We value your feedback and would like to offer a solution. Please contact our support team for further assistance." |

---

#### **2. JSON Output with `llm_complete_json`**

`llm_complete_json` is useful when you want structured output, such as detailed classification or multi-part answers.

- **Example: Sentiment and Detailed Analysis**:  
  Classify whether a review is positive or negative and provide a detailed analysis of the sentiment in JSON format.
  ```sql
  SELECT review_id, llm_complete_json('detailed_sentiment_analysis', 'default', {'text': review_text}, {'max_tokens': 200}) AS analysis 
  FROM product_reviews;
  ```
  **Result**:
  | review_id | analysis |
  |-----------|----------|
  | 112233    | {"sentiment": "negative", "confidence": 0.85, "keywords": ["bad service", "slow response"]} |

- **Example: Review Summary in JSON Format**:  
  Summarize a review and return a JSON output with sentiment, summary, and key points.
  ```sql
  SELECT review_id, llm_complete_json('summarize_review', 'default', {'text': review_text}, {'max_tokens': 150}) AS summary 
  FROM product_reviews;
  ```
  **Result**:
  | review_id | summary |
  |-----------|---------|
  | 332211    | {"summary": "The customer experienced a slow response time from support but was satisfied with the product quality overall.", "sentiment": "mixed", "key_points": ["slow response", "good product quality"]} |

---

#### **3. Filtering with `llm_filter`**

Use `llm_filter` to filter records based on LLM-powered classifications, such as identifying specific sentiments or themes in product reviews.

- **Example: Filter Reviews Mentioning Specific Features**:  
  Filter reviews to return only those that mention specific product features, such as "battery life" or "performance."
  ```sql
  SELECT review_id, customer_name, review_text 
  FROM product_reviews 
  WHERE llm_filter('mentions_feature', 'default', {'text': review_text, 'feature': 'battery life'});
  ```
  **Result**:
  | review_id | customer_name | review_text |
  |-----------|----------------|-------------|
  | 56789     | John Doe       | "The battery life on this phone is fantastic! It lasted me a full two days without needing a recharge." |

- **Example: Filter Reviews with High Confidence in Sentiment**:  
  Filter reviews based on the confidence level of sentiment classification, showing only those with high confidence in the result.
  ```sql
  SELECT review_id, customer_name, review_text 
  FROM product_reviews 
  WHERE llm_filter('is_positive', 'default', {'text': review_text, 'confidence_threshold': 0.9});
  ```
  **Result**:
  | review_id | customer_name | review_text |
  |-----------|----------------|-------------|
  | 99887     | Jane Smith     | "Absolutely loved this product! Exceeded all my expectations." |

---

#### **4. Text Embedding with `llm_embedding`**

`llm_embedding` generates vector embeddings for text, which can be used for tasks like semantic similarity, clustering, or advanced search.

- **Example: Find Reviews Similar to a Target Review**:  
  Generate embeddings for each review and compare them to a target review to find similar reviews based on their semantic content.
  ```sql
  WITH input_embedding AS (
      SELECT llm_embedding({'text': 'Comfortable to wear, but the sound quality is just average. Expected better for the price.'},
               'text-embedding-3-small') AS embedding
  ),
  review_embeddings AS (
      SELECT 
          review_id, 
          array_distance(input_embedding.embedding::DOUBLE[1536], 
                         llm_embedding({'text': review_text}, 'text-embedding-3-small')::DOUBLE[1536]) AS similarity
      FROM product_reviews, input_embedding
  )
  SELECT review_id AS similar_review_id
  FROM review_embeddings
  ORDER BY similarity
  LIMIT 1;
  ```
  **Result**:
  | similar_review_id |
  |-------------------|
  | 4                 |
  
---

You can manage LLM prompts and models dynamically in DuckDB using the following commands:

#### **5. Prompt Management**
- **Get All Prompts**:  
  ```sql
  GET PROMPTS;
  ```

- **Get Specific Prompt**:  
  Retrieve the content of a specific prompt by name.
  ```sql
  GET PROMPT <prompt_name>;
  ```

- **Create a New Prompt**:  
  ```sql
  CREATE PROMPT(<prompt_name>, <prompt_text>);
  ```

- **Update an Existing Prompt**:  
  ```sql
  UPDATE PROMPT(<prompt_name>, <new_prompt_text>);
  ```

- **Delete a Prompt**:  
  ```sql
  DELETE PROMPT <prompt_name>;
  ```

#### **6. Model Management**
- **Get All Models**:  
  ```sql
  GET MODELS;
  ```

- **Get Specific Model**:  
  Retrieve the details of a specific model by name.
  ```sql
  GET MODEL <model_name>;
  ```

- **Create a New Model**:  
  ```sql
  CREATE MODEL(<model_name>, <model_type>, <max_tokens>);
  ```

- **Update a Model**:  
  ```sql
  UPDATE MODEL(<model_name>, <model_type>, <max_tokens>);
  ```

- **Delete a Model**:  
  ```sql
  DELETE MODEL <model_name>;
  ```