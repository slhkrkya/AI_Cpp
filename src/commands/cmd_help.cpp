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

    fmt::print("\nBulut saglayici kullanmak istersen:\n");
    fmt::print("  /model <saglayici>:<model> yaz (orn. openai:gpt-4o-mini, groq:llama-3.3-70b-versatile).\n");
    fmt::print("  Saglayici tanimli degilse (base URL / API anahtari eksikse) uygulama senden ister ve\n");
    fmt::print("  config.json'a kaydeder - yeniden baslatmana gerek kalmaz. OpenAI'in chat-completions\n");
    fmt::print("  API'siyle uyumlu her saglayici icin gecerli (OpenAI, Groq, Mistral, DeepSeek, Together,\n");
    fmt::print("  Fireworks, xAI/Grok, Azure OpenAI, Gemini'nin openai-uyumlu ucu, vb.).\n");
    fmt::print("\nDetaylar icin /config, gecmis oturumlar icin /history, salt-okunur inceleme icin "
               "/plan kullanabilirsin.\n");
    return CommandResult::Handled;
}

}  // namespace aicpp::commands
