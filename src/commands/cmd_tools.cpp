#include "commands/cmd_tools.h"

#include <fmt/core.h>

namespace aicpp::commands {

namespace {

std::string riskToString(tools::RiskCategory risk) {
    switch (risk) {
        case tools::RiskCategory::Read:
            return "Read (otomatik izinli)";
        case tools::RiskCategory::Write:
            return "Write (izin gerekir)";
        case tools::RiskCategory::Execute:
            return "Execute (izin gerekir)";
        case tools::RiskCategory::Network:
            return "Network (izin gerekir)";
    }
    return "?";
}

}  // namespace

CommandResult CmdTools::execute(CommandContext& ctx) {
    fmt::print("Kullanilabilir araclar:\n");
    for (auto* tool : ctx.app.toolRegistry.all()) {
        fmt::print("  {:<12} [{}] {}\n", tool->name(), riskToString(tool->riskCategory()), tool->description());
    }
    return CommandResult::Handled;
}

}  // namespace aicpp::commands
