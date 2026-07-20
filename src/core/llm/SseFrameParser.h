#pragma once

#include <functional>
#include <string>
#include <string_view>

#include "core/llm/IStreamFrameParser.h"
#include "core/llm/Schema.h"

namespace aicpp::llm {

// Generic Server-Sent-Events framing (OpenAI, Anthropic, Gemini ?alt=sse):
// buffers until '\n', extracts "data:" line payloads, hands each raw payload
// string to the provider-specific translator (which parses its own JSON shape
// and understands its own "[DONE]"-style sentinels).
class SseFrameParser : public IStreamFrameParser {
public:
    using Translator = std::function<void(const std::string& dataPayload, const StreamCallback&)>;

    explicit SseFrameParser(Translator translator) : translator_(std::move(translator)) {}

    void feed(std::string_view chunk, const StreamCallback& onEvent) override {
        buffer_.append(chunk.data(), chunk.size());

        size_t pos;
        while ((pos = buffer_.find('\n')) != std::string::npos) {
            std::string line = buffer_.substr(0, pos);
            buffer_.erase(0, pos + 1);
            if (!line.empty() && line.back() == '\r') line.pop_back();

            if (line.rfind("data:", 0) != 0) {
                continue;  // ignore blank lines, "event:", "id:", ": comment" lines
            }
            std::string payload = line.substr(5);
            if (!payload.empty() && payload.front() == ' ') payload.erase(0, 1);
            translator_(payload, onEvent);
        }
    }

private:
    Translator translator_;
    std::string buffer_;
};

}  // namespace aicpp::llm
