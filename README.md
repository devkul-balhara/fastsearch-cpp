# FastSearch (C++)

FastSearch is a simple in-memory full-text search engine written in C++17.  
It uses an **inverted index** for fast ranked lookup and a **Trie** for prefix-based suggestions.  
The goal of this project was to understand how basic search systems work under the hood.

## Features
- Ranked search using **TF-IDF** with document length normalization  
- Supports **multi-word AND queries** (documents must contain all query terms)  
- Prefix autocomplete using a custom Trie  
- Context snippets with basic keyword highlighting in the terminal  

## Design Overview
- An **unordered_map** is used as the inverted index for fast term → document lookups.
- A separate **Trie** is maintained only for prefix suggestions (not for ranking).
- Term frequency is normalized by document length to avoid bias toward long files.
- Posting lists are intersected to enforce strict AND query semantics.
- During indexing, unique terms are filtered before Trie insertion to avoid duplicates.

## How It Works (High Level)
1. All `.txt` files in the `data/` directory are parsed and indexed in memory.
2. Each term maps to a list of `(docID, frequency)` pairs.
3. Queries are tokenized and normalized.
4. Documents containing **all** query terms are ranked using TF-IDF.
5. Top results are displayed with a small highlighted snippet.