#pragma once

#include <filesystem>
#include <mutex>
#include <unordered_map>

namespace aicpp::tools {

// Tracks the last-known write time of files this process has Read or Written,
// so EditFileTool can refuse to patch a file that changed on disk since the
// model last looked at it (forces a fresh Read first).
class FileTracker {
public:
    void markSeen(const std::filesystem::path& path);
    bool isStale(const std::filesystem::path& path) const;
    bool hasBeenSeen(const std::filesystem::path& path) const;

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::filesystem::file_time_type> lastSeenWriteTime_;
};

}  // namespace aicpp::tools
