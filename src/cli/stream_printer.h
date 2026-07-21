#pragma once

#include <cstdio>

#include <fmt/core.h>

#include "cli/theme.h"
#include "core/llm/Schema.h"

namespace aicpp::cli {

// Renders one AgentSession::runTurn()'s StreamEvents: a colored role header
// on the first bit of assistant text, themed tool-call banners, and themed
// errors. Needs to be a stateful object rather than a free function - a
// `static bool` inside a free function would only ever fire once for the
// whole process, not once per turn, and a plain per-turn "have we printed the
// header yet" flag also has to survive tool calls that interrupt the text
// mid-turn (the color is paused for the tool banner, then silently resumed
// with no repeated header once text resumes).
class StreamPrinter {
public:
    // Call once before each AgentSession::runTurn() call.
    void beginTurn() {
        turnStarted_ = false;
        colorOpen_ = false;
    }

    // Call once after each AgentSession::runTurn() call returns, regardless of
    // how it ended - StreamEvent::Type::MessageDone is never actually emitted
    // by the current providers/AgentSession, so this is the only reliable
    // place to guarantee the assistant color gets closed.
    void endTurn() { closeColor(); }

    void operator()(const llm::StreamEvent& ev) {
        using Type = llm::StreamEvent::Type;
        switch (ev.type) {
            case Type::TextDelta:
                if (!turnStarted_) {
                    theme::beginAssistantTurn();
                    turnStarted_ = true;
                    colorOpen_ = true;
                } else if (!colorOpen_) {
                    theme::resumeAssistantColor();
                    colorOpen_ = true;
                }
                fmt::print("{}", ev.text);
                std::fflush(stdout);
                break;
            case Type::ToolCallStart:
                closeColor();
                theme::toolBanner(ev.tool_name, theme::ToolPhase::Start);
                break;
            case Type::ToolCallEnd:
                theme::toolBanner(ev.tool_name, theme::ToolPhase::End);
                if (ev.diff && !ev.diff->empty()) theme::diffBlock(*ev.diff);
                break;
            case Type::MessageDone:
                closeColor();
                break;
            case Type::Error:
                closeColor();
                theme::error(ev.text);
                break;
            default:
                break;
        }
    }

private:
    void closeColor() {
        if (colorOpen_) {
            theme::pauseAssistantColor();
            colorOpen_ = false;
        }
    }

    bool turnStarted_ = false;
    bool colorOpen_ = false;
};

}  // namespace aicpp::cli
