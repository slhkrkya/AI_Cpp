#include "cli/dispatcher.h"

#include <cctype>

#include <fmt/format.h>

#include "cli/theme.h"
#include "i18n/Translator.h"

namespace aicpp::cli {

using commands::CommandContext;
using commands::CommandRegistry;
using commands::CommandResult;
using commands::ICommand;

namespace {

std::vector<std::string> splitArgs(const std::string& s) {
    std::vector<std::string> out;
    std::string cur;
    bool inQuotes = false;
    bool hasCur = false;

    for (char c : s) {
        if (c == '"') {
            inQuotes = !inQuotes;
            hasCur = true;
            continue;
        }
        if (!inQuotes && std::isspace(static_cast<unsigned char>(c))) {
            if (hasCur) {
                out.push_back(cur);
                cur.clear();
                hasCur = false;
            }
            continue;
        }
        cur += c;
        hasCur = true;
    }
    if (hasCur) out.push_back(cur);
    return out;
}

}  // namespace

DispatchOutcome dispatchLine(const std::string& line, CommandRegistry& registry,
                              commands::AppContext& ctx) {
    size_t start = line.find_first_not_of(" \t");
    if (start == std::string::npos || line[start] != '/') {
        return DispatchOutcome::NotACommand;
    }

    size_t nameEnd = line.find_first_of(" \t", start);
    std::string name =
        line.substr(start + 1, (nameEnd == std::string::npos ? line.size() : nameEnd) - start - 1);

    std::string rest;
    if (nameEnd != std::string::npos) {
        size_t restStart = line.find_first_not_of(" \t", nameEnd);
        if (restStart != std::string::npos) rest = line.substr(restStart);
    }

    ICommand* cmd = registry.find(name);
    if (!cmd) {
        theme::warn(fmt::format(fmt::runtime(i18n::t("dispatcher.unknown_command")), name));
        if (auto suggestion = registry.suggest(name)) {
            fmt::print("{}\n", fmt::format(fmt::runtime(i18n::t("dispatcher.suggestion")), *suggestion));
        }
        fmt::print("{}\n", i18n::t("dispatcher.help_hint"));
        return DispatchOutcome::Handled;
    }

    CommandContext cctx{ctx, splitArgs(rest), rest};
    CommandResult result = cmd->execute(cctx);
    return result == CommandResult::HandledAndExit ? DispatchOutcome::ExitRequested
                                                    : DispatchOutcome::Handled;
}

}  // namespace aicpp::cli
