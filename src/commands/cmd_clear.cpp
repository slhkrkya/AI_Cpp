#include "commands/cmd_clear.h"

#include "cli/theme.h"

namespace aicpp::commands {

CommandResult CmdClear::execute(CommandContext& ctx) {
    ctx.app.session.clearHistory();
    cli::theme::clearScreen();
    cli::theme::success(i18n::t("clear.done"));
    return CommandResult::Handled;
}

}  // namespace aicpp::commands
