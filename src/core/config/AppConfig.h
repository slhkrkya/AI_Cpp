#pragma once

#include <string>
#include <unordered_map>

namespace aicpp::config {

struct ProviderConfig {
    std::string host;          // Ollama-style local providers (NDJSON /api/chat)
    std::string base_url;      // any OpenAI-compatible chat-completions provider (Groq, Mistral,
                                // DeepSeek, Together, Fireworks, xAI, Azure OpenAI, Gemini's
                                // openai-compat endpoint, local llama.cpp/LM Studio servers, ...)
    std::string api_key_env;   // env var name holding the API key, for cloud providers
    std::string api_key;       // optional: literal key saved via `/model` for convenience
                                // (plain text in config.json - env var is preferred when set)
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
