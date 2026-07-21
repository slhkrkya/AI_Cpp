#pragma once

#include "commands/icommand.h"
#include "i18n/Translator.h"

namespace aicpp::commands {

class CmdHistory : public ICommand {
public:
    std::string name() const override { return "history"; }
    std::string description() const override { return i18n::t("cmd.history.desc"); }
    std::string usage() const override { return "/history"; }
    CommandResult execute(CommandContext& ctx) override;
};

}  // namespace aicpp::commands
