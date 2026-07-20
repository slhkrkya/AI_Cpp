#include "core/tools/GrepTool.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>

#include <fmt/format.h>
#include <re2/re2.h>

#include "core/tools/FsUtil.h"

namespace aicpp::tools {

namespace fs = std::filesystem;

nlohmann::json GrepTool::parametersSchema() const {
    return {
        {"type", "object"},
        {"properties",
         {
             {"pattern", {{"type", "string"}, {"description", "RE2 regex deseni"}}},
             {"path", {{"type", "string"}, {"description", "Aranacak kok dizin (varsayilan: '.')"}}},
             {"glob", {{"type", "string"}, {"description", "Sadece bu dosya adi desenine uyanlarda ara, orn. '*.cpp'"}}},
         }},
        {"required", nlohmann::json::array({"pattern"})},
    };
}

ToolExecResult GrepTool::execute(const nlohmann::json& args, ToolExecutionContext& ctx) {
    const std::string pattern = args.at("pattern").get<std::string>();
    const std::string subPath = args.value("path", std::string("."));
    const std::string globFilter = args.value("glob", std::string());
    fs::path root = ctx.working_directory / subPath;

    RE2 re(pattern);
    if (!re.ok()) {
        return {false, fmt::format("Gecersiz regex '{}': {}", pattern, re.error()), true};
    }

    std::error_code ec;
    if (!fs::exists(root, ec) || !fs::is_directory(root, ec)) {
        return {false, fmt::format("Dizin bulunamadi: {}", root.string()), true};
    }

    constexpr size_t kMaxResults = 200;
    constexpr uintmax_t kMaxFileBytes = 2 * 1024 * 1024;
    std::vector<std::string> results;

    auto it = fs::recursive_directory_iterator(root, fs::directory_options::skip_permission_denied, ec);
    const auto end = fs::recursive_directory_iterator();
    for (; it != end && results.size() < kMaxResults; it.increment(ec)) {
        const auto& entry = *it;
        std::error_code entryEc;
        bool isDir = entry.is_directory(entryEc);
        if (isDir && isIgnoredDirName(entry.path().filename().string())) {
            it.disable_recursion_pending();
            continue;
        }
        if (isDir) continue;

        uintmax_t size = entry.file_size(entryEc);
        if (entryEc || size > kMaxFileBytes) continue;

        std::string relative = fs::relative(entry.path(), root, ec).generic_string();
        if (!globFilter.empty() && !wildcardMatch(relative, globFilter)) continue;

        std::ifstream file(entry.path(), std::ios::binary);
        if (!file) continue;

        std::string line;
        int lineNo = 0;
        while (std::getline(file, line) && results.size() < kMaxResults) {
            ++lineNo;
            if (RE2::PartialMatch(line, re)) {
                results.push_back(fmt::format("{}:{}: {}", relative, lineNo, line));
            }
        }
    }

    if (results.empty()) {
        return {true, "Eslesme bulunamadi.", false};
    }

    std::string out;
    for (const auto& r : results) {
        out += r;
        out += "\n";
    }
    if (results.size() == kMaxResults) {
        out += fmt::format("[... sonuclar {} satir ile sinirlandirildi ...]\n", kMaxResults);
    }
    return {true, out, false};
}

}  // namespace aicpp::tools
