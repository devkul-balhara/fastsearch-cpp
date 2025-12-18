#include <bits/stdc++.h>
using namespace std;

namespace fs = filesystem;

// --- CONFIG & COLORS ---
const string RED = "\033[1;31m";
const string GREEN = "\033[1;32m";
const string BLUE = "\033[1;34m";
const string GRAY = "\033[90m";
const string RESET = "\033[0m";

const unordered_set<string> STOPWORDS = {
    "the", "is", "at", "which", "on", "and", "a", "an", "in", "to", "of", "for", "it", "that", "this", "are", "with", "as", "by"
};

// --- DATA STRUCTURES ---

class TrieNode{
    public:
    unordered_map<char, TrieNode*> children;
    bool isEndOfWord = false;
};

class Trie{
    TrieNode* root;

    public:
    Trie() {root = new TrieNode();}

    void insert(const string& word){
        TrieNode* curr = root;
        for (char c : word) {
            if (curr->children.find(c) == curr->children.end())
                curr->children[c] = new TrieNode();
            curr = curr->children[c];
        }
        curr->isEndOfWord = true;
    }

    void findWords(TrieNode* node, string prefix, vector<string>& results) {
        if (results.size() >= 3) return; 
        if (node->isEndOfWord) results.push_back(prefix);
        for (auto& pair : node->children) {
            findWords(pair.second, prefix + pair.first, results);
        }
    }

    vector<string> suggest(const string& prefix) {
        TrieNode* curr = root;
        vector<string> results;
        for (char c : prefix) {
            if (curr->children.find(c) == curr->children.end()) return results;
            curr = curr->children[c];
        }
        findWords(curr, prefix, results);
        return results;
    }
};

class DocInfo{
    public:
    int id;
    string filepath;
    string filename;
    int wordCount; // For Length Normalization
};

class SearchEngine {
    unordered_map<string, vector<pair<int, int>>> invertedIndex; // Term -> [(DocID, Frequency)]
    unordered_map<int, DocInfo> documents;
    Trie prefixTrie;
    int docCounter = 0;

    string normalize(string word) {
        string clean = "";
        for (char c : word) {
            if (isalnum(c)) clean += tolower(c);
        }
        return clean;
    }

    vector<string> tokenize(const string& query) {
        stringstream ss(query);
        string word;
        vector<string> tokens;
        while (ss >> word) {
            string clean = normalize(word);
            if (!clean.empty() && STOPWORDS.find(clean) == STOPWORDS.end()) {
                tokens.push_back(clean);
            }
        }
        return tokens;
    }

    public:
    void indexDirectory(const string& dirPath) {
        cout << GRAY << "[System] Indexing documents in " << dirPath << "..." << RESET << endl;
        if (!fs::exists(dirPath)) {
            cout << RED << "Error: Directory not found." << RESET << endl;
            return;
        }
        for (const auto& entry : fs::directory_iterator(dirPath)) {
            if (entry.path().extension() == ".txt") {
                indexFile(entry.path().string());
            }
        }
        cout << GREEN << "[System] Indexing complete. " << docCounter << " documents processed." << RESET << "\n";
    }

    void indexFile(const string& filepath) {
        ifstream file(filepath);
        if (!file.is_open()) return;

        int id = ++docCounter;
        string filename = fs::path(filepath).filename().string();
        
        string word;
        int totalWords = 0;
        unordered_map<string, int> termFreq; 
        unordered_set<string> uniqueTerms; // CRITICAL FIX 1: Avoid duplicate Trie inserts

        while (file >> word) {
            string clean = normalize(word);
            if (clean.empty() || STOPWORDS.count(clean)) continue;
            
            termFreq[clean]++;
            uniqueTerms.insert(clean);
            totalWords++;
        }
        
        documents[id] = {id, filepath, filename, totalWords};

        // Update Trie only with unique terms
        for (const string& term : uniqueTerms) {
            prefixTrie.insert(term);
        }

        // Update Inverted Index
        for (auto& entry : termFreq) {
            invertedIndex[entry.first].push_back({id, entry.second});
        }
    }

