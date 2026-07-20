#include "core/config/Paths.h"

#include <cstdlib>

namespace aicpp::config {

namespace {

std::filesystem::path homeDir() {
#ifdef _WIN32
    const char* profile = std::getenv("USERPROFILE");
    if (profile && *profile) return std::filesystem::path(profile);
#else
    const char* home = std::getenv("HOME");
    if (home && *home) return std::filesystem::path(home);
#endif
    return std::filesystem::current_path();
}

}  // namespace

std::filesystem::path aicppHome() {
    std::filesystem::path home = homeDir() / ".aicpp";
    std::error_code ec;
    std::filesystem::create_directories(home, ec);
    std::filesystem::create_directories(home / "sessions", ec);
    std::filesystem::create_directories(home / "workflows", ec);
    std::filesystem::create_directories(home / "logs", ec);
    return home;
}

std::filesystem::path sessionsDir() { return aicppHome() / "sessions"; }
std::filesystem::path workflowsDir() { return aicppHome() / "workflows"; }
std::filesystem::path logsDir() { return aicppHome() / "logs"; }
std::filesystem::path configFilePath() { return aicppHome() / "config.json"; }

}  // namespace aicpp::config
