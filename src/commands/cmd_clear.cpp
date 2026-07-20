#include "commands/cmd_clear.h"

#include <fmt/core.h>

namespace aicpp::commands {

CommandResult CmdClear::execute(CommandContext& ctx) {
    ctx.app.session.clearHistory();
    fmt::print("\x1b[2J\x1b[H");
    fmt::print("Sohbet gecmisi temizlendi.\n");
    return CommandResult::Handled;
}

}  // namespace aicpp::commands
