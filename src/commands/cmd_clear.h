#pragma once

#include "commands/icommand.h"

namespace aicpp::commands {

class CmdClear : public ICommand {
public:
    std::string name() const override { return "clear"; }
    std::string description() const override { return "Ekrani ve sohbet gecmisini temizler"; }
    std::string usage() const override { return "/clear"; }
    CommandResult execute(CommandContext& ctx) override;
};

}  // namespace aicpp::commands
