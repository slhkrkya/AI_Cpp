#pragma once

#include "commands/icommand.h"
#include "i18n/Translator.h"

namespace aicpp::commands {

class CmdClear : public ICommand {
public:
    std::string name() const override { return "clear"; }
    std::string description() const override { return i18n::t("cmd.clear.desc"); }
    std::string usage() const override { return "/clear"; }
    CommandResult execute(CommandContext& ctx) override;
};

}  // namespace aicpp::commands
