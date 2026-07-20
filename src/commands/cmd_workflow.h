#pragma once

#include "commands/icommand.h"

namespace aicpp::commands {

class CmdWorkflow : public ICommand {
public:
    std::string name() const override { return "workflow"; }
    std::string description() const override {
        return "Kayitli veya serbest-metin bir workflow'u paralel sub-agent'larla calistirir";
    }
    std::string usage() const override { return "/workflow [list | <isim> <girdi> | <serbest aciklama>]"; }
    CommandResult execute(CommandContext& ctx) override;
};

}  // namespace aicpp::commands
