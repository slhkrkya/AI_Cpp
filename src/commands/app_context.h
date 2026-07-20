#pragma once

#include <string>

#include "core/agent/AgentSession.h"
#include "core/config/AppConfig.h"
#include "core/permissions/IPermissionManager.h"
#include "core/session/SessionStore.h"
#include "modes/app_mode.h"

namespace aicpp::commands {

class CommandRegistry;  // forward declaration; full type only needed where members are used

// Shared state slash-commands can act on.
struct AppContext {
    agent::AgentSession& session;
    CommandRegistry& registry;
    tools::ToolRegistry& toolRegistry;
    config::AppConfig& appConfig;
    std::string& currentModelSpec;  // "<provider>:<model>", e.g. "ollama:qwen2.5-coder:7b"
    session::SessionStore& sessionStore;
    session::SessionMeta& currentSessionMeta;
    permissions::IPermissionManager& permissionManager;
    modes::AppMode& appMode;
    // When a command sets this, the main loop submits it as the next user
    // turn right after the command returns (e.g. "/plan approve").
    std::string& pendingFollowUp;
};

}  // namespace aicpp::commands
