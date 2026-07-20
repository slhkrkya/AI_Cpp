#include "core/tools/GlobTool.h"

#include <filesystem>
#include <system_error>

#include <fmt/format.h>

#include "core/tools/FsUtil.h"

namespace aicpp::tools {

namespace fs = std::filesystem;

nlohmann::json GlobTool::parametersSchema() const {
    return {
        {"type", "object"},
        {"properties",
         {
             {"pattern", {{"type", "string"}, {"description", "Dosya adi deseni, orn. '*.cpp'"}}},
             {"path", {{"type", "string"}, {"description", "Aranacak kok dizin (varsayilan: '.')"}}},
         }},
        {"required", nlohmann::json::array({"pattern"})},
    };
}

ToolExecResult GlobTool::execute(const nlohmann::json& args, ToolExecutionContext& ctx) {
    const std::string pattern = args.at("pattern").get<std::string>();
    const std::string subPath = args.value("path", std::string("."));
    fs::path root = ctx.working_directory / subPath;

    std::error_code ec;
    if (!fs::exists(root, ec) || !fs::is_directory(root, ec)) {
        return {false, fmt::format("Dizin bulunamadi: {}", root.string()), true};
    }

    constexpr size_t kMaxResults = 500;
    std::vector<std::string> matches;

    auto it = fs::recursive_directory_iterator(root, fs::directory_options::skip_permission_denied, ec);
    const auto end = fs::recursive_directory_iterator();
    for (; it != end && matches.size() < kMaxResults; it.increment(ec)) {
        const auto& entry = *it;
        std::error_code entryEc;
        bool isDir = entry.is_directory(entryEc);
        if (isDir && isIgnoredDirName(entry.path().filename().string())) {
            it.disable_recursion_pending();
            continue;
        }
        if (isDir) continue;

        std::string relative = fs::relative(entry.path(), root, ec).generic_string();
        if (wildcardMatch(relative, pattern)) {
            matches.push_back(relative);
        }
    }

    if (matches.empty()) {
        return {true, "Eslesen dosya bulunamadi.", false};
    }

    std::string result;
    for (const auto& m : matches) {
        result += m;
        result += "\n";
    }
    if (matches.size() == kMaxResults) {
        result += fmt::format("[... sonuclar {} ile sinirlandirildi ...]\n", kMaxResults);
    }
    return {true, result, false};
}

}  // namespace aicpp::tools
