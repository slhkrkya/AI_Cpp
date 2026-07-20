#pragma once

#include "commands/icommand.h"

namespace aicpp::commands {

class CmdModel : public ICommand {
public:
    std::string name() const override { return "model"; }
    std::string description() const override { return "Aktif LLM saglayicisini/modelini gosterir veya degistirir"; }
    std::string usage() const override { return "/model [saglayici:model]"; }
    CommandResult execute(CommandContext& ctx) override;
};

}  // namespace aicpp::commands
