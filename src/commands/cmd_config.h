#pragma once

#include "commands/icommand.h"
#include "i18n/Translator.h"

namespace aicpp::commands {

class CmdConfig : public ICommand {
public:
    std::string name() const override { return "config"; }
    std::string description() const override { return i18n::t("cmd.config.desc"); }
    std::string usage() const override { return "/config"; }
    CommandResult execute(CommandContext& ctx) override;
};

}  // namespace aicpp::commands
