#pragma once

#include <string>
#include <vector>

#include "core/tools/ITool.h"

namespace aicpp::permissions {

enum class Decision { Allow, Deny, AlwaysAllowThisSession };

// Pushed by app-level modes (e.g. Plan Mode) to temporarily clamp behavior
// without touching the user's persistent policy/grants.
enum class PolicyOverride { ReadOnlyOnly };

class IPermissionManager {
public:
    virtual ~IPermissionManager() = default;

    // Blocks until the decision is known: auto-allowed by policy, previously
    // granted "always allow" for this tool this session, or answered by the user.
    virtual Decision requestPermission(const std::string& toolName, tools::RiskCategory risk,
                                        const std::string& humanSummary) = 0;

    virtual void pushOverride(PolicyOverride override) = 0;
    virtual void popOverride() = 0;

    // Exports the current "always allow" grants (tool names) for persistence,
    // e.g. saving alongside a session. Returns {} for permission managers
    // with no such state (e.g. ReadOnlyPermissionManager).
    virtual std::vector<std::string> exportAlwaysAllowed() const = 0;

    // Merges previously-persisted grants back in, e.g. on /resume - additive,
    // does not clear grants already made earlier in the current process.
    virtual void restoreAlwaysAllowed(const std::vector<std::string>& tools) = 0;
};

}  // namespace aicpp::permissions
