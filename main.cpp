#include <bits/stdc++.h>
using namespace std;
namespace fs = filesystem;

// ==========================================
// 1. TRIE DATA STRUCTURE (For Autocomplete)
// ==========================================
struct TrieNode {
    unordered_map<char, TrieNode*> children;
    bool isEnd = false;
};

class Trie {
    TrieNode* root;

public:
    Trie() {
        root = new TrieNode();
    }

    void insert(const string& word) {
        TrieNode* node = root;
        for (char c : word) {
            if (!node->children[c])
                node->children[c] = new TrieNode();
            node = node->children[c];
        }
        node->isEnd = true;
    }

    void dfs(TrieNode* node, string prefix, vector<string>& res) {
        if (node->isEnd) res.push_back(prefix);
        for (auto& [ch, child] : node->children) {
            dfs(child, prefix + ch, res);
        }
    }

    vector<string> getSuggestions(string prefix) {
        TrieNode* node = root;
        for (char c : prefix) {
            if (!node->children[c]) return {};
            node = node->children[c];
        }

        vector<string> res;
        dfs(node, prefix, res);
        sort(res.begin(), res.end());
        return res;
    }
};

// ==========================================
// 2. SEARCH ENGINE CORE
// ==========================================
class SearchEngine {
    unordered_map<string, unordered_map<string, int>> invertedIndex;
    unordered_map<string, string> fileContent; 
    Trie trie;

    // Helper function to remove punctuation and hidden chars (like \r)
    string cleanToken(const string& word) {
        string clean = "";
        for (char c : word) {
            if (isalnum(c)) {
                clean += tolower(c);
            }
        }
        return clean;
    }

public:
    void indexFiles(const string& path) {
        cout << "\nIndexing files from: " << path << "...\n";

        for (const auto& entry : fs::directory_iterator(path)) {
            ifstream file(entry.path());
            if (!file.is_open()) continue;

            string filename = entry.path().filename();
            string word, content;
            unordered_set<string> uniqueWords;

            string line;
            while (getline(file, line)) {
                content += line + " ";
                stringstream ss(line);
                
                while (ss >> word) {
                    // Clean the word of punctuation before indexing
                    string cleanedWord = cleanToken(word);
                    if (cleanedWord.empty()) continue;

                    invertedIndex[cleanedWord][filename]++;
                    uniqueWords.insert(cleanedWord);
                }
            }

            for (const auto& w : uniqueWords) {
                trie.insert(w);
            }

            fileContent[filename] = content;
        }
        cout << "Indexing completed.\n";
    }

    void search(string query, bool exactMatch) {
        cout << "\n============================\n";
        cout << " Results for: \"" << query << "\"\n";
        cout << " Mode: " << (exactMatch ? "Exact Match" : "Normal Search") << "\n";
        cout << "============================\n";

        // --- STEP 1: TOKENIZATION & CLEANING ---
        vector<string> lowerTerms;
        stringstream ss(query);
        string term;
        while (ss >> term) {
            string cleanedTerm = cleanToken(term);
            if (!cleanedTerm.empty()) {
                lowerTerms.push_back(cleanedTerm);
            }
        }

        if (lowerTerms.empty()) {
            cout << "0 result(s) found.\n";
            return;
        }

        for (const string& t : lowerTerms) {
            if (invertedIndex.find(t) == invertedIndex.end()) {
                cout << "0 result(s) found.\n";
                return;
            }
        }

        // --- STEP 2: POSTING LIST INTERSECTION ---
        unordered_set<string> validDocs;
        for (auto& [doc, freq] : invertedIndex[lowerTerms[0]]) {
            validDocs.insert(doc);
        }

        for (size_t i = 1; i < lowerTerms.size(); i++) {
            unordered_set<string> nextDocs;
            for (auto& [doc, freq] : invertedIndex[lowerTerms[i]]) {
                if (validDocs.count(doc)) {
                    nextDocs.insert(doc); 
                }
            }
            validDocs = nextDocs;
        }

        // --- STEP 3: SCORING & EXACT MATCH VERIFICATION ---
        int totalDocs = fileContent.size();
        vector<pair<double, string>> results;

        for (const string& doc : validDocs) {
            if (exactMatch) {
                if (fileContent[doc].find(query) == string::npos) {
                    continue; 
                }
            }

            double totalScore = 0;
            for (const string& t : lowerTerms) {
                double tf = invertedIndex[t][doc];
                int docsWithWord = invertedIndex[t].size();
                
                double idf = log((double)totalDocs / (1 + docsWithWord)); 
                totalScore += (tf * idf);
            }

            results.push_back({totalScore, doc});
        }

        sort(results.rbegin(), results.rend());

        if (results.empty()) {
            cout << "0 result(s) found.\n";
            return;
        }

        cout << results.size() << " result(s) found\n\n";

        int i = 1;
        for (auto& [score, file] : results) {
            cout << i++ << ") " << file << "\n";
            cout << "   Score : " << fixed << setprecision(4) << score << "\n\n";
        }
    }

    void suggest(string prefix) {
        string cleanedPrefix = cleanToken(prefix);
        if (cleanedPrefix.empty()) {
            cout << "No suggestions found.\n";
            return;
        }

        auto suggestions = trie.getSuggestions(cleanedPrefix);

        cout << "\n============================\n";
        cout << " Suggestions for: \"" << prefix << "\"\n";
        cout << "============================\n";

        if (suggestions.empty()) {
            cout << "No suggestions found.\n";
            return;
        }

        for (const auto& word : suggestions) {
            cout << "- " << word << "\n";
        }
        cout << "\n";
    }
};

// ==========================================
// 3. MAIN CLI MENU
// ==========================================
int main() {
    SearchEngine engine;
    
    string path = "data"; 
    engine.indexFiles(path);

    while (true) {
        cout << "\n=========== MENU ===========\n";
        cout << "1) Normal Search (Case-Insensitive)\n";
        cout << "2) Exact Match Search (Case-Sensitive)\n";
        cout << "3) Suggest Auto-complete\n";
        cout << "4) Exit\n";
        cout << "============================\n";
        cout << "\nEnter choice: ";

        int choice;
        cin >> choice;

        if (choice == 1) {
            string query;
            cout << "\nEnter search query: ";
            cin.ignore(); 
            getline(cin, query); 
            
            engine.search(query, false); 
        }
        else if (choice == 2) {
            string query;
            cout << "\nEnter exact phrase to match: ";
            cin.ignore();
            getline(cin, query);
            
            engine.search(query, true); 
        }
        else if (choice == 3) {
            string prefix;
            cout << "\nEnter prefix: ";
            cin >> prefix;
            engine.suggest(prefix);
        }
        else {
            cout << "\nExiting...\n\n";
            break;
        }
    }
    return 0;
}