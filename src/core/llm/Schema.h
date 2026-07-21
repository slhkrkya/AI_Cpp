#pragma once

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace aicpp::llm {

// A single line in a unified-diff-style rendering of a file change (see
// core/tools/LineDiff.h). Lives here rather than in core/tools so that
// StreamEvent (below) can carry it without core/llm depending on core/tools.
enum class DiffLineType { Context, Added, Removed, Collapsed };

struct DiffLine {
    DiffLineType type;
    std::string text;
};

enum class Role { System, User, Assistant, Tool };

struct ToolCall {
    std::string id;
    std::string name;
    nlohmann::json arguments = nlohmann::json::object();
};

struct ToolResult {
    std::string tool_call_id;
    std::string tool_name;
    std::string content;
    bool is_error = false;
};

struct ContentBlock {
    enum class Type { Text, Thinking, ToolCall, ToolResult };

    Type type = Type::Text;
    std::string text;
    ToolCall tool_call;
    ToolResult tool_result;

    static ContentBlock makeText(std::string t) {
        ContentBlock b;
        b.type = Type::Text;
        b.text = std::move(t);
        return b;
    }
};

struct Message {
    Role role = Role::User;
    std::vector<ContentBlock> content;

    static Message user(std::string text) {
        Message m;
        m.role = Role::User;
        m.content.push_back(ContentBlock::makeText(std::move(text)));
        return m;
    }

    static Message assistantText(std::string text) {
        Message m;
        m.role = Role::Assistant;
        m.content.push_back(ContentBlock::makeText(std::move(text)));
        return m;
    }

    // Concatenates all Text blocks; used for providers/tools that just need plain text.
    std::string textOnly() const {
        std::string out;
        for (const auto& block : content) {
            if (block.type == ContentBlock::Type::Text) {
                out += block.text;
            }
        }
        return out;
    }
};

struct ToolDefinition {
    std::string name;
    std::string description;
    nlohmann::json parameters = nlohmann::json::object();
};

struct Usage {
    int input_tokens = 0;
    int output_tokens = 0;
};

struct ChatRequest {
    std::string model;
    std::optional<std::string> system_prompt;
    std::vector<Message> messages;
    std::vector<ToolDefinition> tools;
    int max_tokens = 4096;
    std::optional<double> temperature;
    bool stream = true;
};

struct ChatResponse {
    std::vector<ContentBlock> content;
    std::string stop_reason;  // "end_turn" | "tool_use" | "max_tokens" | "error"
    Usage usage;
};

struct StreamEvent {
    enum class Type {
        TextDelta,
        ToolCallStart,
        ToolCallArgsDelta,
        ToolCallEnd,
        ThinkingDelta,
        MessageDone,
        Error
    };

    Type type = Type::TextDelta;
    std::string text;
    int tool_index = -1;
    std::string tool_id;
    std::string tool_name;
    std::string args_fragment;
    nlohmann::json args_complete;
    std::optional<Usage> usage;

    // Populated for ToolCallEnd when the tool produced a diff preview (see
    // ToolExecResult::diff). nullopt = not applicable (most tools); empty
    // vector = computed but no visible change; non-empty = the actual diff.
    std::optional<std::vector<DiffLine>> diff;
};

using StreamCallback = std::function<void(const StreamEvent&)>;

}  // namespace aicpp::llm
