#include "commands/cmd_tools.h"

#include <fmt/core.h>

#include "cli/theme.h"

namespace aicpp::commands {

namespace {

std::string riskToString(tools::RiskCategory risk) {
    switch (risk) {
        case tools::RiskCategory::Read:
            return i18n::t("tools.risk.read");
        case tools::RiskCategory::Write:
            return i18n::t("tools.risk.write");
        case tools::RiskCategory::Execute:
            return i18n::t("tools.risk.execute");
        case tools::RiskCategory::Network:
            return i18n::t("tools.risk.network");
    }
    return "?";
}

}  // namespace

CommandResult CmdTools::execute(CommandContext& ctx) {
    cli::theme::sectionHeader(i18n::t("tools.header"));
    for (auto* tool : ctx.app.toolRegistry.all()) {
        fmt::print("  {:<12} [{}] {}\n", tool->name(), riskToString(tool->riskCategory()), tool->description());
    }
    return CommandResult::Handled;
}

}  // namespace aicpp::commands
