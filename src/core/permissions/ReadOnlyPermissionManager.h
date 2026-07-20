#pragma once

#include "core/permissions/IPermissionManager.h"

namespace aicpp::permissions {

// Used exclusively by workflow sub-agents: reads are auto-allowed, everything
// else is silently denied (no interactive prompt - concurrent sub-agents
// can't share a terminal prompt anyway). Real mutation happens only in the
// single, normally-permission-gated synthesis turn.
class ReadOnlyPermissionManager : public IPermissionManager {
public:
    Decision requestPermission(const std::string&, tools::RiskCategory risk, const std::string&) override {
        return risk == tools::RiskCategory::Read ? Decision::Allow : Decision::Deny;
    }

    void pushOverride(PolicyOverride) override {}
    void popOverride() override {}
};

}  // namespace aicpp::permissions
