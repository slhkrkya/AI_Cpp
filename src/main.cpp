#include <fmt/format.h>

#include <algorithm>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <memory>

#include "cli/dispatcher.h"
#include "cli/line_editor.h"
#include "cli/stream_printer.h"
#include "cli/terminal.h"
#include "cli/theme.h"
#include "commands/app_context.h"
#include "commands/cmd_clear.h"
#include "commands/cmd_config.h"
#include "commands/cmd_exit.h"
#include "commands/cmd_help.h"
#include "commands/cmd_history.h"
#include "commands/cmd_language.h"
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
#include "core/net/CancelToken.h"
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
#include "i18n/Language.h"
#include "i18n/Translator.h"
#include "modes/app_mode.h"

namespace {

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

int main(int argc, char** argv) {
    aicpp::cli::enableAnsiSupport();
    aicpp::cli::theme::init();
    aicpp::cli::installCtrlCHandler();

    auto appConfig = aicpp::config::AppConfig::load();
    auto lang = aicpp::i18n::parseLanguage(appConfig.language).value_or(aicpp::i18n::Language::Tr);
    aicpp::i18n::init(lang);

    // Optional positional argument: the project directory to work in (all
    // relative tool paths - glob/read_file/write_file/etc. - resolve against
    // the process cwd, so picking the wrong launch directory silently breaks
    // every relative path the model uses). Defaults to the current directory
    // when omitted, same as before this was added.
    if (argc > 1) {
        std::error_code ec;
        std::filesystem::path target = argv[1];
        if (!std::filesystem::is_directory(target, ec) || ec) {
            aicpp::cli::theme::error(
                fmt::format(fmt::runtime(aicpp::i18n::t("app.invalid_cwd")), target.string()));
            return 1;
        }
        std::filesystem::current_path(target, ec);
        if (ec) {
            aicpp::cli::theme::error(
                fmt::format(fmt::runtime(aicpp::i18n::t("app.invalid_cwd")), target.string()));
            return 1;
        }
    } else if (auto picked = aicpp::cli::pickWorkingDirectory(std::filesystem::current_path())) {
        // No path argument (e.g. launched by double-clicking the exe, where
        // there's no terminal to type one into) - let the user confirm/pick
        // the project folder instead of silently trusting whatever directory
        // Explorer happened to launch us in.
        std::error_code ec;
        std::filesystem::current_path(*picked, ec);
    }

    aicpp::workflow::ensureExampleSpec();

    std::string currentModelSpec = aicpp::config::ProviderFactory::defaultSpec(appConfig);
    std::string providerError;
    auto resolved = aicpp::config::ProviderFactory::create(currentModelSpec, appConfig, providerError);
    if (!resolved) {
        aicpp::cli::theme::error(
            fmt::format(fmt::runtime(aicpp::i18n::t("app.startup_provider_failed")), providerError));
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
    registry.registerCommand(std::make_shared<aicpp::commands::CmdLanguage>());

    aicpp::commands::AppContext appCtx{session,      registry,          toolRegistry,     appConfig,
                                        currentModelSpec, sessionStore,  currentSessionMeta, permissionManager,
                                        appMode,      pendingFollowUp};
    aicpp::cli::LineEditor editor;
    aicpp::cli::StreamPrinter streamPrinter;

    auto runAndSave = [&](const std::string& text) {
        if (currentSessionMeta.title.empty()) {
            currentSessionMeta.title = makeTitle(text);
        }
        currentSessionMeta.model_spec = currentModelSpec;

        aicpp::net::cancelRequested().store(false);
        streamPrinter.beginTurn();
        try {
            session.runTurn(text, std::ref(streamPrinter));
        } catch (const std::exception& e) {
            aicpp::cli::theme::error(
                fmt::format(fmt::runtime(aicpp::i18n::t("app.turn_failed")), e.what()));
        }
        streamPrinter.endTurn();
        fmt::print("\n\n");

        if (aicpp::net::cancelRequested().load()) {
            aicpp::cli::theme::warn(aicpp::i18n::t("app.cancelled"));
        }

        aicpp::session::SessionData data;
        data.meta = currentSessionMeta;
        data.transcript = session.history();
        data.always_allowed_tools = permissionManager.exportAlwaysAllowed();
        sessionStore.save(data);
        currentSessionMeta = data.meta;
    };

    fmt::print("{}\n", fmt::format(fmt::runtime(aicpp::i18n::t("app.banner")), currentModelSpec));
    aicpp::cli::theme::info(
        fmt::format(fmt::runtime(aicpp::i18n::t("app.cwd_line")), std::filesystem::current_path().string()));
    fmt::print("\n");

    while (true) {
        std::string prompt = appMode == aicpp::modes::AppMode::Planning ? "[PLAN] > " : "> ";
        auto lineOpt = editor.readLine(aicpp::cli::theme::coloredPrompt(prompt));
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

    fmt::print("{}\n", aicpp::i18n::t("app.bye"));
    return 0;
}
