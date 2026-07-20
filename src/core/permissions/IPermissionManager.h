#pragma once

#include <string>

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
};

}  // namespace aicpp::permissions
