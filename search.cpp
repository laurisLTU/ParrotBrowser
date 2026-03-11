#include "search.h"
#include <algorithm>

namespace SearchEngine {
    std::wstring ProcessInput(const std::string& input) {
        std::string processed = input;
        
        // Remove leading/trailing whitespace
        processed.erase(0, processed.find_first_not_of(" "));
        processed.erase(processed.find_last_not_of(" ") + 1);

        // Simple heuristic: If it contains a dot and no spaces, treat as URL
        bool hasDot = (processed.find('.') != std::string::npos);
        bool hasSpace = (processed.find(' ') != std::string::npos);

        std::string finalUrl;

        if (hasDot && !hasSpace) {
            // It looks like a URL
            if (processed.find("http") != 0) {
                finalUrl = "https://" + processed;
            } else {
                finalUrl = processed;
            }
        } else {
            // It looks like a search query
            // Format: https://www.google.com/search?q=query+with+pluses
            std::string query = processed;
            std::replace(query.begin(), query.end(), ' ', '+');
            finalUrl = "https://www.google.com/search?q=" + query;
        }

        return std::wstring(finalUrl.begin(), finalUrl.end());
    }
}