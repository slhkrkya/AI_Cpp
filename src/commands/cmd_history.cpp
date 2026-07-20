#include "commands/cmd_history.h"

#include <fmt/core.h>

namespace aicpp::commands {

CommandResult CmdHistory::execute(CommandContext& ctx) {
    const auto& meta = ctx.app.currentSessionMeta;
    fmt::print("Mevcut oturum: {} ({} mesaj, model: {})\n", meta.id, ctx.app.session.history().size(),
               meta.model_spec);

    auto recent = ctx.app.sessionStore.listRecent(20);
    if (recent.empty()) {
        fmt::print("\nKayitli baska oturum yok.\n");
        return CommandResult::Handled;
    }

    fmt::print("\nSon oturumlar:\n");
    for (const auto& m : recent) {
        std::string title = m.title.empty() ? "(basliksiz)" : m.title;
        fmt::print("  {}  {}  {}{}\n", m.id, m.updated_at, title, m.id == meta.id ? "  <- bu oturum" : "");
    }
    fmt::print("\n/resume <id> ile bir oturuma devam edebilirsin (M5).\n");
    return CommandResult::Handled;
}

}  // namespace aicpp::commands
