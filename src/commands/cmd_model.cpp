#include "commands/cmd_model.h"

#include <cctype>
#include <iostream>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include "cli/theme.h"
#include "core/config/ProviderFactory.h"

namespace aicpp::commands {

namespace {

bool readYesNo(const std::string& prompt) {
    fmt::print("{}", cli::theme::promptText(prompt));
    std::string answer;
    if (!std::getline(std::cin, answer) || answer.empty()) return false;
    return std::tolower(static_cast<unsigned char>(answer[0])) == 'y';
}

// If `spec` targets a provider that isn't fully configured yet (missing
// base_url and/or API key), ask for it right here, save it to config.json,
// and let the caller retry immediately - no env var juggling or app restart
// needed. Works for ANY provider id the user types, as long as it speaks the
// OpenAI-compatible chat-completions API (which covers most cloud LLM
// providers today, not just OpenAI itself).
bool tryInteractiveProviderSetup(CommandContext& ctx, const std::string& spec, const std::string& priorError) {
    size_t colon = spec.find(':');
    if (colon == std::string::npos) return false;  // malformed spec, nothing to configure

    std::string providerId = spec.substr(0, colon);
    std::string model = spec.substr(colon + 1);
    if (providerId == "ollama") return false;  // no key/base_url needed; failure here means "can't reach it"

    cli::theme::warn(priorError);
    fmt::print("{}\n", fmt::format(fmt::runtime(i18n::t("model.setup.explain")), providerId));
    if (!readYesNo("(y/n): ")) return false;

    auto& pc = ctx.app.appConfig.providers[providerId];

    if (pc.base_url.empty() && providerId != "openai") {
        fmt::print("{}", cli::theme::promptText(i18n::t("model.setup.ask_base_url")));
        std::string url;
        if (!std::getline(std::cin, url) || url.empty()) {
            cli::theme::warn(i18n::t("model.setup.url_empty"));
            return false;
        }
        pc.base_url = url;
    }

    if (pc.api_key.empty()) {
        fmt::print("{}", cli::theme::promptText(i18n::t("model.setup.ask_api_key")));
        std::string key;
        if (!std::getline(std::cin, key) || key.empty()) {
            cli::theme::warn(i18n::t("model.setup.key_empty"));
            return false;
        }
        pc.api_key = key;
    }

    if (pc.default_model.empty()) pc.default_model = model;
    ctx.app.appConfig.save();

    cli::theme::warn(i18n::t("model.setup.saved_plaintext_warning"));
    return true;
}

}  // namespace

CommandResult CmdModel::execute(CommandContext& ctx) {
    if (ctx.rawArgs.empty()) {
        fmt::print("{}\n", fmt::format(fmt::runtime(i18n::t("model.current")), ctx.app.currentModelSpec));
        fmt::print("{}\n", fmt::format(fmt::runtime(i18n::t("model.configured_providers")),
                                        fmt::join(config::ProviderFactory::availableProviderIds(ctx.app.appConfig),
                                                   ", ")));
        fmt::print("{}\n", i18n::t("model.switch_hint"));
        fmt::print("{}\n", i18n::t("model.other_provider_hint"));
        return CommandResult::Handled;
    }

    std::string error;
    auto resolved = config::ProviderFactory::create(ctx.rawArgs, ctx.app.appConfig, error);

    if (!resolved && tryInteractiveProviderSetup(ctx, ctx.rawArgs, error)) {
        resolved = config::ProviderFactory::create(ctx.rawArgs, ctx.app.appConfig, error);
    }

    if (!resolved) {
        cli::theme::error(fmt::format(fmt::runtime(i18n::t("model.change_failed")), error));
        return CommandResult::ShowError;
    }

    ctx.app.session.setProvider(resolved->provider);
    ctx.app.session.setModel(resolved->model);
    ctx.app.currentModelSpec = ctx.rawArgs;

    cli::theme::success(fmt::format(fmt::runtime(i18n::t("model.changed")), ctx.rawArgs));
    return CommandResult::Handled;
}

}  // namespace aicpp::commands
