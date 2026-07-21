#pragma once

#include "commands/icommand.h"
#include "i18n/Translator.h"

namespace aicpp::commands {

class CmdPlan : public ICommand {
public:
    std::string name() const override { return "plan"; }
    std::string description() const override { return i18n::t("cmd.plan.desc"); }
    std::string usage() const override { return i18n::t("cmd.plan.usage"); }
    CommandResult execute(CommandContext& ctx) override;
};

}  // namespace aicpp::commands
