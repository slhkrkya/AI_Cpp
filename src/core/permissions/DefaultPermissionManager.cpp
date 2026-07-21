#include "core/permissions/DefaultPermissionManager.h"

#include <iostream>

#include <fmt/format.h>

#include "cli/theme.h"
#include "i18n/Translator.h"

namespace aicpp::permissions {

using tools::RiskCategory;

Decision DefaultPermissionManager::requestPermission(const std::string& toolName, RiskCategory risk,
                                                       const std::string& humanSummary) {
    if (risk == RiskCategory::Read) {
        return Decision::Allow;
    }

    if (!overrideStack_.empty() && overrideStack_.back() == PolicyOverride::ReadOnlyOnly) {
        return Decision::Deny;  // Plan Mode: silently refuse, no prompt.
    }

    if (alwaysAllowed_.count(toolName)) {
        return Decision::Allow;
    }

    std::string header = fmt::format(fmt::runtime(i18n::t("permission.wants_to")), toolName);
    cli::theme::permissionPrompt(header, humanSummary);
    fmt::print("{}", cli::theme::promptText(i18n::t("permission.ask_yna")));

    std::string answer;
    if (!std::getline(std::cin, answer) || answer.empty()) {
        return Decision::Deny;
    }

    char c = static_cast<char>(std::tolower(static_cast<unsigned char>(answer[0])));
    if (c == 'a') {
        alwaysAllowed_.insert(toolName);
        return Decision::AlwaysAllowThisSession;
    }
    if (c == 'y') {
        return Decision::Allow;
    }
    return Decision::Deny;
}

}  // namespace aicpp::permissions
