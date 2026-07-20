#include "commands/cmd_config.h"

#include <fstream>
#include <sstream>

#include <fmt/core.h>

#include "core/config/Paths.h"

namespace aicpp::commands {

CommandResult CmdConfig::execute(CommandContext&) {
    auto path = config::configFilePath();
    fmt::print("Config dosyasi: {}\n\n", path.string());

    std::ifstream in(path);
    if (!in) {
        fmt::print("(dosya okunamadi)\n");
        return CommandResult::ShowError;
    }

    std::ostringstream buffer;
    buffer << in.rdbuf();
    fmt::print("{}\n", buffer.str());
    fmt::print("\nDegistirmek icin dosyayi bir editorde duzenleyip uygulamayi yeniden baslat.\n");
    return CommandResult::Handled;
}

}  // namespace aicpp::commands
