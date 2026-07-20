#pragma once

#include "core/tools/ITool.h"

namespace aicpp::tools {

class BashTool : public ITool {
public:
    std::string name() const override { return "bash"; }
    std::string description() const override {
        return "Bir kabuk komutu calistirir (Windows'ta PowerShell, POSIX'te bash) ve "
               "stdout/stderr ciktisi ile cikis kodunu dondurur.";
    }
    nlohmann::json parametersSchema() const override;
    RiskCategory riskCategory() const override { return RiskCategory::Execute; }
    bool isConcurrencySafe() const override { return false; }

    ToolExecResult execute(const nlohmann::json& args, ToolExecutionContext& ctx) override;
    std::string summarize(const nlohmann::json& args) const override;
};

}  // namespace aicpp::tools
