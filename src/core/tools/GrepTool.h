#pragma once

#include "core/tools/ITool.h"

namespace aicpp::tools {

class GrepTool : public ITool {
public:
    std::string name() const override { return "grep"; }
    std::string description() const override {
        return "Bir dizin agacinda dosya icerigi uzerinde regex arama yapar (RE2 sozdizimi) ve "
               "eslesen satirlari dondurur.";
    }
    nlohmann::json parametersSchema() const override;
    RiskCategory riskCategory() const override { return RiskCategory::Read; }
    bool isConcurrencySafe() const override { return true; }

    ToolExecResult execute(const nlohmann::json& args, ToolExecutionContext& ctx) override;
};

}  // namespace aicpp::tools
