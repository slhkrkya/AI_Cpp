#pragma once

#include <nlohmann/json.hpp>

#include "core/llm/Schema.h"

namespace aicpp::llm {

// Round-trips a full conversation (including tool calls/results) to/from JSON
// for session persistence. Free functions (not to_json/from_json overloads)
// to keep this explicit at call sites.
nlohmann::json messageToJson(const Message& message);
Message messageFromJson(const nlohmann::json& j);

}  // namespace aicpp::llm
