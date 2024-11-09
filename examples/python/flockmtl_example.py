#The following steps need to be followed for setting up FlocktMTL with python

#Step 1: Store API secrets in a .env file, and load it
#We store the OpenAI API key in our .env file for the tutorial. and use the python-dotenv package to read this secret. Any other suitable method can also be used

#Step 2: Install DuckDB for python.
#For FlockMTL v0.1.0 "Schwartz Deli", DuckDB version >= 1.1.2 is required

#Step 3: Create DuckDB database.
#We use a temporary in-memory database. Persistent database can also be used. Please see https://duckdb.org/docs/connect/overview.html for detailed methods to create databases

#Step 4: Install FlockMTL
#FlockMTL needs to be installed once per new DuckDB installation. DuckDB installs the latest release from DuckDB Community extensions (https://community-extensions.duckdb.org/)

#Step 5: Load FlockMTL
#FlockMTL needs to be loaded every time when the script is run 

#Step 6: Use FlockMTL
#After step 5, FlockMTL is installed and ready to use. We create a new table called product_reviews, and use LLM function calls to extract semantic details from it

import duckdb
from dotenv import load_dotenv
import os

# Step 1: Load environment variables
def load_env():
    load_dotenv()
    print("Environment variables loaded.")

# Step 2: Connect to DuckDB
def connect_to_duckdb():
    # Using an in-memory database; switch to file-based by changing ':memory:'
    connection = duckdb.connect(':memory:', config={'allow_unsigned_extensions': 'true'})
    print("Connected to DuckDB.")
    return connection

# Step 3: Install FlockMTL
def install_flockmtl(con):
    con.execute("INSTALL flockmtl FROM community")
    con.execute("LOAD flockmtl")
    print("FlockMTL installed and loaded.")

# Step 4: Create table and load CSV data
def load_csv_to_table(con, csv_path='products.csv'):
    con.execute(f"CREATE TABLE product_reviews AS SELECT * FROM read_csv_auto('{csv_path}')")
    print("Data loaded into product_reviews table.")

# Step 5: Display default models
def display_models(con):
    models = con.execute("GET MODELS;").fetchall()
    print("Default models supported by FlockMTL:")
    for model in models:
        print(model)

# Step 6: Create and register sentiment analysis prompt
def create_sentiment_analysis_prompt(con):
    prompt = """
    Analyze the sentiment of the following product review. Consider both the review text and the star rating. 
    Provide a brief sentiment label (positive, negative, or neutral) and a short explanation for your decision.

    Review: {review}
    Star Rating: {rating}

    Output your response in the following JSON format:
    {
        "sentiment": "positive/negative/neutral",
        "explanation": "Brief explanation of the sentiment analysis"
    }
    """
    con.execute(f"CREATE PROMPT ('sentiment-analysis', '{prompt}');")
    print("Sentiment analysis prompt created.")

# Step 7: Perform sentiment analysis
def perform_sentiment_analysis(con):
    query = """
    CREATE TABLE sentiment_analysis AS
    WITH sentiment_analysis AS (
        SELECT 
            ProductID AS product_id, 
            ID AS review_id,
            Review AS review_text, 
            Rating AS star_rating, 
            llm_complete_json('sentiment-analysis', 'default', {'review': review_text, 'rating': star_rating}) AS sentiment_json
        FROM 
            product_reviews
    )
    SELECT * 
    FROM sentiment_analysis;
    """
    con.execute(query)
    print("Sentiment analysis completed and stored in sentiment_analysis table.")

# Step 8: Create and register high-impact review prompt
def create_high_impact_review_prompt(con):
    prompt = """
    Determine if the given review is a high-impact review that provides valuable insights. Consider the following factors:

    1. Sentiment: {sentiment}
    2. Star Rating: {rating}
    3. Review Length: {review_length}

    A high-impact review typically has:

    - A strong sentiment (very positive or very negative)
    - An extreme rating (1-2 or 4-5 stars)
    - Sufficient length to provide detailed feedback (usually more than 50 words)

    Output your decision as a boolean true or false.
    """
    con.execute(f"CREATE PROMPT ('is-high-impact-review', '{prompt}');")
    print("High-impact review prompt created.")

# Step 9: Filter high-impact reviews
def filter_high_impact_reviews(con):
    query = """
    CREATE TABLE filtered_reviews AS
    WITH filtered_reviews AS (
        SELECT 
            * 
        FROM 
            sentiment_analysis 
        WHERE 
            llm_filter('is-high-impact-review', 'gpt-4o', {
                'sentiment': sentiment_json, 
                'rating': star_rating, 
                'review_length': LENGTH(review_text)
            })
    )
    SELECT * 
    FROM filtered_reviews;
    """
    con.execute(query)
    print("Filtered high-impact reviews and stored in filtered_reviews table.")

# Step 10: Create and register theme extraction prompt
def create_extract_themes_prompt(con):
    prompt = """
    Analyze the following sentiment analysis JSON. Identify and extract key themes or topics discussed in the product review. 
    Output the themes in a JSON array format.
    
    Sentiment Analysis JSON: {sentiment_json}
    
    Output your response in the following JSON format:
    {
        "themes": ["theme1", "theme2", "theme3"]
    }
    """
    con.execute(f"CREATE PROMPT ('extract-themes', '{prompt}');")
    print("Theme extraction prompt created.")

# Step 11: Extract themes from reviews
def extract_themes(con):
    query = """
    CREATE TABLE themes_extracted AS
    WITH themes_extracted AS (
        SELECT 
            product_id, 
            review_id, 
            review_text,
            star_rating,
            llm_complete_json('extract-themes', 'gpt-4o', {'sentiment_json': sentiment_json}) AS themes
        FROM 
            filtered_reviews
    )
    SELECT * 
    FROM themes_extracted;
    """
    con.execute(query)
    print("Themes extracted and stored in themes_extracted table.")

# Main function to run all steps
def main():
    load_env()
    con = connect_to_duckdb()
    install_flockmtl(con)
    load_csv_to_table(con)
    display_models(con)
    create_sentiment_analysis_prompt(con)
    perform_sentiment_analysis(con)
    create_high_impact_review_prompt(con)
    filter_high_impact_reviews(con)
    create_extract_themes_prompt(con)
    extract_themes(con)

    # Optionally, fetch and display the final results
    results = con.execute("SELECT * FROM themes_extracted;").fetchall()
    print("Themes extracted from reviews:")
    for row in results:
        print(row)

    con.close()
    print("Connection closed.")

# Run the script
if __name__ == "__main__":
    main()

