#pragma once

#include <string>
#include <vector>

#include "core/llm/Schema.h"

namespace aicpp::session {

struct SessionMeta {
    std::string id;
    std::string title;
    std::string created_at;  // ISO-8601 UTC
    std::string updated_at;
    std::string cwd;
    std::string model_spec;  // "<provider>:<model>"
};

struct SessionData {
    SessionMeta meta;
    std::vector<llm::Message> transcript;
    std::vector<std::string> always_allowed_tools;  // "always allow" permission grants for this session
};

}  // namespace aicpp::session
