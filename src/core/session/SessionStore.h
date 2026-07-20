#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "core/session/Session.h"

namespace aicpp::session {

class SessionStore {
public:
    explicit SessionStore(std::filesystem::path dir);

    // Atomic write (.tmp + rename) to <dir>/<id>.json.
    void save(SessionData& data) const;
    std::optional<SessionData> load(const std::string& id) const;

    // Metadata only, newest updated_at first.
    std::vector<SessionMeta> listRecent(size_t limit = 20) const;

    static std::string generateId();
    static std::string nowIso8601();

private:
    std::filesystem::path dir_;
};

}  // namespace aicpp::session
