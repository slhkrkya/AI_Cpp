#pragma once

#include "commands/icommand.h"

namespace aicpp::commands {

class CmdConfig : public ICommand {
public:
    std::string name() const override { return "config"; }
    std::string description() const override { return "Config dosyasinin yolunu ve icerigini gosterir"; }
    std::string usage() const override { return "/config"; }
    CommandResult execute(CommandContext& ctx) override;
};

}  // namespace aicpp::commands
