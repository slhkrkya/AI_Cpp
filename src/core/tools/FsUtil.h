#pragma once

#include <string>
#include <unordered_set>

namespace aicpp::tools {

// Directory names skipped by GlobTool/GrepTool traversal so results (and the
// context window) aren't flooded with VCS/build/dependency noise.
inline bool isIgnoredDirName(const std::string& name) {
    static const std::unordered_set<std::string> kIgnored = {
        ".git", "build", "out", "vcpkg_installed", "node_modules", ".vs", ".idea",
    };
    return kIgnored.count(name) > 0;
}

// Classic linear two-pointer glob match (supports '*' and '?'); iterative, no
// backtracking blow-up risk even on adversarial patterns.
inline bool wildcardMatch(const std::string& text, const std::string& pattern) {
    size_t t = 0, p = 0, star = std::string::npos, match = 0;
    while (t < text.size()) {
        if (p < pattern.size() && (pattern[p] == text[t] || pattern[p] == '?')) {
            ++t;
            ++p;
        } else if (p < pattern.size() && pattern[p] == '*') {
            star = p;
            match = t;
            ++p;
        } else if (star != std::string::npos) {
            p = star + 1;
            t = ++match;
        } else {
            return false;
        }
    }
    while (p < pattern.size() && pattern[p] == '*') ++p;
    return p == pattern.size();
}

}  // namespace aicpp::tools
