#pragma once

#include <memory>

#include "core/tools/FileTracker.h"
#include "core/tools/ITool.h"

namespace aicpp::tools {

// Diff-based edit: replaces a unique occurrence of old_string with new_string.
// Refuses if the file hasn't been read yet, or changed on disk since the last
// read/write we tracked (forces the model to re-read first).
class EditFileTool : public ITool {
public:
    explicit EditFileTool(std::shared_ptr<FileTracker> tracker) : tracker_(std::move(tracker)) {}

    std::string name() const override { return "edit_file"; }
    std::string description() const override {
        return "Bir dosyada old_string'in tek bir gecisini new_string ile degistirir. "
               "Once read_file ile dosya okunmus olmalidir.";
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
