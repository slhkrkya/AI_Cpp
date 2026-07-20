#pragma once

#include <memory>
#include <string>

#include "core/llm/Schema.h"

namespace aicpp::llm {

class ILLMProvider {
public:
    virtual ~ILLMProvider() = default;

    virtual std::string id() const = 0;
    virtual bool supportsTools() const = 0;

    // Streams the response via onEvent (TextDelta/ToolCall*/MessageDone/Error) and
    // also returns the fully-assembled ChatResponse once the stream ends.
    virtual ChatResponse chatStream(const ChatRequest& request, const StreamCallback& onEvent) = 0;
};

using ProviderPtr = std::shared_ptr<ILLMProvider>;

}  // namespace aicpp::llm
