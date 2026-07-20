#pragma once

#include "commands/icommand.h"

namespace aicpp::commands {

class CmdPlan : public ICommand {
public:
    std::string name() const override { return "plan"; }
    std::string description() const override {
        return "Plan modunu ac/kapat (sadece okuma araclari), /plan approve ile uygula";
    }
    std::string usage() const override { return "/plan [metin|approve|cancel]"; }
    CommandResult execute(CommandContext& ctx) override;
};

}  // namespace aicpp::commands
