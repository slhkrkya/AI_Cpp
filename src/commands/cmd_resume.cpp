#include "commands/cmd_resume.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include "cli/theme.h"
#include "core/config/ProviderFactory.h"

namespace aicpp::commands {

namespace {

std::string roleLabel(llm::Role role) {
    switch (role) {
        case llm::Role::User:
            return i18n::t("role.user");
        case llm::Role::Assistant:
            return i18n::t("role.assistant");
        case llm::Role::Tool:
            return i18n::t("role.tool");
        case llm::Role::System:
            return i18n::t("role.system");
    }
    return "?";
}

bool isAllDigits(const std::string& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isdigit(c); });
}

}  // namespace

CommandResult CmdResume::execute(CommandContext& ctx) {
    auto recent = ctx.app.sessionStore.listRecent(20);

    if (ctx.rawArgs.empty()) {
        if (recent.empty()) {
            cli::theme::info(i18n::t("resume.none_saved"));
            return CommandResult::Handled;
        }
        cli::theme::sectionHeader(i18n::t("resume.list_header"));
        for (size_t i = 0; i < recent.size(); ++i) {
            std::string title = recent[i].title.empty() ? i18n::t("common.untitled") : recent[i].title;
            fmt::print("  [{}] {}  {}  {}\n", i + 1, recent[i].id, recent[i].updated_at, title);
        }
        fmt::print("\n{}\n", i18n::t("resume.hint"));
        return CommandResult::Handled;
    }

    std::string targetId = ctx.rawArgs;
    if (isAllDigits(targetId)) {
        size_t idx = std::strtoul(targetId.c_str(), nullptr, 10);
        if (idx >= 1 && idx <= recent.size()) {
            targetId = recent[idx - 1].id;
        }
    }

    auto data = ctx.app.sessionStore.load(targetId);
    if (!data) {
        cli::theme::error(fmt::format(fmt::runtime(i18n::t("resume.not_found")), targetId));
        return CommandResult::ShowError;
    }

    std::string error;
    auto resolved = config::ProviderFactory::create(data->meta.model_spec, ctx.app.appConfig, error);
    if (resolved) {
        ctx.app.session.setProvider(resolved->provider);
        ctx.app.session.setModel(resolved->model);
        ctx.app.currentModelSpec = data->meta.model_spec;
    } else {
        cli::theme::warn(fmt::format(fmt::runtime(i18n::t("resume.provider_warning")), data->meta.model_spec, error));
    }

    ctx.app.session.setHistory(data->transcript);
    ctx.app.currentSessionMeta = data->meta;
    ctx.app.permissionManager.restoreAlwaysAllowed(data->always_allowed_tools);

    std::string title = data->meta.title.empty() ? i18n::t("common.untitled") : data->meta.title;
    fmt::print("\n");
    cli::theme::success(fmt::format(fmt::runtime(i18n::t("resume.loaded")), data->meta.id, title,
                                      data->transcript.size(), ctx.app.currentModelSpec));

    if (!data->always_allowed_tools.empty()) {
        cli::theme::info(fmt::format(fmt::runtime(i18n::t("resume.grants_restored")),
                                       fmt::join(data->always_allowed_tools, ", ")));
    }

    size_t start = data->transcript.size() > 4 ? data->transcript.size() - 4 : 0;
    fmt::print("\n");
    cli::theme::sectionHeader(i18n::t("resume.recent_messages_header"));
    for (size_t i = start; i < data->transcript.size(); ++i) {
        const auto& msg = data->transcript[i];
        std::string text = msg.textOnly();
        if (text.size() > 200) text = text.substr(0, 200) + "...";
        if (!text.empty()) fmt::print("[{}] {}\n", roleLabel(msg.role), text);
    }
    fmt::print("\n");

    return CommandResult::Handled;
}

}  // namespace aicpp::commands
