#pragma once

#include <string_view>

#include "core/llm/Schema.h"

namespace aicpp::llm {

// Fed raw bytes as they arrive from the HTTP client's write callback.
// Implementations buffer partial lines/frames and emit zero or more
// normalized StreamEvents per call to feed().
class IStreamFrameParser {
public:
    virtual ~IStreamFrameParser() = default;
    virtual void feed(std::string_view chunk, const StreamCallback& onEvent) = 0;
};

}  // namespace aicpp::llm
