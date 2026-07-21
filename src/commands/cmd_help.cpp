#include "commands/cmd_help.h"

#include <fmt/core.h>

#include "cli/theme.h"
#include "commands/command_registry.h"

namespace aicpp::commands {

CommandResult CmdHelp::execute(CommandContext& ctx) {
    fmt::print("{}\n\n", i18n::t("help.banner"));
    cli::theme::sectionHeader(i18n::t("help.commands_header"));
    for (auto* cmd : ctx.app.registry.all()) {
        fmt::print("  {:<12} {}\n", cmd->usage(), cmd->description());
    }

    fmt::print("\n");
    cli::theme::sectionHeader(i18n::t("help.cloud_intro"));
    fmt::print("{}\n", i18n::t("help.cloud_usage"));
    fmt::print("{}\n", i18n::t("help.cloud_autoconfig"));
    fmt::print("{}\n", i18n::t("help.language_hint"));
    fmt::print("\n{}\n", i18n::t("help.footer"));
    return CommandResult::Handled;
}

}  // namespace aicpp::commands
