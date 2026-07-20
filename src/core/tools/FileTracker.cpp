#include "core/tools/FileTracker.h"

#include <system_error>

namespace aicpp::tools {

void FileTracker::markSeen(const std::filesystem::path& path) {
    std::error_code ec;
    auto writeTime = std::filesystem::last_write_time(path, ec);
    if (ec) return;

    std::lock_guard<std::mutex> lock(mutex_);
    lastSeenWriteTime_[path.string()] = writeTime;
}

bool FileTracker::hasBeenSeen(const std::filesystem::path& path) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastSeenWriteTime_.find(path.string()) != lastSeenWriteTime_.end();
}

bool FileTracker::isStale(const std::filesystem::path& path) const {
    std::error_code ec;
    auto currentWriteTime = std::filesystem::last_write_time(path, ec);
    if (ec) return true;  // can't verify -> treat as stale, force a re-read

    std::lock_guard<std::mutex> lock(mutex_);
    auto it = lastSeenWriteTime_.find(path.string());
    if (it == lastSeenWriteTime_.end()) return true;
    return it->second != currentWriteTime;
}

}  // namespace aicpp::tools
