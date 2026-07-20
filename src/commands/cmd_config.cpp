#include "commands/cmd_config.h"

#include <fstream>

#include <fmt/core.h>
#include <nlohmann/json.hpp>

#include "core/config/Paths.h"

namespace aicpp::commands {

CommandResult CmdConfig::execute(CommandContext&) {
    auto path = config::configFilePath();
    fmt::print("Config dosyasi: {}\n\n", path.string());

    std::ifstream in(path);
    if (!in) {
        fmt::print("(dosya okunamadi)\n");
        return CommandResult::ShowError;
    }

    nlohmann::json j;
    try {
        in >> j;
    } catch (const nlohmann::json::parse_error&) {
        fmt::print("(dosya gecerli JSON degil)\n");
        return CommandResult::ShowError;
    }

    // Redact any saved API keys before printing - /config output can end up
    // on screen, in scrollback, or pasted somewhere; the raw file is fine to
    // edit directly, just not fine to echo verbatim.
    if (j.contains("providers") && j["providers"].is_object()) {
        for (auto& [id, entry] : j["providers"].items()) {
            if (entry.contains("api_key") && entry["api_key"].is_string() &&
                !entry["api_key"].get<std::string>().empty()) {
                entry["api_key"] = "***gizli*** (goruntuleme icin redakte edildi, dosyada duz metin duruyor)";
            }
        }
    }

    fmt::print("{}\n", j.dump(2));
    fmt::print("\nDegistirmek icin dosyayi bir editorde duzenleyip uygulamayi yeniden baslat.\n");
    return CommandResult::Handled;
}

}  // namespace aicpp::commands
