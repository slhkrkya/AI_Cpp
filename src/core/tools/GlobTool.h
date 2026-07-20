#pragma once

#include "core/tools/ITool.h"

namespace aicpp::tools {

class GlobTool : public ITool {
public:
    std::string name() const override { return "glob"; }
    std::string description() const override {
        return "Bir dizin agacinda dosya adi desenine gore arama yapar. '*' herhangi bir dizi "
               "(alt dizinler dahil) ile eslesir, '?' tek karakterle eslesir. Ornek: '*.cpp'.";
    }
    nlohmann::json parametersSchema() const override;
    RiskCategory riskCategory() const override { return RiskCategory::Read; }
    bool isConcurrencySafe() const override { return true; }

    ToolExecResult execute(const nlohmann::json& args, ToolExecutionContext& ctx) override;
};

}  // namespace aicpp::tools
