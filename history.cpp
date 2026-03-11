#include "history.h"
#include <vector>
#include <string>

namespace HistoryManager {
    static std::vector<std::string> browsingHistory;

    void AddEntry(const std::string& url) {
        // Don't add duplicates in a row
        if (!browsingHistory.empty() && browsingHistory.back() == url) return;
        
        browsingHistory.push_back(url);
        
        // Keep only last 50 entries
        if (browsingHistory.size() > 50) {
            browsingHistory.erase(browsingHistory.begin());
        }
    }

    const std::vector<std::string>& GetHistory() {
        return browsingHistory;
    }
}