#pragma once

#include <string>

#include "commands/app_context.h"
#include "commands/command_registry.h"

namespace aicpp::cli {

enum class DispatchOutcome { NotACommand, Handled, ExitRequested };

// Decides whether `line` is a slash-command or a normal chat message, and
// executes it if it's a command. Unknown commands are reported to the user
// and NOT forwarded to the model (to avoid burning an API call on a typo).
DispatchOutcome dispatchLine(const std::string& line, commands::CommandRegistry& registry,
                              commands::AppContext& ctx);

}  // namespace aicpp::cli
