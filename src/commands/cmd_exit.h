#pragma once

#include "commands/icommand.h"
#include "i18n/Translator.h"

namespace aicpp::commands {

class CmdExit : public ICommand {
public:
    std::string name() const override { return "exit"; }
    std::vector<std::string> aliases() const override { return {"quit"}; }
    std::string description() const override { return i18n::t("cmd.exit.desc"); }
    std::string usage() const override { return "/exit"; }
    CommandResult execute(CommandContext& ctx) override;
};

}  // namespace aicpp::commands
