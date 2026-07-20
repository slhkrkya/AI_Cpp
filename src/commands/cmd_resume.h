#pragma once

#include "commands/icommand.h"

namespace aicpp::commands {

class CmdResume : public ICommand {
public:
    std::string name() const override { return "resume"; }
    std::string description() const override { return "Gecmis bir oturuma kaldigi yerden devam eder"; }
    std::string usage() const override { return "/resume [numara-veya-id]"; }
    CommandResult execute(CommandContext& ctx) override;
};

}  // namespace aicpp::commands
