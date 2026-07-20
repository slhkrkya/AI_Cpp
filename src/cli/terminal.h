#pragma once

namespace aicpp::cli {

// Enables ANSI/VT escape sequence processing on the current console.
// No-op on platforms where it's already the default (POSIX terminals).
void enableAnsiSupport();

}  // namespace aicpp::cli
