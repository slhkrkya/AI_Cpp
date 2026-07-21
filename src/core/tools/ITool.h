#pragma once

#include <atomic>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "core/llm/Schema.h"

namespace aicpp::tools {

enum class RiskCategory { Read, Write, Execute, Network };

struct ToolExecutionContext {
    std::filesystem::path working_directory;
    std::atomic<bool>* cancel_token = nullptr;
};

struct ToolExecResult {
    bool success = true;
    std::string content_for_model;
    bool is_error = false;
    // nullopt = tool doesn't produce diffs (most tools); empty vector =
    // computed, no visible change; non-empty = the actual diff to show.
    std::optional<std::vector<llm::DiffLine>> diff;
};

class ITool {
public:
    virtual ~ITool() = default;

    virtual std::string name() const = 0;
    virtual std::string description() const = 0;
    virtual nlohmann::json parametersSchema() const = 0;
    virtual RiskCategory riskCategory() const = 0;
    virtual bool isConcurrencySafe() const = 0;

    virtual ToolExecResult execute(const nlohmann::json& args, ToolExecutionContext& ctx) = 0;

    // Human-readable one-liner shown in the permission prompt (or logged for
    // auto-allowed reads). Built by the tool itself so read tools can stay
    // silent while write/exec tools produce a reviewable summary.
    virtual std::string summarize(const nlohmann::json& args) const { return name() + " " + args.dump(); }
};

}  // namespace aicpp::tools
