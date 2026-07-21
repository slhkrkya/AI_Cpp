#include "commands/cmd_plan.h"

#include <fmt/core.h>

#include "cli/theme.h"

namespace aicpp::commands {

namespace {

// LLM-facing instruction, not user-facing UI text - deliberately left in
// English regardless of the interface language (see i18n scope note).
constexpr const char* kPlanModeNote =
    "You are in PLAN MODE. Do not call any write or execute tool - only read-only tools "
    "are permitted right now (calling one will simply be denied). Investigate the codebase "
    "using read-only tools as needed, then produce a numbered, concrete implementation plan "
    "and stop there. Wait for the user to type '/plan approve' before any changes are made.";

}  // namespace

CommandResult CmdPlan::execute(CommandContext& ctx) {
    auto& mode = ctx.app.appMode;
    const std::string& arg = ctx.rawArgs;

    if (arg == "approve") {
        if (mode != modes::AppMode::Planning) {
            cli::theme::error(i18n::t("plan.not_active"));
            return CommandResult::ShowError;
        }
        ctx.app.permissionManager.popOverride();
        ctx.app.session.clearExtraSystemNote();
        mode = modes::AppMode::Normal;
        fmt::print("\n");
        cli::theme::success(i18n::t("plan.approved"));
        ctx.app.pendingFollowUp =
            "The user approved the plan above. Proceed to implement it now using tools as "
            "needed, respecting normal permission prompts.";
        return CommandResult::Handled;
    }

    if (arg == "cancel") {
        if (mode != modes::AppMode::Planning) {
            cli::theme::warn(i18n::t("plan.not_active"));
            return CommandResult::Handled;
        }
        ctx.app.permissionManager.popOverride();
        ctx.app.session.clearExtraSystemNote();
        mode = modes::AppMode::Normal;
        cli::theme::info(i18n::t("plan.cancelled"));
        return CommandResult::Handled;
    }

    if (mode == modes::AppMode::Planning) {
        if (arg.empty()) {
            ctx.app.permissionManager.popOverride();
            ctx.app.session.clearExtraSystemNote();
            mode = modes::AppMode::Normal;
            cli::theme::info(i18n::t("plan.closed"));
            return CommandResult::Handled;
        }
        // Already planning and got more text: treat it as another planning turn.
        ctx.app.pendingFollowUp = arg;
        return CommandResult::Handled;
    }

    ctx.app.permissionManager.pushOverride(permissions::PolicyOverride::ReadOnlyOnly);
    ctx.app.session.setExtraSystemNote(kPlanModeNote);
    mode = modes::AppMode::Planning;
    cli::theme::planBanner(i18n::t("plan.enter_banner"));

    if (!arg.empty()) {
        ctx.app.pendingFollowUp = arg;
    }
    return CommandResult::Handled;
}

}  // namespace aicpp::commands
