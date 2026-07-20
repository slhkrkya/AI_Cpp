#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "core/llm/ILLMProvider.h"
#include "core/permissions/IPermissionManager.h"
#include "core/tools/ToolRegistry.h"

namespace aicpp::agent {

// Drives one conversational turn end-to-end: builds the request (system
// prompt + history + tool defs), streams the provider's reply, and if the
// model requested tool calls, executes them (permission-gated) and loops
// until a final answer or kMaxToolIterations is hit.
class AgentSession {
public:
    AgentSession(llm::ProviderPtr provider, std::string model, std::string systemPrompt,
                 tools::ToolRegistry* toolRegistry = nullptr,
                 permissions::IPermissionManager* permissionManager = nullptr);

    void setProvider(llm::ProviderPtr provider) { provider_ = std::move(provider); }
    void setModel(std::string model) { model_ = std::move(model); }
    const std::string& model() const { return model_; }
    llm::ProviderPtr provider() const { return provider_; }

    void runTurn(const std::string& userInput, const llm::StreamCallback& onEvent);

    const std::vector<llm::Message>& history() const { return history_; }
    void clearHistory() { history_.clear(); }
    void setHistory(std::vector<llm::Message> messages) { history_ = std::move(messages); }

    // Appended to the base system prompt for the duration of a mode (e.g. Plan
    // Mode's read-only directive). Cleared when the mode ends.
    void setExtraSystemNote(std::string note) { extraSystemNote_ = std::move(note); }
    void clearExtraSystemNote() { extraSystemNote_.clear(); }

private:
    llm::ToolResult executeToolCall(const llm::ToolCall& call, const llm::StreamCallback& onEvent);

    llm::ProviderPtr provider_;
    std::string model_;
    std::string systemPrompt_;
    std::string extraSystemNote_;
    std::vector<llm::Message> history_;
    tools::ToolRegistry* toolRegistry_;
    permissions::IPermissionManager* permissionManager_;
    std::filesystem::path workingDirectory_;

    static constexpr int kMaxToolIterations = 25;
};

}  // namespace aicpp::agent
