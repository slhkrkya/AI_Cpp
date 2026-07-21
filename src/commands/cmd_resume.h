#pragma once

#include "commands/icommand.h"
#include "i18n/Translator.h"

namespace aicpp::commands {

class CmdResume : public ICommand {
public:
    std::string name() const override { return "resume"; }
    std::string description() const override { return i18n::t("cmd.resume.desc"); }
    std::string usage() const override { return i18n::t("cmd.resume.usage"); }
    CommandResult execute(CommandContext& ctx) override;
};

}  // namespace aicpp::commands
