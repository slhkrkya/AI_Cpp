#include "commands/cmd_help.h"

#include <fmt/core.h>

#include "commands/command_registry.h"

namespace aicpp::commands {

CommandResult CmdHelp::execute(CommandContext& ctx) {
    fmt::print("aicpp - terminal AI kodlama asistani\n\n");
    fmt::print("Komutlar:\n");
    for (auto* cmd : ctx.app.registry.all()) {
        fmt::print("  {:<12} {}\n", cmd->usage(), cmd->description());
    }

    std::string envName = "OPENAI_API_KEY";
    auto it = ctx.app.appConfig.providers.find("openai");
    if (it != ctx.app.appConfig.providers.end() && !it->second.api_key_env.empty()) {
        envName = it->second.api_key_env;
    }

    fmt::print("\nBulut saglayici kullanmak istersen (orn. OpenAI):\n");
    fmt::print("  1) PowerShell'de:  $env:{} = \"...anahtarin...\"\n", envName);
    fmt::print("  2) aicpp'yi kapatip yeniden baslat (/exit, sonra .\\build\\aicpp.exe)\n");
    fmt::print("  3) /model openai:gpt-4o-mini yaz\n");
    fmt::print("  Kalici olmasi icin: setx {} \"...anahtarin...\"  (sonra yeni bir terminal ac)\n", envName);
    fmt::print("\nDetaylar icin /config, gecmis oturumlar icin /history, salt-okunur inceleme icin "
               "/plan kullanabilirsin.\n");
    return CommandResult::Handled;
}

}  // namespace aicpp::commands
