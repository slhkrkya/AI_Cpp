#pragma once

#include <functional>
#include <string>
#include <string_view>

#include <nlohmann/json.hpp>

#include "core/llm/IStreamFrameParser.h"
#include "core/llm/Schema.h"

namespace aicpp::llm {

// Ollama-style NDJSON: one bare JSON object per line, no "data:" prefix,
// no blank-line framing (unlike SSE).
class NdjsonFrameParser : public IStreamFrameParser {
public:
    using Translator = std::function<void(const nlohmann::json&, const StreamCallback&)>;

    explicit NdjsonFrameParser(Translator translator) : translator_(std::move(translator)) {}

    void feed(std::string_view chunk, const StreamCallback& onEvent) override {
        buffer_.append(chunk.data(), chunk.size());

        size_t pos;
        while ((pos = buffer_.find('\n')) != std::string::npos) {
            std::string line = buffer_.substr(0, pos);
            buffer_.erase(0, pos + 1);
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (line.empty()) {
                continue;
            }
            try {
                translator_(nlohmann::json::parse(line), onEvent);
            } catch (const nlohmann::json::parse_error&) {
                // Ignore malformed/partial lines; shouldn't happen at '\n' boundaries.
            }
        }
    }

private:
    Translator translator_;
    std::string buffer_;
};

}  // namespace aicpp::llm
