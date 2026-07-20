#pragma once

#include <memory>

#include "core/tools/FileTracker.h"
#include "core/tools/ITool.h"

namespace aicpp::tools {

class WriteFileTool : public ITool {
public:
    explicit WriteFileTool(std::shared_ptr<FileTracker> tracker) : tracker_(std::move(tracker)) {}

    std::string name() const override { return "write_file"; }
    std::string description() const override {
        return "Bir dosyayi baştan yazar (var olan icerigin tamamini degistirir) veya olusturur.";
    }
    nlohmann::json parametersSchema() const override;
    RiskCategory riskCategory() const override { return RiskCategory::Write; }
    bool isConcurrencySafe() const override { return false; }

    ToolExecResult execute(const nlohmann::json& args, ToolExecutionContext& ctx) override;
    std::string summarize(const nlohmann::json& args) const override;

private:
    std::shared_ptr<FileTracker> tracker_;
};

}  // namespace aicpp::tools
