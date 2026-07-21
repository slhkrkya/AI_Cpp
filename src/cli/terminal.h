#pragma once

namespace aicpp::cli {

// Enables ANSI/VT escape sequence processing on the current console.
// No-op on platforms where it's already the default (POSIX terminals).
void enableAnsiSupport();

// Installs a Ctrl-C handler that only intervenes while net::streamInFlight()
// is true (i.e. an AI response is actively streaming): it sets
// net::cancelRequested() and suppresses the default terminate-the-process
// behavior. At all other times (REPL waiting for input, permission prompts,
// tool execution), Ctrl-C is left to behave exactly as it does today - it
// terminates the app immediately.
void installCtrlCHandler();

}  // namespace aicpp::cli
