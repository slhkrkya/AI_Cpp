#pragma once

#include <atomic>

namespace aicpp::net {

// Set to true by the OS-level Ctrl-C handler (installed via
// cli::installCtrlCHandler) while a stream is in flight; polled by
// HttpClient's write callback to abort the transfer early. Reset to false at
// the start of each turn (see main.cpp's runAndSave).
std::atomic<bool>& cancelRequested();

// True only while AgentSession::runTurn is actually blocked inside a
// provider_->chatStream() call - i.e. exactly the window during which Ctrl-C
// can be usefully turned into "abort this HTTP transfer" instead of the OS's
// default "terminate the process" behavior. False at all other times (REPL
// waiting for input, permission prompts, tool execution), so Ctrl-C there
// still terminates the app immediately, same as today.
std::atomic<bool>& streamInFlight();

}  // namespace aicpp::net
