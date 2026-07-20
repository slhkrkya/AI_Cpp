#include <fmt/core.h>

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <memory>

#include "cli/dispatcher.h"
#include "cli/line_editor.h"
#include "cli/terminal.h"
#include "commands/app_context.h"
#include "commands/cmd_clear.h"
#include "commands/cmd_config.h"
#include "commands/cmd_exit.h"
#include "commands/cmd_help.h"
#include "commands/cmd_history.h"
#include "commands/cmd_model.h"
#include "commands/cmd_plan.h"
#include "commands/cmd_resume.h"
#include "commands/cmd_tools.h"
#include "commands/cmd_workflow.h"
#include "commands/command_registry.h"
#include "core/agent/AgentSession.h"
#include "core/config/AppConfig.h"
#include "core/config/Paths.h"
#include "core/config/ProviderFactory.h"
#include "core/llm/Schema.h"
#include "core/permissions/DefaultPermissionManager.h"
#include "core/session/SessionStore.h"
#include "core/tools/BashTool.h"
#include "core/tools/EditFileTool.h"
#include "core/tools/FileTracker.h"
#include "core/tools/GlobTool.h"
#include "core/tools/GrepTool.h"
#include "core/tools/ReadFileTool.h"
#include "core/tools/ToolRegistry.h"
#include "core/tools/WriteFileTool.h"
#include "core/workflow/WorkflowStore.h"
#include "modes/app_mode.h"

namespace {

void printStreamEvent(const aicpp::llm::StreamEvent& ev) {
    using Type = aicpp::llm::StreamEvent::Type;
    switch (ev.type) {
        case Type::TextDelta:
            fmt::print("{}", ev.text);
            std::fflush(stdout);
            break;
        case Type::ToolCallStart:
            fmt::print("\n[arac calisiyor: {}]\n", ev.tool_name);
            break;
        case Type::ToolCallEnd:
            fmt::print("[arac tamamlandi: {}]\n", ev.tool_name);
            break;
        case Type::Error:
            fmt::print(stderr, "\n[hata] {}\n", ev.text);
            break;
        default:
            break;
    }
}

std::string makeTitle(const std::string& firstMessage) {
    constexpr size_t kMaxLen = 60;
    std::string title = firstMessage;
    std::replace(title.begin(), title.end(), '\n', ' ');
    if (title.size() > kMaxLen) {
        title.resize(kMaxLen);
        title += "...";
    }
    return title;
}

}  // namespace

int main() {
    aicpp::cli::enableAnsiSupport();

    auto appConfig = aicpp::config::AppConfig::load();
    aicpp::workflow::ensureExampleSpec();

    std::string currentModelSpec = aicpp::config::ProviderFactory::defaultSpec(appConfig);
    std::string providerError;
    auto resolved = aicpp::config::ProviderFactory::create(currentModelSpec, appConfig, providerError);
    if (!resolved) {
        fmt::print(stderr, "Baslangic sagliyicisi kurulamadi: {}\n", providerError);
        return 1;
    }

    auto fileTracker = std::make_shared<aicpp::tools::FileTracker>();

    aicpp::tools::ToolRegistry toolRegistry;
    toolRegistry.registerTool(std::make_shared<aicpp::tools::ReadFileTool>(fileTracker));
    toolRegistry.registerTool(std::make_shared<aicpp::tools::WriteFileTool>(fileTracker));
    toolRegistry.registerTool(std::make_shared<aicpp::tools::EditFileTool>(fileTracker));
    toolRegistry.registerTool(std::make_shared<aicpp::tools::GlobTool>());
    toolRegistry.registerTool(std::make_shared<aicpp::tools::GrepTool>());
    toolRegistry.registerTool(std::make_shared<aicpp::tools::BashTool>());

    aicpp::permissions::DefaultPermissionManager permissionManager;

    aicpp::agent::AgentSession session(resolved->provider, resolved->model,
                                        "You are aicpp, a helpful terminal coding assistant. "
                                        "Only call a tool when it's actually necessary to answer "
                                        "accurately (e.g. inspecting or modifying real files in the "
                                        "user's project). For greetings, opinions, or general "
                                        "questions that don't require looking at the project, answer "
                                        "directly in plain text without calling any tool. If a tool "
                                        "call is denied, do not immediately retry it - explain that to "
                                        "the user in text instead.",
                                        &toolRegistry, &permissionManager);

    aicpp::session::SessionStore sessionStore(aicpp::config::sessionsDir());
    aicpp::session::SessionMeta currentSessionMeta;
    currentSessionMeta.id = aicpp::session::SessionStore::generateId();
    currentSessionMeta.cwd = std::filesystem::current_path().string();
    currentSessionMeta.model_spec = currentModelSpec;

    aicpp::modes::AppMode appMode = aicpp::modes::AppMode::Normal;
    std::string pendingFollowUp;

    aicpp::commands::CommandRegistry registry;
    registry.registerCommand(std::make_shared<aicpp::commands::CmdHelp>());
    registry.registerCommand(std::make_shared<aicpp::commands::CmdExit>());
    registry.registerCommand(std::make_shared<aicpp::commands::CmdClear>());
    registry.registerCommand(std::make_shared<aicpp::commands::CmdModel>());
    registry.registerCommand(std::make_shared<aicpp::commands::CmdTools>());
    registry.registerCommand(std::make_shared<aicpp::commands::CmdConfig>());
    registry.registerCommand(std::make_shared<aicpp::commands::CmdHistory>());
    registry.registerCommand(std::make_shared<aicpp::commands::CmdResume>());
    registry.registerCommand(std::make_shared<aicpp::commands::CmdPlan>());
    registry.registerCommand(std::make_shared<aicpp::commands::CmdWorkflow>());

    aicpp::commands::AppContext appCtx{session,      registry,          toolRegistry,     appConfig,
                                        currentModelSpec, sessionStore,  currentSessionMeta, permissionManager,
                                        appMode,      pendingFollowUp};
    aicpp::cli::LineEditor editor;

    auto runAndSave = [&](const std::string& text) {
        if (currentSessionMeta.title.empty()) {
            currentSessionMeta.title = makeTitle(text);
        }
        currentSessionMeta.model_spec = currentModelSpec;

        session.runTurn(text, printStreamEvent);
        fmt::print("\n\n");

        aicpp::session::SessionData data;
        data.meta = currentSessionMeta;
        data.transcript = session.history();
        sessionStore.save(data);
        currentSessionMeta = data.meta;
    };

    fmt::print("aicpp v0.7 - /help ile komutlari gorebilirsin. Aktif model: {}\n\n", currentModelSpec);

    while (true) {
        std::string prompt = appMode == aicpp::modes::AppMode::Planning ? "[PLAN] > " : "> ";
        auto lineOpt = editor.readLine(prompt);
        if (!lineOpt) break;

        const std::string& line = *lineOpt;
        if (line.empty()) continue;

        auto outcome = aicpp::cli::dispatchLine(line, registry, appCtx);
        if (outcome == aicpp::cli::DispatchOutcome::ExitRequested) break;
        if (outcome == aicpp::cli::DispatchOutcome::Handled) {
            if (!pendingFollowUp.empty()) {
                std::string followUp = pendingFollowUp;
                pendingFollowUp.clear();
                runAndSave(followUp);
            }
            continue;
        }

        runAndSave(line);
    }

    fmt::print("Bye.\n");
    return 0;
}
