#include "core/config/ProviderFactory.h"

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

    if (providerId == "openai") {
        std::string envName = pc.api_key_env.empty() ? "OPENAI_API_KEY" : pc.api_key_env;
        const char* key = std::getenv(envName.c_str());
        if (!key || !*key) {
            error = fmt::format("{} ortam degiskeni tanimli degil.", envName);
            return std::nullopt;
        }
        return ResolvedModel{std::make_shared<providers::OpenAIProvider>(std::string(key)), model};
    }

    error = fmt::format("Bilinmeyen saglayici: '{}' (kullanilabilir: {})", providerId,
                         fmt::join(availableProviderIds(), ", "));
    return std::nullopt;
}

std::vector<std::string> ProviderFactory::availableProviderIds() { return {"ollama", "openai"}; }

std::string ProviderFactory::defaultSpec(const AppConfig& appConfig) {
    auto it = appConfig.providers.find(appConfig.default_provider);
    std::string model = (it != appConfig.providers.end()) ? it->second.default_model : "";
    return appConfig.default_provider + ":" + model;
}

}  // namespace aicpp::config
