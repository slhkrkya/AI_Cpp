#pragma once

#include "commands/icommand.h"
#include "i18n/Translator.h"

namespace aicpp::commands {

class CmdModel : public ICommand {
public:
    std::string name() const override { return "model"; }
    std::string description() const override { return i18n::t("cmd.model.desc"); }
    std::string usage() const override { return i18n::t("cmd.model.usage"); }
    CommandResult execute(CommandContext& ctx) override;
};

}  // namespace aicpp::commands
