---
title: fusion_relative
sidebar_position: 5
---

# `fusion_relative` Function

The `fusion_relative` function combines two numerical values (e.g., BM25 and embedding scores) into a single, unified relevance score. This function is ideal for hybrid search cases where multiple ranking scores are combined to produce the best-fit result.

## 1. Basic Usage Examples

### 1.1 Simple Example: Combining Two Scores

```sql
SELECT fusion_relative(2::DOUBLE, 3::DOUBLE);
```

**Description**: In this simple example, the `fusion_relative` function takes two numeric values (2.0 and 3.0) as inputs and returns the higher of the two values as the output.

**Output**:

```sql
3.0
```

**Explanation**: The function returns `3.0` because it applies a fusion algorithm that combines the scores, and in this case, it selects the higher value.

### 1.2 Example with BM25 and Embedding Scores

```sql
SELECT
    fusion_relative(bm25_score::DOUBLE, embedding_score::DOUBLE) AS combined_score
FROM search_results;
```

**Description**: This query combines the BM25 score (`bm25_score`) and the embedding score (`embedding_score`) for each row in the `search_results` table. The function merges both scores into a single `combined_score`.

## 2. Advanced Example

```sql
WITH combined_scores AS (
    SELECT
        index_column,
        fusion_relative(bm25_score::DOUBLE, embedding_score::DOUBLE) AS combined_score
    FROM search_results
)
SELECT
    index_column,
    combined_score
FROM combined_scores
ORDER BY combined_score DESC;
```

**Description**: In this advanced example, the query merges BM25 and embedding scores into a `combined_score` for each document and sorts the results by `combined_score` in descending order, prioritizing more relevant documents.

## 3. Input Parameters

The `fusion_relative` function takes two numerical inputs:

### 3.1 First Numerical Input

- **Parameter**: First numeric value (e.g., BM25 score)
- **Description**: The first value used in the fusion algorithm.
- **Example**:
  ```sql
  2::DOUBLE
  ```

### 3.2 Second Numerical Input

- **Parameter**: Second numeric value (e.g., embedding score)
- **Description**: The second value used in the fusion algorithm.
- **Example**:
  ```sql
  3::DOUBLE
  ```

## 4. Output

The function returns a **numerical value** that represents the combined relevance score, calculated by the fusion algorithm. Typically, it returns the higher of the two input values, although the exact behavior may depend on the fusion method used.

**Example Output**:  
For the input values `2.0` and `3.0`, the result would be:

- **Input**:

  - `2.0`
  - `3.0`

- **Output**:
  ```sql
  3.0
  ```

This output represents the highest relevance score, suitable for scenarios like hybrid search where multiple ranking methods are combined.
