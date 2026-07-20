#include "commands/cmd_resume.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>

#include <fmt/core.h>

#include "core/config/ProviderFactory.h"

namespace aicpp::commands {

namespace {

std::string roleLabel(llm::Role role) {
    switch (role) {
        case llm::Role::User:
            return "Sen";
        case llm::Role::Assistant:
            return "Asistan";
        case llm::Role::Tool:
            return "Arac";
        case llm::Role::System:
            return "Sistem";
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
            fmt::print("Kayitli oturum yok.\n");
            return CommandResult::Handled;
        }
        fmt::print("Gecmis oturumlar:\n");
        for (size_t i = 0; i < recent.size(); ++i) {
            std::string title = recent[i].title.empty() ? "(basliksiz)" : recent[i].title;
            fmt::print("  [{}] {}  {}  {}\n", i + 1, recent[i].id, recent[i].updated_at, title);
        }
        fmt::print("\nDevam etmek icin: /resume <numara-veya-id>\n");
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
        fmt::print("Oturum bulunamadi: {}\n", targetId);
        return CommandResult::ShowError;
    }

    std::string error;
    auto resolved = config::ProviderFactory::create(data->meta.model_spec, ctx.app.appConfig, error);
    if (resolved) {
        ctx.app.session.setProvider(resolved->provider);
        ctx.app.session.setModel(resolved->model);
        ctx.app.currentModelSpec = data->meta.model_spec;
    } else {
        fmt::print("Uyari: kayitli model '{}' yuklenemedi ({}), mevcut model korunuyor.\n",
                    data->meta.model_spec, error);
    }

    ctx.app.session.setHistory(data->transcript);
    ctx.app.currentSessionMeta = data->meta;

    std::string title = data->meta.title.empty() ? "(basliksiz)" : data->meta.title;
    fmt::print("\nOturum yuklendi: {} - {} ({} mesaj, model: {})\n", data->meta.id, title,
               data->transcript.size(), ctx.app.currentModelSpec);

    size_t start = data->transcript.size() > 4 ? data->transcript.size() - 4 : 0;
    fmt::print("\nSon mesajlar:\n");
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
