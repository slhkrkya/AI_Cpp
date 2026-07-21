#include "providers/openai/OpenAIProvider.h"

#include <map>

#include "core/llm/SseFrameParser.h"
#include "core/net/CancelToken.h"

namespace aicpp::providers {

using llm::ChatRequest;
using llm::ChatResponse;
using llm::ContentBlock;
using llm::Message;
using llm::Role;
using llm::StreamCallback;
using llm::StreamEvent;

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

struct ToolCallAccumulator {
    std::string id;
    std::string name;
    std::string argsBuffer;
};

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
                    toolCalls.push_back({{"id", block.tool_call.id},
                                          {"type", "function"},
                                          {"function",
                                           {{"name", block.tool_call.name},
                                            {"arguments", block.tool_call.arguments.dump()}}}});
                }
            }
            nlohmann::json m{{"role", "assistant"}};
            m["content"] = text.empty() ? nlohmann::json(nullptr) : nlohmann::json(text);
            if (!toolCalls.empty()) m["tool_calls"] = toolCalls;
            messages.push_back(std::move(m));
        } else if (msg.role == Role::Tool) {
            for (const auto& block : msg.content) {
                if (block.type == ContentBlock::Type::ToolResult) {
                    messages.push_back({{"role", "tool"},
                                         {"tool_call_id", block.tool_result.tool_call_id},
                                         {"content", block.tool_result.content}});
                }
            }
        } else {
            messages.push_back({{"role", roleToString(msg.role)}, {"content", msg.textOnly()}});
        }
    }

    nlohmann::json body{
        {"model", request.model},
        {"messages", messages},
        {"stream", true},
        {"stream_options", {{"include_usage", true}}},
    };

    if (request.temperature) {
        body["temperature"] = *request.temperature;
    }

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

OpenAIProvider::OpenAIProvider(std::string apiKey, std::string baseUrl)
    : apiKey_(std::move(apiKey)), baseUrl_(std::move(baseUrl)) {}

ChatResponse OpenAIProvider::chatStream(const ChatRequest& request, const StreamCallback& onEvent) {
    ChatResponse response;

    std::string accumulatedText;
    std::map<int, ToolCallAccumulator> toolAcc;
    llm::Usage usage;

    llm::SseFrameParser parser([&](const std::string& payload, const StreamCallback& emit) {
        if (payload == "[DONE]") return;

        nlohmann::json j;
        try {
            j = nlohmann::json::parse(payload);
        } catch (const nlohmann::json::parse_error&) {
            return;
        }

        if (j.contains("usage") && j["usage"].is_object()) {
            usage.input_tokens = j["usage"].value("prompt_tokens", 0);
            usage.output_tokens = j["usage"].value("completion_tokens", 0);
        }

        if (!j.contains("choices") || j["choices"].empty()) return;
        const auto& choice = j["choices"][0];

        if (choice.contains("delta")) {
            const auto& delta = choice["delta"];
            if (delta.contains("content") && delta["content"].is_string()) {
                std::string piece = delta["content"].get<std::string>();
                if (!piece.empty()) {
                    accumulatedText += piece;
                    StreamEvent ev;
                    ev.type = StreamEvent::Type::TextDelta;
                    ev.text = piece;
                    emit(ev);
                }
            }
            if (delta.contains("tool_calls")) {
                for (const auto& tc : delta["tool_calls"]) {
                    int idx = tc.value("index", 0);
                    auto& acc = toolAcc[idx];
                    if (tc.contains("id") && tc["id"].is_string()) {
                        std::string idVal = tc["id"].get<std::string>();
                        if (!idVal.empty()) acc.id = idVal;
                    }
                    if (tc.contains("function")) {
                        const auto& fn = tc["function"];
                        if (fn.contains("name") && fn["name"].is_string()) {
                            acc.name += fn["name"].get<std::string>();
                        }
                        if (fn.contains("arguments") && fn["arguments"].is_string()) {
                            acc.argsBuffer += fn["arguments"].get<std::string>();
                        }
                    }
                }
            }
        }
    });

    const std::string body = buildRequestBody(request).dump();
    const std::string url = baseUrl_ + "/chat/completions";
    const std::vector<std::string> headers = {
        "Content-Type: application/json",
        "Authorization: Bearer " + apiKey_,
    };

    auto httpResp = http_.postStreaming(url, headers, body,
                                          [&](std::string_view chunk) { parser.feed(chunk, onEvent); },
                                          &net::cancelRequested());

    if (net::cancelRequested().load()) {
        response.stop_reason = "cancelled";
        if (!accumulatedText.empty()) {
            response.content.push_back(ContentBlock::makeText(accumulatedText));
        }
        return response;
    }

    if (!httpResp.ok()) {
        response.stop_reason = "error";
        StreamEvent errEv;
        errEv.type = StreamEvent::Type::Error;
        errEv.text = !httpResp.error.empty() ? httpResp.error
                                              : ("OpenAI HTTP " + std::to_string(httpResp.status_code));
        onEvent(errEv);
        return response;
    }

    if (!accumulatedText.empty()) {
        response.content.push_back(ContentBlock::makeText(accumulatedText));
    }
    for (auto& [idx, acc] : toolAcc) {
        ContentBlock block;
        block.type = ContentBlock::Type::ToolCall;
        block.tool_call.id = acc.id.empty() ? ("call_" + std::to_string(idx)) : acc.id;
        block.tool_call.name = acc.name;
        try {
            block.tool_call.arguments =
                acc.argsBuffer.empty() ? nlohmann::json::object() : nlohmann::json::parse(acc.argsBuffer);
        } catch (const nlohmann::json::parse_error&) {
            block.tool_call.arguments = nlohmann::json::object();
        }
        response.content.push_back(std::move(block));
    }

    response.stop_reason = toolAcc.empty() ? "end_turn" : "tool_use";
    response.usage = usage;
    return response;
}

}  // namespace aicpp::providers
