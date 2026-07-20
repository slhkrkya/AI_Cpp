#include "core/tools/EditFileTool.h"

#include <fstream>
#include <sstream>
#include <system_error>

#include <fmt/format.h>

namespace aicpp::tools {

nlohmann::json EditFileTool::parametersSchema() const {
    return {
        {"type", "object"},
        {"properties",
         {
             {"path", {{"type", "string"}, {"description", "Duzenlenecek dosyanin yolu"}}},
             {"old_string", {{"type", "string"}, {"description", "Degistirilecek benzersiz metin"}}},
             {"new_string", {{"type", "string"}, {"description", "Yerine yazilacak metin"}}},
         }},
        {"required", nlohmann::json::array({"path", "old_string", "new_string"})},
    };
}

std::string EditFileTool::summarize(const nlohmann::json& args) const {
    return fmt::format("Edit {}", args.value("path", "?"));
}

ToolExecResult EditFileTool::execute(const nlohmann::json& args, ToolExecutionContext& ctx) {
    auto path = ctx.working_directory / args.at("path").get<std::string>();
    const std::string oldStr = args.at("old_string").get<std::string>();
    const std::string newStr = args.at("new_string").get<std::string>();

    std::error_code ec;
    if (!std::filesystem::exists(path, ec) || ec) {
        return {false, fmt::format("Dosya bulunamadi: {}", path.string()), true};
    }

    if (!tracker_->hasBeenSeen(path)) {
        return {false,
                fmt::format("{} once read_file ile okunmadi. Once oku, sonra tekrar dene.", path.string()),
                true};
    }
    if (tracker_->isStale(path)) {
        return {false,
                fmt::format("{} son okumadan beri diskte degismis. Once read_file ile tekrar oku.",
                             path.string()),
                true};
    }

    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return {false, fmt::format("Dosya acilamadi: {}", path.string()), true};
    }
    std::ostringstream buf;
    buf << in.rdbuf();
    std::string content = buf.str();
    in.close();

    size_t firstPos = content.find(oldStr);
    if (firstPos == std::string::npos) {
        return {false, "old_string dosyada bulunamadi. Once dosyayi read_file ile tekrar okuyup tam eslesen metni kullan.",
                true};
    }
    size_t secondPos = content.find(oldStr, firstPos + 1);
    if (secondPos != std::string::npos) {
        return {false,
                "old_string dosyada birden fazla yerde geciyor; benzersiz olmasi icin daha fazla "
                "baglam (surrounding context) ekle.",
                true};
    }

    content.replace(firstPos, oldStr.size(), newStr);

    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) {
        return {false, fmt::format("Dosyaya yazilamadi: {}", path.string()), true};
    }
    out.write(content.data(), static_cast<std::streamsize>(content.size()));
    out.close();

    tracker_->markSeen(path);
    return {true, fmt::format("{} guncellendi.", path.string()), false};
}

}  // namespace aicpp::tools
