#pragma once

#include <string>
#include <unordered_map>

namespace aicpp::config {

struct ProviderConfig {
    std::string host;          // Ollama-style local providers
    std::string api_key_env;   // env var name holding the API key, for cloud providers
    std::string default_model;
};

struct AppConfig {
    std::string default_provider = "ollama";
    std::unordered_map<std::string, ProviderConfig> providers;

    static AppConfig defaults();

    // Loads from ~/.aicpp/config.json, creating it with defaults() if missing.
    static AppConfig load();
    void save() const;
};

}  // namespace aicpp::config
