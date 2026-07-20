#include "commands/cmd_exit.h"

namespace aicpp::commands {

CommandResult CmdExit::execute(CommandContext&) { return CommandResult::HandledAndExit; }

}  // namespace aicpp::commands
