#pragma once

#include "commands/icommand.h"

namespace aicpp::commands {

class CmdHistory : public ICommand {
public:
    std::string name() const override { return "history"; }
    std::string description() const override { return "Mevcut oturumu ve gecmis oturumlari listeler"; }
    std::string usage() const override { return "/history"; }
    CommandResult execute(CommandContext& ctx) override;
};

}  // namespace aicpp::commands
