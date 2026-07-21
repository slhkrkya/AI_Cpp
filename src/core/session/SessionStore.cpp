#include "core/session/SessionStore.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <random>
#include <sstream>

#include <nlohmann/json.hpp>

#include "core/llm/Serialization.h"

namespace aicpp::session {

SessionStore::SessionStore(std::filesystem::path dir) : dir_(std::move(dir)) {
    std::error_code ec;
    std::filesystem::create_directories(dir_, ec);
}

std::string SessionStore::nowIso8601() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

std::string SessionStore::generateId() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif

    std::random_device rd;
    std::uniform_int_distribution<int> dist(0, 15);
    std::ostringstream suffix;
    for (int i = 0; i < 6; ++i) suffix << std::hex << dist(rd);

    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y%m%d-%H%M%S") << "-" << suffix.str();
    return ss.str();
}

void SessionStore::save(SessionData& data) const {
    data.meta.updated_at = nowIso8601();
    if (data.meta.created_at.empty()) data.meta.created_at = data.meta.updated_at;

    nlohmann::json j;
    j["meta"] = {{"id", data.meta.id},
                 {"title", data.meta.title},
                 {"created_at", data.meta.created_at},
                 {"updated_at", data.meta.updated_at},
                 {"cwd", data.meta.cwd},
                 {"model_spec", data.meta.model_spec}};

    nlohmann::json transcript = nlohmann::json::array();
    for (const auto& msg : data.transcript) {
        transcript.push_back(llm::messageToJson(msg));
    }
    j["transcript"] = transcript;
    j["always_allowed_tools"] = data.always_allowed_tools;

    auto finalPath = dir_ / (data.meta.id + ".json");
    auto tmpPath = dir_ / (data.meta.id + ".json.tmp");

    std::ofstream out(tmpPath, std::ios::binary | std::ios::trunc);
    if (!out) return;
    out << j.dump(2);
    out.close();

    std::error_code ec;
    std::filesystem::rename(tmpPath, finalPath, ec);
}

std::optional<SessionData> SessionStore::load(const std::string& id) const {
    auto path = dir_ / (id + ".json");
    std::ifstream in(path);
    if (!in) return std::nullopt;

    nlohmann::json j;
    try {
        in >> j;
    } catch (const nlohmann::json::parse_error&) {
        return std::nullopt;
    }

    SessionData data;
    if (j.contains("meta")) {
        const auto& m = j["meta"];
        data.meta.id = m.value("id", id);
        data.meta.title = m.value("title", std::string());
        data.meta.created_at = m.value("created_at", std::string());
        data.meta.updated_at = m.value("updated_at", std::string());
        data.meta.cwd = m.value("cwd", std::string());
        data.meta.model_spec = m.value("model_spec", std::string());
    }
    if (j.contains("transcript") && j["transcript"].is_array()) {
        for (const auto& msgJson : j["transcript"]) {
            data.transcript.push_back(llm::messageFromJson(msgJson));
        }
    }
    data.always_allowed_tools = j.value("always_allowed_tools", std::vector<std::string>{});
    return data;
}

std::vector<SessionMeta> SessionStore::listRecent(size_t limit) const {
    std::vector<SessionMeta> result;
    std::error_code ec;
    if (!std::filesystem::exists(dir_, ec)) return result;

    for (const auto& entry : std::filesystem::directory_iterator(dir_, ec)) {
        if (entry.path().extension() != ".json") continue;

        std::ifstream in(entry.path());
        if (!in) continue;
        nlohmann::json j;
        try {
            in >> j;
        } catch (const nlohmann::json::parse_error&) {
            continue;
        }
        if (!j.contains("meta")) continue;

        SessionMeta meta;
        const auto& m = j["meta"];
        meta.id = m.value("id", entry.path().stem().string());
        meta.title = m.value("title", std::string());
        meta.created_at = m.value("created_at", std::string());
        meta.updated_at = m.value("updated_at", std::string());
        meta.cwd = m.value("cwd", std::string());
        meta.model_spec = m.value("model_spec", std::string());
        result.push_back(std::move(meta));
    }

    std::sort(result.begin(), result.end(),
              [](const SessionMeta& a, const SessionMeta& b) { return a.updated_at > b.updated_at; });
    if (result.size() > limit) result.resize(limit);
    return result;
}

}  // namespace aicpp::session
