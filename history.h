#pragma once
#include <vector>
#include <string>

namespace HistoryManager {
    void AddEntry(const std::string& url);
    const std::vector<std::string>& GetHistory();
}