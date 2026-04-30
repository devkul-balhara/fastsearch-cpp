# fastsearch-cpp — In-Memory Search Engine (C++17)

fastsearch-cpp is a high-performance, in-memory search engine built in C++17. It implements core Information Retrieval (IR) techniques with an emphasis on efficient query execution, ranking, and system-level trade-offs.

---

## Features

* Multi-word AND Search
  - Uses posting list intersection to efficiently filter candidate documents.
* Exact Match Search (Case-Sensitive)
  - Implements a filter -> verify pipeline:
    - Fast candidate generation via inverted index.
    - Precise validation using forward index.
  - Avoids duplicating a case-sensitive index, significantly saving memory.
* TF-IDF Ranking
  - Scores documents based on term importance.
  - Reduces the impact of common words (stop-word mitigation).
* Autocomplete (Trie)
  - Prefix-based suggestions in `O(L)` time.
* Text Normalization
  - Removes punctuation and hidden characters (\r).
  - Ensures consistent indexing and avoids recall loss.

---

## Architecture

* Inverted Index
  `unordered_map<string, unordered_map<string, int>>`
  term -> (document -> frequency)

* Forward Index
  `unordered_map<string, string>`
  raw document storage for exact match verification

* Trie
  `unordered_map<char, TrieNode*>`
  memory-efficient prefix search

---

## Query Pipeline

1. Query Input
2. Tokenize & Normalize
3. Posting List Intersection
4. Candidate Documents
5. (Optional) Exact Match Verification
6. TF-IDF Scoring
7. Sorted Results Output

---

## Complexity

* Indexing: `O(T)` where T is the number of tokens.
* Lookup: `O(1)` average time.
* Intersection: Proportional to the size of the smallest posting list.
* Autocomplete: `O(L + K log K)` where L is prefix length and K is the number of matches.

---

## Getting Started

### Project Structure

fastsearch-cpp/
├── data/           # Source text files for indexing
├── .gitignore      # Git exclusion rules
├── main.cpp        # Search Engine Logic
└── README.md       # Project documentation

### Build & Run

Ensure you have a compiler that supports C++17.

1. Navigate to the project directory:
   cd fastsearch-cpp

2. Compile with O2 optimization:
   g++ -std=c++17 -O2 -o main main.cpp

3. Run the engine:
   ./main

---

## Key Design Decisions

* Filter -> Verify over dual indexing: Reduces memory footprint significantly by avoiding two separate indices for case sensitivity.
* `unordered_map` over `map`: Prioritizes constant-time average lookup for high-throughput queries.
* Trie with hashmap nodes: Avoids fixed-size overhead for sparse datasets while maintaining fast prefix lookups.

---

## Limitations & Future Improvements

Current Limitations:
* Single-threaded indexing and search.
* In-memory only (not persistent).
* No positional index (phrase queries use post-filtering).
* No posting list compression.

Future Roadmap:
* BM25 ranking for better relevance.
* Parallel indexing using `std::async` or thread pools.
* Positional index for efficient phrase searching.
* Memory-mapped storage for handling datasets larger than RAM.

---

fastsearch-cpp demonstrates how modern search systems balance latency, memory, and accuracy using layered indexing and ranking strategies.