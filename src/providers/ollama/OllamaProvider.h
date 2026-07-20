#pragma once

#include <string>

#include "core/llm/ILLMProvider.h"
#include "core/net/HttpClient.h"

namespace aicpp::providers {

class OllamaProvider : public llm::ILLMProvider {
public:
    explicit OllamaProvider(std::string host = "http://localhost:11434");

    std::string id() const override { return "ollama"; }
    bool supportsTools() const override { return true; }

    llm::ChatResponse chatStream(const llm::ChatRequest& request,
                                  const llm::StreamCallback& onEvent) override;

private:
    std::string host_;
    net::HttpClient http_;
};

}  // namespace aicpp::providers
