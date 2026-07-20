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
    //
    // "ollama" is handled natively (NDJSON /api/chat). Every other provider id
    // is treated as an OpenAI-compatible chat-completions API - this covers
    // OpenAI itself plus Groq, Mistral, DeepSeek, Together, Fireworks, xAI/Grok,
    // Azure OpenAI, Gemini's openai-compat endpoint, local llama.cpp/LM Studio
    // servers, etc. Only api_key + base_url differ per id, both configurable
    // in config.json's providers.<id> (base_url required unless id == "openai").
    //
    // Returns nullopt (and fills `error`) if the spec is malformed or the
    // provider's api_key/base_url can't be resolved.
    static std::optional<ResolvedModel> create(const std::string& spec, const AppConfig& appConfig,
                                                 std::string& error);

    // Provider ids currently configured in config.json (informational only -
    // any id can be used via /model, configured ones just don't need to be
    // set up interactively again).
    static std::vector<std::string> availableProviderIds(const AppConfig& appConfig);

    // "<default_provider>:<default_model>" derived from config, e.g. "ollama:qwen2.5-coder:7b".
    static std::string defaultSpec(const AppConfig& appConfig);
};

}  // namespace aicpp::config
