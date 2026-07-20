#include "core/tools/ReadFileTool.h"

#include <fstream>
#include <sstream>
#include <system_error>

#include <fmt/format.h>

namespace aicpp::tools {

nlohmann::json ReadFileTool::parametersSchema() const {
    return {
        {"type", "object"},
        {"properties", {{"path", {{"type", "string"}, {"description", "Okunacak dosyanin yolu"}}}}},
        {"required", nlohmann::json::array({"path"})},
    };
}

ToolExecResult ReadFileTool::execute(const nlohmann::json& args, ToolExecutionContext& ctx) {
    auto path = ctx.working_directory / args.at("path").get<std::string>();

    std::error_code ec;
    if (!std::filesystem::exists(path, ec) || ec) {
        return {false, fmt::format("Dosya bulunamadi: {}", path.string()), true};
    }
    if (std::filesystem::is_directory(path, ec)) {
        return {false, fmt::format("{} bir dizin, dosya degil", path.string()), true};
    }

    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return {false, fmt::format("Dosya acilamadi: {}", path.string()), true};
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    constexpr size_t kMaxChars = 200000;
    bool truncated = content.size() > kMaxChars;
    if (truncated) content.resize(kMaxChars);

    tracker_->markSeen(path);

    if (truncated) {
        content += fmt::format("\n\n[... dosya cok uzun, ilk {} karakter gosterildi ...]", kMaxChars);
    }
    return {true, content, false};
}

}  // namespace aicpp::tools
