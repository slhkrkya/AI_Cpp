#include "commands/cmd_history.h"

#include <fmt/format.h>

#include "cli/theme.h"

namespace aicpp::commands {

CommandResult CmdHistory::execute(CommandContext& ctx) {
    const auto& meta = ctx.app.currentSessionMeta;
    fmt::print("{}\n", fmt::format(fmt::runtime(i18n::t("history.current")), meta.id,
                                     ctx.app.session.history().size(), meta.model_spec));

    auto recent = ctx.app.sessionStore.listRecent(20);
    if (recent.empty()) {
        fmt::print("\n");
        cli::theme::info(i18n::t("history.no_other"));
        return CommandResult::Handled;
    }

    fmt::print("\n");
    cli::theme::sectionHeader(i18n::t("history.recent_header"));
    for (const auto& m : recent) {
        std::string title = m.title.empty() ? i18n::t("common.untitled") : m.title;
        fmt::print("  {}  {}  {}{}\n", m.id, m.updated_at, title,
                   m.id == meta.id ? i18n::t("history.current_marker") : std::string());
    }
    fmt::print("\n{}\n", i18n::t("history.resume_hint"));
    return CommandResult::Handled;
}

}  // namespace aicpp::commands
