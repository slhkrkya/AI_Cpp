#pragma once

#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

#include "core/llm/Schema.h"

namespace aicpp::cli::theme {

// Detects NO_COLOR (https://no-color.org) once. Call after enableAnsiSupport().
void init();

// Whether ANSI styling should be emitted on `stream` right now: not disabled
// via NO_COLOR, and the stream is actually a terminal (not redirected/piped).
// stdout and stderr are checked independently - one can be redirected while
// the other stays attached to a real console.
bool richOutputEnabled(std::FILE* stream);

// Semantic, single-print-call helpers - never split a styled message across
// multiple print calls, since WorkflowEngine's progress callback can invoke
// these concurrently from several std::async threads and interleaved partial
// escapes are how colors leak into each other.
void info(std::string_view text);     // neutral, non-actionable info
void success(std::string_view text);  // confirmations
void warn(std::string_view text);     // non-fatal caution
void error(std::string_view text);    // always stderr

void sectionHeader(std::string_view text);  // "* Title" bullet header, e.g. for list headings
void planBanner(std::string_view body);     // bordered banner for entering PLAN MODE
void permissionPrompt(std::string_view header, std::string_view summary);  // the y/n/a tool prompt block

// Wraps the replxx prompt string (e.g. "> ") with color. replxx tolerates raw
// ANSI escapes embedded in the prompt it's given and accounts for their zero
// display width, so no change to cli/line_editor is needed.
std::string coloredPrompt(std::string_view plainPrompt);

// Inline (no trailing newline) styled fragment, e.g. for a prompt printed
// right before a blocking std::getline read.
std::string promptText(std::string_view text);

enum class ToolPhase { Start, End };
void toolBanner(std::string_view toolName, ToolPhase phase);

// Assistant-turn streaming: begin prints a role header and opens the
// assistant color; resume re-opens it (no header) after it was paused, e.g.
// by a tool-call banner appearing mid-turn; pause closes it. All three are
// idempotent no-ops when color is disabled.
void beginAssistantTurn();
void resumeAssistantColor();
void pauseAssistantColor();

// Centralizes the one raw (non-semantic) escape sequence in the codebase.
void clearScreen();

// Renders a file-change diff (see core/tools/LineDiff.h): Added lines green,
// Removed red, Collapsed (elided unchanged runs) dim, Context plain. Built
// into one combined string and printed via a single fmt::print call, same
// discipline as the rest of this module.
void diffBlock(const std::vector<llm::DiffLine>& lines);

}  // namespace aicpp::cli::theme
