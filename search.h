#pragma once
#include <string>

namespace SearchEngine {
    // Processes input: if it's not a URL, it turns it into a Google Search URL
    std::wstring ProcessInput(const std::string& input);
}