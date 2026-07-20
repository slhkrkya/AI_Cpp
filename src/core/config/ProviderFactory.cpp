#include "core/config/ProviderFactory.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include "providers/ollama/OllamaProvider.h"
#include "providers/openai/OpenAIProvider.h"

namespace aicpp::config {

namespace {

std::string getEnvOr(const char* name, const std::string& fallback) {
    if (!name || !*name) return fallback;
    const char* value = std::getenv(name);
    return (value && *value) ? std::string(value) : fallback;
}

std::string toUpperGuessEnvName(const std::string& providerId) {
    std::string upper = providerId;
    std::transform(upper.begin(), upper.end(), upper.begin(),
                    [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return upper + "_API_KEY";
}

}  // namespace

std::optional<ResolvedModel> ProviderFactory::create(const std::string& spec, const AppConfig& appConfig,
                                                       std::string& error) {
    size_t colon = spec.find(':');
    if (colon == std::string::npos || colon == 0 || colon == spec.size() - 1) {
        error = "Format hatali. Kullanim: <saglayici>:<model>, orn. ollama:qwen2.5-coder:7b";
        return std::nullopt;
    }

    std::string providerId = spec.substr(0, colon);
    std::string model = spec.substr(colon + 1);

    auto it = appConfig.providers.find(providerId);
    ProviderConfig pc = (it != appConfig.providers.end()) ? it->second : ProviderConfig{};

    if (providerId == "ollama") {
        std::string host = getEnvOr("OLLAMA_HOST", pc.host.empty() ? "http://localhost:11434" : pc.host);
        return ResolvedModel{std::make_shared<providers::OllamaProvider>(host), model};
    }

    // Everything else: treated as an OpenAI-compatible chat-completions API.
    std::string apiKey = pc.api_key;
    if (apiKey.empty()) {
        std::string envName = pc.api_key_env.empty() ? toUpperGuessEnvName(providerId) : pc.api_key_env;
        const char* envVal = std::getenv(envName.c_str());
        if (envVal && *envVal) apiKey = envVal;
        if (apiKey.empty()) {
            error = fmt::format("'{}' icin API anahtari yok ({} tanimli degil, config.json'da da kayitli degil).",
                                 providerId, envName);
            return std::nullopt;
        }
    }

    std::string baseUrl = pc.base_url;
    if (baseUrl.empty()) {
        if (providerId == "openai") {
            baseUrl = "https://api.openai.com/v1";
        } else {
            error = fmt::format(
                "'{}' icin base_url tanimli degil. config.json'da providers.{}.base_url ekle "
                "(orn. Groq icin https://api.groq.com/openai/v1).",
                providerId, providerId);
            return std::nullopt;
        }
    }

    return ResolvedModel{std::make_shared<providers::OpenAIProvider>(apiKey, baseUrl), model};
}

std::vector<std::string> ProviderFactory::availableProviderIds(const AppConfig& appConfig) {
    std::vector<std::string> ids;
    ids.reserve(appConfig.providers.size());
    for (const auto& [id, pc] : appConfig.providers) {
        (void)pc;
        ids.push_back(id);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

std::string ProviderFactory::defaultSpec(const AppConfig& appConfig) {
    auto it = appConfig.providers.find(appConfig.default_provider);
    std::string model = (it != appConfig.providers.end()) ? it->second.default_model : "";
    return appConfig.default_provider + ":" + model;
}

}  // namespace aicpp::config
