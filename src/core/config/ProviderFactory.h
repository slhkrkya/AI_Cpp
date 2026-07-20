#pragma once

#include <optional>
#include <string>
#include <vector>

#include "core/config/AppConfig.h"
#include "core/llm/ILLMProvider.h"

namespace aicpp::config {

struct ResolvedModel {
    llm::ProviderPtr provider;
    std::string model;
};

class ProviderFactory {
public:
    // spec format: "<provider>:<model>", e.g. "ollama:qwen2.5-coder:7b" or "openai:gpt-4o-mini".
    // Returns nullopt (and fills `error`) if the provider is unknown or a required env var is missing.
    static std::optional<ResolvedModel> create(const std::string& spec, const AppConfig& appConfig,
                                                 std::string& error);

    static std::vector<std::string> availableProviderIds();

    // "<default_provider>:<default_model>" derived from config, e.g. "ollama:qwen2.5-coder:7b".
    static std::string defaultSpec(const AppConfig& appConfig);
};

}  // namespace aicpp::config
