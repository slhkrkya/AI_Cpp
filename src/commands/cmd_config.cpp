#include "commands/cmd_config.h"

#include <fstream>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "cli/theme.h"
#include "core/config/Paths.h"

namespace aicpp::commands {

CommandResult CmdConfig::execute(CommandContext&) {
    auto path = config::configFilePath();
    fmt::print("{}\n\n", fmt::format(fmt::runtime(i18n::t("config.path")), path.string()));

    std::ifstream in(path);
    if (!in) {
        cli::theme::error(i18n::t("config.read_failed"));
        return CommandResult::ShowError;
    }

    nlohmann::json j;
    try {
        in >> j;
    } catch (const nlohmann::json::parse_error&) {
        cli::theme::error(i18n::t("config.invalid_json"));
        return CommandResult::ShowError;
    }

    // Redact any saved API keys before printing - /config output can end up
    // on screen, in scrollback, or pasted somewhere; the raw file is fine to
    // edit directly, just not fine to echo verbatim.
    if (j.contains("providers") && j["providers"].is_object()) {
        for (auto& [id, entry] : j["providers"].items()) {
            if (entry.contains("api_key") && entry["api_key"].is_string() &&
                !entry["api_key"].get<std::string>().empty()) {
                entry["api_key"] = i18n::t("config.redacted_marker");
            }
        }
    }

    fmt::print("{}\n", j.dump(2));
    fmt::print("\n{}\n", i18n::t("config.footer"));
    return CommandResult::Handled;
}

}  // namespace aicpp::commands
