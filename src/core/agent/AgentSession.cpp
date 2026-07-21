#include "core/agent/AgentSession.h"

#include <fmt/format.h>

#include "core/net/CancelToken.h"

namespace aicpp::agent {

using llm::ChatRequest;
using llm::ChatResponse;
using llm::ContentBlock;
using llm::Message;
using llm::Role;
using llm::StreamCallback;
using llm::StreamEvent;
using llm::ToolCall;
using llm::ToolResult;

AgentSession::AgentSession(llm::ProviderPtr provider, std::string model, std::string systemPrompt,
                           tools::ToolRegistry* toolRegistry, permissions::IPermissionManager* permissionManager)
    : provider_(std::move(provider)),
      model_(std::move(model)),
      systemPrompt_(std::move(systemPrompt)),
      toolRegistry_(toolRegistry),
      permissionManager_(permissionManager),
      workingDirectory_(std::filesystem::current_path()) {}

void AgentSession::runTurn(const std::string& userInput, const StreamCallback& onEvent) {
    history_.push_back(Message::user(userInput));

    for (int iteration = 0; iteration < kMaxToolIterations; ++iteration) {
        if (net::cancelRequested().load()) return;  // cancelled while a tool was executing

        ChatRequest request;
        request.model = model_;
        request.system_prompt =
            extraSystemNote_.empty() ? systemPrompt_ : systemPrompt_ + "\n\n" + extraSystemNote_;
        request.messages = history_;
        request.stream = true;
        if (toolRegistry_) request.tools = toolRegistry_->allDefinitions();

        net::streamInFlight().store(true);
        ChatResponse response = provider_->chatStream(request, onEvent);
        net::streamInFlight().store(false);

        Message assistantMessage;
        assistantMessage.role = Role::Assistant;
        assistantMessage.content = response.content;
        history_.push_back(assistantMessage);

        if (response.stop_reason != "tool_use") {
            return;
        }

        std::vector<ContentBlock> resultBlocks;
        for (const auto& block : response.content) {
            if (block.type != ContentBlock::Type::ToolCall) continue;
            ToolResult tr = executeToolCall(block.tool_call, onEvent);
            ContentBlock rb;
            rb.type = ContentBlock::Type::ToolResult;
            rb.tool_result = tr;
            resultBlocks.push_back(std::move(rb));
        }

        if (resultBlocks.empty()) {
            return;  // model said tool_use but we found nothing to execute; avoid looping forever
        }

        Message toolMessage;
        toolMessage.role = Role::Tool;
        toolMessage.content = std::move(resultBlocks);
        history_.push_back(std::move(toolMessage));
    }
}

ToolResult AgentSession::executeToolCall(const ToolCall& call, const StreamCallback& onEvent) {
    ToolResult result;
    result.tool_call_id = call.id;
    result.tool_name = call.name;

    if (!toolRegistry_) {
        result.is_error = true;
        result.content = "Tool registry mevcut degil.";
        return result;
    }

    tools::ITool* tool = toolRegistry_->find(call.name);
    if (!tool) {
        result.is_error = true;
        result.content = fmt::format("Bilinmeyen arac: {}", call.name);
        return result;
    }

    if (permissionManager_) {
        auto decision = permissionManager_->requestPermission(tool->name(), tool->riskCategory(),
                                                                tool->summarize(call.arguments));
        if (decision == permissions::Decision::Deny) {
            result.is_error = true;
            result.content = "Kullanici bu islem icin izin vermedi.";
            return result;
        }
    }

    StreamEvent startEv;
    startEv.type = StreamEvent::Type::ToolCallStart;
    startEv.tool_id = call.id;
    startEv.tool_name = call.name;
    onEvent(startEv);

    tools::ToolExecutionContext ctx;
    ctx.working_directory = workingDirectory_;
    tools::ToolExecResult execResult = tool->execute(call.arguments, ctx);

    result.content = execResult.content_for_model;
    result.is_error = execResult.is_error;

    StreamEvent endEv;
    endEv.type = StreamEvent::Type::ToolCallEnd;
    endEv.tool_id = call.id;
    endEv.tool_name = call.name;
    endEv.diff = execResult.diff;
    onEvent(endEv);

    return result;
}

}  // namespace aicpp::agent
