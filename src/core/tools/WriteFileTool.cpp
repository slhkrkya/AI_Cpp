#include "core/tools/WriteFileTool.h"

#include <fstream>
#include <sstream>
#include <system_error>

#include <fmt/format.h>

#include "core/tools/LineDiff.h"
#include "i18n/Translator.h"

namespace aicpp::tools {

nlohmann::json WriteFileTool::parametersSchema() const {
    return {
        {"type", "object"},
        {"properties",
         {
             {"path", {{"type", "string"}, {"description", "Yazilacak dosyanin yolu"}}},
             {"content", {{"type", "string"}, {"description", "Dosyaya yazilacak tam icerik"}}},
         }},
        {"required", nlohmann::json::array({"path", "content"})},
    };
}

std::string WriteFileTool::summarize(const nlohmann::json& args) const {
    std::string path = args.value("path", "?");
    size_t bytes = args.value("content", std::string{}).size();
    return fmt::format("Write {} ({} bayt)", path, bytes);
}

ToolExecResult WriteFileTool::execute(const nlohmann::json& args, ToolExecutionContext& ctx) {
    auto path = ctx.working_directory / args.at("path").get<std::string>();
    std::string content = args.at("content").get<std::string>();

    std::error_code ec;
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), ec);
    }

    std::string oldContent;
    if (std::filesystem::exists(path, ec) && !ec) {
        std::ifstream in(path, std::ios::binary);
        if (in) {
            std::ostringstream buf;
            buf << in.rdbuf();
            oldContent = buf.str();
        }
    }

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        return {false, fmt::format("Dosyaya yazilamadi: {}", path.string()), true};
    }
    file.write(content.data(), static_cast<std::streamsize>(content.size()));
    file.close();

    tracker_->markSeen(path);

    ToolExecResult result;
    result.success = true;
    result.content_for_model = fmt::format("{} yazildi ({} bayt).", path.string(), content.size());
    result.is_error = false;
    if (auto diff = computeLineDiff(oldContent, content)) {
        result.diff = std::move(*diff);
    } else {
        result.diff = std::vector<llm::DiffLine>{{llm::DiffLineType::Collapsed, i18n::t("diff.too_large")}};
    }
    return result;
}

}  // namespace aicpp::tools
