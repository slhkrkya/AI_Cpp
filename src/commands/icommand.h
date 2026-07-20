#pragma once

#include <string>
#include <vector>

#include "commands/app_context.h"

namespace aicpp::commands {

enum class CommandResult { Handled, HandledAndExit, ShowError };

struct CommandContext {
    AppContext& app;
    std::vector<std::string> args;  // quote-aware tokenized
    std::string rawArgs;             // untokenized remainder, for commands that want free text
};

class ICommand {
public:
    virtual ~ICommand() = default;

    virtual std::string name() const = 0;
    virtual std::vector<std::string> aliases() const { return {}; }
    virtual std::string description() const = 0;
    virtual std::string usage() const = 0;

    virtual CommandResult execute(CommandContext& ctx) = 0;
};

}  // namespace aicpp::commands
