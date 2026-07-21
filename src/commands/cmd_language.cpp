#include "commands/cmd_language.h"

#include <algorithm>
#include <cctype>

#include <fmt/format.h>

#include "cli/theme.h"

namespace aicpp::commands {

namespace {

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

}  // namespace

CommandResult CmdLanguage::execute(CommandContext& ctx) {
    if (ctx.rawArgs.empty()) {
        auto lang = i18n::currentLanguage();
        fmt::print("{}\n",
                   fmt::format(fmt::runtime(i18n::t("language.current")), i18n::languageNativeName(lang),
                               i18n::languageCode(lang)));
        fmt::print("{}\n", i18n::t("language.available"));
        fmt::print("{}\n", i18n::t("language.usage_hint"));
        return CommandResult::Handled;
    }

    auto lang = i18n::parseLanguage(toLower(ctx.rawArgs));
    if (!lang) {
        cli::theme::error(fmt::format(fmt::runtime(i18n::t("language.invalid")), ctx.rawArgs));
        return CommandResult::ShowError;
    }

    i18n::setLanguage(*lang);
    ctx.app.appConfig.language = i18n::languageCode(*lang);
    ctx.app.appConfig.save();

    // Printed AFTER switching, in the newly-selected language - visible proof
    // the switch actually took effect immediately, no restart needed.
    cli::theme::success(
        fmt::format(fmt::runtime(i18n::t("language.switched")), i18n::languageNativeName(*lang), i18n::languageCode(*lang)));
    return CommandResult::Handled;
}

}  // namespace aicpp::commands
