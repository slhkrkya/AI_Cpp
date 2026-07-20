#include "core/llm/Serialization.h"

namespace aicpp::llm {

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

Role roleFromString(const std::string& s) {
    if (s == "system") return Role::System;
    if (s == "assistant") return Role::Assistant;
    if (s == "tool") return Role::Tool;
    return Role::User;
}

std::string blockTypeToString(ContentBlock::Type t) {
    switch (t) {
        case ContentBlock::Type::Text:
            return "text";
        case ContentBlock::Type::Thinking:
            return "thinking";
        case ContentBlock::Type::ToolCall:
            return "tool_call";
        case ContentBlock::Type::ToolResult:
            return "tool_result";
    }
    return "text";
}

ContentBlock::Type blockTypeFromString(const std::string& s) {
    if (s == "thinking") return ContentBlock::Type::Thinking;
    if (s == "tool_call") return ContentBlock::Type::ToolCall;
    if (s == "tool_result") return ContentBlock::Type::ToolResult;
    return ContentBlock::Type::Text;
}

nlohmann::json blockToJson(const ContentBlock& b) {
    nlohmann::json j{{"type", blockTypeToString(b.type)}};
    switch (b.type) {
        case ContentBlock::Type::Text:
        case ContentBlock::Type::Thinking:
            j["text"] = b.text;
            break;
        case ContentBlock::Type::ToolCall:
            j["tool_call"] = {{"id", b.tool_call.id}, {"name", b.tool_call.name}, {"arguments", b.tool_call.arguments}};
            break;
        case ContentBlock::Type::ToolResult:
            j["tool_result"] = {{"tool_call_id", b.tool_result.tool_call_id},
                                 {"tool_name", b.tool_result.tool_name},
                                 {"content", b.tool_result.content},
                                 {"is_error", b.tool_result.is_error}};
            break;
    }
    return j;
}

ContentBlock blockFromJson(const nlohmann::json& j) {
    ContentBlock b;
    b.type = blockTypeFromString(j.value("type", std::string("text")));
    switch (b.type) {
        case ContentBlock::Type::Text:
        case ContentBlock::Type::Thinking:
            b.text = j.value("text", std::string());
            break;
        case ContentBlock::Type::ToolCall:
            if (j.contains("tool_call")) {
                b.tool_call.id = j["tool_call"].value("id", std::string());
                b.tool_call.name = j["tool_call"].value("name", std::string());
                b.tool_call.arguments = j["tool_call"].value("arguments", nlohmann::json::object());
            }
            break;
        case ContentBlock::Type::ToolResult:
            if (j.contains("tool_result")) {
                b.tool_result.tool_call_id = j["tool_result"].value("tool_call_id", std::string());
                b.tool_result.tool_name = j["tool_result"].value("tool_name", std::string());
                b.tool_result.content = j["tool_result"].value("content", std::string());
                b.tool_result.is_error = j["tool_result"].value("is_error", false);
            }
            break;
    }
    return b;
}

}  // namespace

nlohmann::json messageToJson(const Message& message) {
    nlohmann::json content = nlohmann::json::array();
    for (const auto& block : message.content) {
        content.push_back(blockToJson(block));
    }
    return {{"role", roleToString(message.role)}, {"content", content}};
}

Message messageFromJson(const nlohmann::json& j) {
    Message m;
    m.role = roleFromString(j.value("role", std::string("user")));
    if (j.contains("content") && j["content"].is_array()) {
        for (const auto& blockJson : j["content"]) {
            m.content.push_back(blockFromJson(blockJson));
        }
    }
    return m;
}

}  // namespace aicpp::llm