    void showSnippet(const string& filepath, const string& queryTerm) {
        ifstream file(filepath);
        string word, clean;
        vector<string> buffer;
        bool found = false;

        while (file >> word) {
            clean = normalize(word);
            buffer.push_back(word);
            if (buffer.size() > 15) buffer.erase(buffer.begin()); 

            if (clean == queryTerm && !found) {
                found = true;
                cout << "   ... ";
                for (size_t i = 0; i < buffer.size() - 1; i++) cout << buffer[i] << " ";
                cout << RED << word << RESET << " "; 
                for (int i = 0; i < 5 && (file >> word); i++) cout << word << " ";
                cout << "..." << endl;
                return;
            }
        }
    }

    void search(const string& rawQuery) {
        vector<string> tokens = tokenize(rawQuery);
        if (tokens.empty()) return;

        // A. Autocomplete (based on last token if only one exists, or simply first)
        if (tokens.size() == 1) {
             vector<string> suggestions = prefixTrie.suggest(tokens[0]);
             if (!suggestions.empty() && suggestions[0] != tokens[0]) {
                 cout << BLUE << "Did you mean? " << RESET;
                 for (const auto& s : suggestions) cout << s << " ";
                 cout << "\n";
             }
        }

        // B. Multi-term Intersection & Scoring
        // Map: DocID -> Score
        unordered_map<int, double> docScores;
        unordered_map<int, int> docMatchCount; // CRITICAL FIX 3: Count matches for AND logic

        for (const string& token : tokens) {
            if (invertedIndex.find(token) == invertedIndex.end()) {
                cout << GRAY << "No exact matches for term: " << token << RESET << "\n";
                continue; 
            }

            double idf = log10((double)docCounter / (1 + invertedIndex[token].size()));

            for (auto& entry : invertedIndex[token]) {
                int docId = entry.first;
                int tf = entry.second;
                
                // CRITICAL FIX 2: Length Normalization
                double tfNorm = (double)tf / documents[docId].wordCount;
                double score = tfNorm * idf;

                docScores[docId] += score;
                docMatchCount[docId]++;
            }
        }

        // Filter: Strict AND (Intersection) - Doc must contain ALL valid tokens
        vector<pair<double, int>> finalResults;
        int requiredMatches = 0;
        // Count how many tokens actually exist in our index to determine required matches
        for(const auto& t : tokens) if(invertedIndex.count(t)) requiredMatches++;

        if (requiredMatches == 0) {
            cout << "No documents found matching query." << endl;
            return;
        }

        for (auto& entry : docScores) {
            if (docMatchCount[entry.first] == requiredMatches) {
                finalResults.push_back({entry.second, entry.first});
            }
        }

        sort(finalResults.rbegin(), finalResults.rend()); 

        // C. Display Results
        cout << "\nResults for '" << RED << rawQuery << RESET << "' (" << finalResults.size() << " hits):\n";
        int rank = 1;
        for (auto& res : finalResults) {
            if (rank > 3) break; 
            DocInfo& doc = documents[res.second];
            
            cout << rank++ << ". " << GREEN << doc.filename << RESET 
                 << " (Score: " << fixed << setprecision(4) << res.first << ")\n";
            
            // Show snippet for the first token found
            showSnippet(doc.filepath, tokens[0]);
            cout << GRAY << "-----------------------------------" << RESET << "\n";
        }
        cout << endl;
    }
};

int main() {
    SearchEngine engine;
    
    if (!fs::exists("data")) {
        fs::create_directory("data");
        cout << RED << "Created 'data' folder. Add .txt files and restart!" << RESET << endl;
        return 0;
    }

    engine.indexDirectory("data");

    string query;
    while (true) {
        cout << "Search > ";
        getline(cin, query);
        if (query == "exit") break;
        engine.search(query);
    }
    return 0;
}