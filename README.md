# FastSearch (C++)

FastSearch is a simple in-memory keyword-based search engine written in C++17.  
It uses an **inverted index** for fast lookup and a **Trie** for prefix-based suggestions.  
The goal of this project was to understand how basic search systems work under the hood.

## Features
- Ranked search using a simplified **TF-IDF** scoring approach  
- Currently supports **single-term queries** (designed with multi-word support in mind)  
- Prefix autocomplete using a custom Trie  
- Context snippets showing keyword matches in documents  

## Design Overview
- An **unordered_map** is used as the inverted index for fast term → document lookups  
- A separate **Trie** is maintained only for prefix suggestions (not used in ranking)  
- Ranking is based on term frequency and a simplified inverse document frequency  
- During indexing, **unique terms per document** are inserted into the Trie to avoid duplicates  

## How It Works (High Level)
1. All `.txt` files in the `data/` directory are parsed and indexed in memory  
2. Each term maps to a list of `(document, frequency)` pairs  
3. Query is normalized (converted to lowercase)  
4. Matching documents are ranked using TF-IDF  
5. Results are sorted by relevance and displayed with a snippet  

## Limitations & Future Improvements
- Only supports **single-term queries**  
- No stop-word removal or stemming  
- Entire index is stored in memory (not scalable for large datasets)  
- Multi-word query support can be added using posting list intersection  