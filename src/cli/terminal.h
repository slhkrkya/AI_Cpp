#pragma once

#include <filesystem>
#include <optional>

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

// Shows a native folder-picker dialog seeded at initialDir, so a user who
// launched aicpp.exe by double-clicking it (no terminal, no way to pass a
// path argument) can still choose which project directory to work in.
// Returns nullopt if the user cancelled, or on platforms without a dialog
// (non-Windows) - callers should just keep the current directory in that
// case, exactly like today.
std::optional<std::filesystem::path> pickWorkingDirectory(const std::filesystem::path& initialDir);

}  // namespace aicpp::cli
