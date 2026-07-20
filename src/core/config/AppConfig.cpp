#include "core/config/AppConfig.h"

#include <fstream>

#include <nlohmann/json.hpp>

#include "core/config/Paths.h"

namespace aicpp::config {

AppConfig AppConfig::defaults() {
    AppConfig cfg;
    cfg.default_provider = "ollama";
    cfg.providers["ollama"] = ProviderConfig{"http://localhost:11434", "", "qwen2.5-coder:7b"};
    cfg.providers["openai"] = ProviderConfig{"", "OPENAI_API_KEY", "gpt-4o-mini"};
    return cfg;
}

AppConfig AppConfig::load() {
    auto path = configFilePath();

    std::ifstream in(path);
    if (!in) {
        AppConfig cfg = defaults();
        cfg.save();
        return cfg;
    }

    nlohmann::json j;
    try {
        in >> j;
    } catch (const nlohmann::json::parse_error&) {
        return defaults();
    }

    AppConfig cfg;
    cfg.default_provider = j.value("default_provider", std::string("ollama"));

    if (j.contains("providers") && j["providers"].is_object()) {
        for (auto& [id, entry] : j["providers"].items()) {
            ProviderConfig pc;
            pc.host = entry.value("host", std::string());
            pc.api_key_env = entry.value("api_key_env", std::string());
            pc.default_model = entry.value("default_model", std::string());
            cfg.providers[id] = pc;
        }
    }

    if (cfg.providers.empty()) {
        cfg = defaults();
    }

    return cfg;
}

void AppConfig::save() const {
    nlohmann::json j;
    j["default_provider"] = default_provider;
    j["providers"] = nlohmann::json::object();
    for (const auto& [id, pc] : providers) {
        nlohmann::json entry;
        if (!pc.host.empty()) entry["host"] = pc.host;
        if (!pc.api_key_env.empty()) entry["api_key_env"] = pc.api_key_env;
        if (!pc.default_model.empty()) entry["default_model"] = pc.default_model;
        j["providers"][id] = entry;
    }

    std::ofstream out(configFilePath());
    out << j.dump(2);
}

}  // namespace aicpp::config
