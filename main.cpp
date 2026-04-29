#include <bits/stdc++.h>
#include <filesystem>
using namespace std;
namespace fs = filesystem;

// Trie for prefix-based autocomplete suggestions
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

    // Return words matching given prefix
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

// ---------------- SEARCH ENGINE ----------------
class SearchEngine {
    // Inverted Index: word -> (document -> frequency)
    unordered_map<string, unordered_map<string, int>> invertedIndex;
    unordered_map<string, unordered_map<string, int>> wordFreq;

    unordered_map<string, string> fileContent;
    Trie trie;

public:
    void indexFiles(const string& path) {
        cout << "\nIndexing files from: " << path << "...\n";

        for (const auto& entry : fs::directory_iterator(path)) {
            ifstream file(entry.path());
            if (!file.is_open()) continue;

            string filename = entry.path().filename();
            string word, content;

            // Build index and insert into Trie
            while (file >> word) {
                for (auto& c : word) c = tolower(c);

                invertedIndex[word][filename]++;
                wordFreq[word][filename]++;
                trie.insert(word);

                content += word + " ";
            }

            fileContent[filename] = content;
        }

        cout << "Indexing completed.\n";
    }

    // Extract small context around first occurrence
    string getSnippet(const string& content, const string& word) {
        size_t pos = content.find(word);
        if (pos == string::npos) return "";

        int start = max(0, (int)pos - 20);
        int end = min((int)content.size(), (int)pos + 40);

        return "... " + content.substr(start, end - start) + " ...";
    }

    void search(string query) {
        for (auto& c : query) c = tolower(c);

        cout << "\n============================\n";
        cout << " Results for: \"" << query << "\"\n";
        cout << "============================\n";

        if (invertedIndex.find(query) == invertedIndex.end()) {
            cout << "No results found.\n";
            return;
        }

        int totalDocs = fileContent.size();
        int docsWithWord = invertedIndex[query].size();

        cout << docsWithWord << " result(s) found\n\n";

        int i = 1;
        for (auto& [file, freq] : invertedIndex[query]) {

            // Simplified TF-IDF scoring
            double tf = wordFreq[query][file];
            double idf = log((double)totalDocs / (1 + docsWithWord));
            double score = tf * idf;

            cout << i++ << ") " << file << "\n";
            cout << "   Score : " << fixed << setprecision(4) << score << "\n";
            cout << "   Freq  : " << freq << "\n";
            cout << "   Match : " << getSnippet(fileContent[file], query) << "\n";
            cout << "\n";
        }
    }

    void suggest(string prefix) {
        for (auto& c : prefix) c = tolower(c);

        auto suggestions = trie.getSuggestions(prefix);

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

// ---------------- MAIN ----------------
int main() {
    SearchEngine engine;

    string path = "data"; // local dataset folder
    engine.indexFiles(path);

    while (true) {
        cout << "\n=========== MENU ===========\n";
        cout << "1) Search\n";
        cout << "2) Suggest\n";
        cout << "3) Exit\n";
        cout << "============================\n";
        cout << "\nEnter choice: ";

        int choice;
        cin >> choice;

        if (choice == 1) {
            string query;
            cout << "\nEnter word: ";
            cin >> query;
            engine.search(query);
        }
        else if (choice == 2) {
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
}