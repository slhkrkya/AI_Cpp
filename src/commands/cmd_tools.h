#pragma once

#include "commands/icommand.h"
#include "i18n/Translator.h"

namespace aicpp::commands {

class CmdTools : public ICommand {
public:
    std::string name() const override { return "tools"; }
    std::string description() const override { return i18n::t("cmd.tools.desc"); }
    std::string usage() const override { return "/tools"; }
    CommandResult execute(CommandContext& ctx) override;
};

}  // namespace aicpp::commands
