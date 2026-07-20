#pragma once

#include "commands/icommand.h"

namespace aicpp::commands {

class CmdTools : public ICommand {
public:
    std::string name() const override { return "tools"; }
    std::string description() const override { return "Kullanilabilir araclari ve risk seviyelerini listeler"; }
    std::string usage() const override { return "/tools"; }
    CommandResult execute(CommandContext& ctx) override;
};

}  // namespace aicpp::commands
