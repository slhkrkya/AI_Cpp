#pragma once

#include "commands/icommand.h"
#include "i18n/Translator.h"

namespace aicpp::commands {

class CmdWorkflow : public ICommand {
public:
    std::string name() const override { return "workflow"; }
    std::string description() const override { return i18n::t("cmd.workflow.desc"); }
    std::string usage() const override { return i18n::t("cmd.workflow.usage"); }
    CommandResult execute(CommandContext& ctx) override;
};

}  // namespace aicpp::commands
