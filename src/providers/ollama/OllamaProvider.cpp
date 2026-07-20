#include "providers/ollama/OllamaProvider.h"

#include "core/llm/NdjsonFrameParser.h"

namespace aicpp::providers {

using llm::ChatRequest;
using llm::ChatResponse;
using llm::ContentBlock;
using llm::Message;
using llm::Role;
using llm::StreamCallback;
using llm::StreamEvent;
using llm::ToolCall;

namespace {

std::string roleToString(Role role) {
    switch (role) {
        case Role::System:
            return "system";
        case Role::User:
            return "user";
        case Role::Assistant:
            return "assistant";
        case Role::Tool:
            return "tool";
    }
    return "user";
}

nlohmann::json buildRequestBody(const ChatRequest& request) {
    nlohmann::json messages = nlohmann::json::array();

    if (request.system_prompt) {
        messages.push_back({{"role", "system"}, {"content", *request.system_prompt}});
    }

    for (const auto& msg : request.messages) {
        if (msg.role == Role::Assistant) {
            std::string text;
            nlohmann::json toolCalls = nlohmann::json::array();
            for (const auto& block : msg.content) {
                if (block.type == ContentBlock::Type::Text) {
                    text += block.text;
                } else if (block.type == ContentBlock::Type::ToolCall) {
                    toolCalls.push_back(
                        {{"function", {{"name", block.tool_call.name}, {"arguments", block.tool_call.arguments}}}});
                }
            }
            nlohmann::json m{{"role", "assistant"}, {"content", text}};
            if (!toolCalls.empty()) m["tool_calls"] = toolCalls;
            messages.push_back(std::move(m));
        } else if (msg.role == Role::Tool) {
            for (const auto& block : msg.content) {
                if (block.type == ContentBlock::Type::ToolResult) {
                    messages.push_back({{"role", "tool"}, {"content", block.tool_result.content}});
                }
            }
        } else {
            messages.push_back({{"role", roleToString(msg.role)}, {"content", msg.textOnly()}});
        }
    }

    nlohmann::json body{
        {"model", request.model},
        {"messages", messages},
        {"stream", request.stream},
    };

    if (!request.tools.empty()) {
        nlohmann::json tools = nlohmann::json::array();
        for (const auto& tool : request.tools) {
            tools.push_back({{"type", "function"},
                              {"function",
                               {{"name", tool.name}, {"description", tool.description}, {"parameters", tool.parameters}}}});
        }
        body["tools"] = tools;
    }

    return body;
}

}  // namespace

OllamaProvider::OllamaProvider(std::string host) : host_(std::move(host)) {}

ChatResponse OllamaProvider::chatStream(const ChatRequest& request, const StreamCallback& onEvent) {
    ChatResponse response;

    std::string accumulatedText;
    std::vector<ContentBlock> toolCallBlocks;
    int toolCallCounter = 0;
    llm::Usage lastUsage;

    llm::NdjsonFrameParser parser([&](const nlohmann::json& j, const StreamCallback& emit) {
        if (j.contains("message") && j["message"].contains("content")) {
            std::string piece = j["message"]["content"].get<std::string>();
            if (!piece.empty()) {
                accumulatedText += piece;
                StreamEvent ev;
                ev.type = StreamEvent::Type::TextDelta;
                ev.text = piece;
                emit(ev);
            }
        }

        if (j.contains("message") && j["message"].contains("tool_calls")) {
            for (const auto& tc : j["message"]["tool_calls"]) {
                if (!tc.contains("function")) continue;

                // Only record the requested call here; AgentSession is the sole
                // emitter of ToolCallStart/End (it knows whether permission was
                // granted and when execution actually happens).
                ContentBlock block;
                block.type = ContentBlock::Type::ToolCall;
                block.tool_call.id = "call_" + std::to_string(toolCallCounter++);
                block.tool_call.name = tc["function"].value("name", "");
                block.tool_call.arguments = tc["function"].value("arguments", nlohmann::json::object());
                toolCallBlocks.push_back(block);
            }
        }

        if (j.value("done", false)) {
            lastUsage.input_tokens = j.value("prompt_eval_count", 0);
            lastUsage.output_tokens = j.value("eval_count", 0);

            StreamEvent doneEv;
            doneEv.type = StreamEvent::Type::MessageDone;
            doneEv.usage = lastUsage;
            emit(doneEv);
        }
    });

    const std::string body = buildRequestBody(request).dump();
    const std::string url = host_ + "/api/chat";

    auto httpResp = http_.postStreaming(
        url, {"Content-Type: application/json"}, body,
        [&](std::string_view chunk) { parser.feed(chunk, onEvent); });

    if (!httpResp.ok()) {
        response.stop_reason = "error";
        StreamEvent errEv;
        errEv.type = StreamEvent::Type::Error;
        errEv.text = !httpResp.error.empty()
                         ? httpResp.error
                         : ("Ollama HTTP " + std::to_string(httpResp.status_code));
        onEvent(errEv);
        response.content.push_back(ContentBlock::makeText(accumulatedText));
        return response;
    }

    if (!accumulatedText.empty()) {
        response.content.push_back(ContentBlock::makeText(accumulatedText));
    }
    for (auto& block : toolCallBlocks) {
        response.content.push_back(std::move(block));
    }

    response.stop_reason = toolCallBlocks.empty() ? "end_turn" : "tool_use";
    response.usage = lastUsage;
    return response;
}

}  // namespace aicpp::providers
