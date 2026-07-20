#include "commands/cmd_plan.h"

#include <fmt/core.h>

namespace aicpp::commands {

namespace {

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
            fmt::print("Su an plan modunda degilsin.\n");
            return CommandResult::ShowError;
        }
        ctx.app.permissionManager.popOverride();
        ctx.app.session.clearExtraSystemNote();
        mode = modes::AppMode::Normal;
        fmt::print("\nPlan onaylandi, uygulamaya geciliyor...\n");
        ctx.app.pendingFollowUp =
            "The user approved the plan above. Proceed to implement it now using tools as "
            "needed, respecting normal permission prompts.";
        return CommandResult::Handled;
    }

    if (arg == "cancel") {
        if (mode != modes::AppMode::Planning) {
            fmt::print("Su an plan modunda degilsin.\n");
            return CommandResult::Handled;
        }
        ctx.app.permissionManager.popOverride();
        ctx.app.session.clearExtraSystemNote();
        mode = modes::AppMode::Normal;
        fmt::print("Plan iptal edildi (plan gecmiste kaliyor ama uygulanmayacak).\n");
        return CommandResult::Handled;
    }

    if (mode == modes::AppMode::Planning) {
        if (arg.empty()) {
            ctx.app.permissionManager.popOverride();
            ctx.app.session.clearExtraSystemNote();
            mode = modes::AppMode::Normal;
            fmt::print("Plan modu kapatildi.\n");
            return CommandResult::Handled;
        }
        // Already planning and got more text: treat it as another planning turn.
        ctx.app.pendingFollowUp = arg;
        return CommandResult::Handled;
    }

    ctx.app.permissionManager.pushOverride(permissions::PolicyOverride::ReadOnlyOnly);
    ctx.app.session.setExtraSystemNote(kPlanModeNote);
    mode = modes::AppMode::Planning;
    fmt::print(
        "\n● PLAN MODE - sadece okuma araclari calisir.\n"
        "  Onaylamak icin: /plan approve   Iptal icin: /plan cancel\n\n");

    if (!arg.empty()) {
        ctx.app.pendingFollowUp = arg;
    }
    return CommandResult::Handled;
}

}  // namespace aicpp::commands
