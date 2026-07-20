#pragma once

#include <filesystem>

namespace aicpp::config {

// Resolves ~/.aicpp (Windows: %USERPROFILE%\.aicpp, POSIX: $HOME/.aicpp) and
// ensures it (and its sessions/ and workflows/ and logs/ subdirs) exist.
std::filesystem::path aicppHome();

std::filesystem::path sessionsDir();
std::filesystem::path workflowsDir();
std::filesystem::path logsDir();
std::filesystem::path configFilePath();

}  // namespace aicpp::config
