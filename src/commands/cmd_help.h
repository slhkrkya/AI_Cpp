#pragma once

#include "commands/icommand.h"

namespace aicpp::commands {

class CmdHelp : public ICommand {
public:
    std::string name() const override { return "help"; }
    std::string description() const override { return "Kullanilabilir komutlari listeler"; }
    std::string usage() const override { return "/help"; }
    CommandResult execute(CommandContext& ctx) override;
};

}  // namespace aicpp::commands
