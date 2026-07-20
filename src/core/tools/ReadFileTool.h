#pragma once

#include <memory>

#include "core/tools/FileTracker.h"
#include "core/tools/ITool.h"

namespace aicpp::tools {

class ReadFileTool : public ITool {
public:
    explicit ReadFileTool(std::shared_ptr<FileTracker> tracker) : tracker_(std::move(tracker)) {}

    std::string name() const override { return "read_file"; }
    std::string description() const override {
        return "Bir dosyanin icerigini okur. Duzenlemeden (edit_file) once dosyayi okumak zorunludur.";
    }
    nlohmann::json parametersSchema() const override;
    RiskCategory riskCategory() const override { return RiskCategory::Read; }
    bool isConcurrencySafe() const override { return true; }

    ToolExecResult execute(const nlohmann::json& args, ToolExecutionContext& ctx) override;

private:
    std::shared_ptr<FileTracker> tracker_;
};

}  // namespace aicpp::tools
