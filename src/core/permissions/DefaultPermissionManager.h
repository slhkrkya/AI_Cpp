#pragma once

#include <unordered_set>
#include <vector>

#include "core/permissions/IPermissionManager.h"

namespace aicpp::permissions {

// Policy: Read is always auto-allowed. Write/Execute/Network prompt the user,
// unless the user previously chose "always allow" for that tool this session.
// While a ReadOnlyOnly override is active (Plan Mode), Write/Execute/Network
// are silently denied without prompting.
class DefaultPermissionManager : public IPermissionManager {
public:
    Decision requestPermission(const std::string& toolName, tools::RiskCategory risk,
                                const std::string& humanSummary) override;

    void pushOverride(PolicyOverride override) override { overrideStack_.push_back(override); }
    void popOverride() override {
        if (!overrideStack_.empty()) overrideStack_.pop_back();
    }

private:
    std::unordered_set<std::string> alwaysAllowed_;
    std::vector<PolicyOverride> overrideStack_;
};

}  // namespace aicpp::permissions
