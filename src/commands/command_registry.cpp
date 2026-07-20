#include "commands/command_registry.h"

#include <algorithm>
#include <cctype>
#include <vector>

namespace aicpp::commands {

namespace {

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

int levenshtein(const std::string& a, const std::string& b) {
    const size_t n = a.size(), m = b.size();
    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));
    for (size_t i = 0; i <= n; ++i) dp[i][0] = static_cast<int>(i);
    for (size_t j = 0; j <= m; ++j) dp[0][j] = static_cast<int>(j);
    for (size_t i = 1; i <= n; ++i) {
        for (size_t j = 1; j <= m; ++j) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            dp[i][j] = std::min({dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + cost});
        }
    }
    return dp[n][m];
}

}  // namespace

void CommandRegistry::registerCommand(std::shared_ptr<ICommand> cmd) {
    ICommand* raw = cmd.get();
    byName_[toLower(raw->name())] = raw;
    for (const auto& alias : raw->aliases()) {
        byName_[toLower(alias)] = raw;
    }
    commands_.push_back(std::move(cmd));
}

ICommand* CommandRegistry::find(const std::string& nameOrAlias) const {
    auto it = byName_.find(toLower(nameOrAlias));
    return it == byName_.end() ? nullptr : it->second;
}

std::vector<ICommand*> CommandRegistry::all() const {
    std::vector<ICommand*> result;
    result.reserve(commands_.size());
    for (const auto& cmd : commands_) result.push_back(cmd.get());
    return result;
}

std::optional<std::string> CommandRegistry::suggest(const std::string& typo) const {
    std::string bestName;
    int bestDistance = 3;  // only suggest for close typos
    for (const auto& [name, cmd] : byName_) {
        (void)cmd;
        int d = levenshtein(toLower(typo), name);
        if (d < bestDistance) {
            bestDistance = d;
            bestName = name;
        }
    }
    if (bestName.empty()) return std::nullopt;
    return bestName;
}

}  // namespace aicpp::commands
