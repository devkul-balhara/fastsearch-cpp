# FastSearch (C++)

FastSearch is a highly optimized, in-memory search engine written in C++17.  
It demonstrates core Information Retrieval concepts, utilizing an **Inverted Index** for fast high-recall lookups, a **Forward Index** for precise exact-matching, and a **Trie** for $O(L)$ autocomplete suggestions. 

The goal of this project is to understand system design trade-offs, time/space complexity optimization, and search ranking algorithms under the hood.

## 🚀 Key Features
- **Multi-Word Queries (AND Search):** Supports complex queries by utilizing **Posting List Intersection** to find documents containing all search terms.
- **Exact Match Search (Case-Sensitive):** Implements a highly efficient **"Filter then Verify"** architecture. It uses the lowercase inverted index to rapidly filter candidate documents, then scans the raw forward index to verify exact casing and phrasing, preventing memory bloat.
- **Ranked Search (TF-IDF):** Dynamically scores and ranks matching documents using Term Frequency - Inverse Document Frequency to penalize overly common words.
- **Robust Text Normalization:** Actively sanitizes ingested text by stripping punctuation and hidden carriage returns (`\r`) using alphanumeric filtering to prevent indexing bugs and maximize recall.
- **Prefix Autocomplete:** Uses a custom `unordered_map`-based Trie to provide instant word suggestions based on the indexed vocabulary.

## 🧠 System Architecture
- **Inverted Index:** An `unordered_map<string, unordered_map<string, int>>` used for $O(1)$ fast term → document frequency lookups. All keys are aggressively lowercased and stripped of punctuation to prioritize recall.
- **Forward Index:** An `unordered_map<string, string>` that stores the raw, untouched text of every document. This acts as a secondary verification layer for Exact Match queries.
- **Trie:** Maintained exclusively for prefix suggestions. It utilizes `unordered_map` nodes instead of fixed arrays to drastically reduce memory overhead (handling sparse node branches and diverse character sets safely).

## 🛠️ How It Works (Execution Flow)
1. **Indexing Phase:** All `.txt` files in the `data/` directory are parsed. Words are sanitized, lowercased, and mapped to a list of `(document, frequency)` pairs.
2. **Query Normalization:** User input is tokenized and sanitized.
3. **Boolean Filtering:** For multi-word queries, the engine performs a set intersection to immediately drop documents that do not contain all terms.
4. **Post-Filtering (If Exact Match):** The engine physically scans the raw forward index of the remaining candidate files to ensure the exact case-sensitive phrase exists.
5. **Scoring:** The surviving documents are ranked using the TF-IDF mathematical formula.
6. **Output:** Results are sorted in descending order by relevance score.

## 💻 Interactive CLI Menu
The engine runs via a clean Command Line Interface with the following modes:
```text
=========== MENU ===========
1) Normal Search (Case-Insensitive, Multi-Word AND)
2) Exact Match Search (Case-Sensitive Phrase)
3) Suggest Auto-complete
4) Exit
============================