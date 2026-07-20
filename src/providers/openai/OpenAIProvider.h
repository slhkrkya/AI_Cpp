#pragma once

#include <string>

#include "core/llm/ILLMProvider.h"
#include "core/net/HttpClient.h"

namespace aicpp::providers {

class OpenAIProvider : public llm::ILLMProvider {
public:
    explicit OpenAIProvider(std::string apiKey, std::string baseUrl = "https://api.openai.com/v1");

    std::string id() const override { return "openai"; }
    bool supportsTools() const override { return true; }

    llm::ChatResponse chatStream(const llm::ChatRequest& request,
                                  const llm::StreamCallback& onEvent) override;

private:
    std::string apiKey_;
    std::string baseUrl_;
    net::HttpClient http_;
};

}  // namespace aicpp::providers
