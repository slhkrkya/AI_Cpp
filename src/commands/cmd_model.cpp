#include "commands/cmd_model.h"

#include <fmt/core.h>
#include <fmt/ranges.h>

#include "core/config/ProviderFactory.h"

namespace aicpp::commands {

CommandResult CmdModel::execute(CommandContext& ctx) {
    if (ctx.rawArgs.empty()) {
        fmt::print("Mevcut model: {}\n", ctx.app.currentModelSpec);
        fmt::print("Kullanilabilir saglayicilar: {}\n", fmt::join(config::ProviderFactory::availableProviderIds(), ", "));
        fmt::print("Degistirmek icin: /model <saglayici>:<model>  (orn. /model openai:gpt-4o-mini)\n");
        return CommandResult::Handled;
    }

    std::string error;
    auto resolved = config::ProviderFactory::create(ctx.rawArgs, ctx.app.appConfig, error);
    if (!resolved) {
        fmt::print("Model degistirilemedi: {}\n", error);
        return CommandResult::ShowError;
    }

    ctx.app.session.setProvider(resolved->provider);
    ctx.app.session.setModel(resolved->model);
    ctx.app.currentModelSpec = ctx.rawArgs;

    fmt::print("Model degistirildi: {}\n", ctx.rawArgs);
    return CommandResult::Handled;
}

}  // namespace aicpp::commands
