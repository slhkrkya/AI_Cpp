#include "commands/cmd_model.h"

#include <cctype>
#include <iostream>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include "core/config/ProviderFactory.h"

namespace aicpp::commands {

namespace {

bool readYesNo(const std::string& prompt) {
    fmt::print("{}", prompt);
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

    fmt::print("{}\n", priorError);
    fmt::print(
        "'{}' saglayicisini simdi ayarlamak ister misin? Girdiklerin ~/.aicpp/config.json'a\n"
        "kaydedilir, tekrar baslatmana gerek kalmaz. Bu, OpenAI'in chat-completions API'siyle\n"
        "uyumlu HERHANGI bir saglayici icin calisir (OpenAI, Groq, Mistral, DeepSeek, Together,\n"
        "Fireworks, xAI/Grok, Azure OpenAI, Gemini'nin openai-uyumlu ucu, yerel llama.cpp/LM\n"
        "Studio sunucusu, vb.).\n",
        providerId);
    if (!readYesNo("(y/n): ")) return false;

    auto& pc = ctx.app.appConfig.providers[providerId];

    if (pc.base_url.empty() && providerId != "openai") {
        fmt::print("Base URL (orn. https://api.groq.com/openai/v1): ");
        std::string url;
        if (!std::getline(std::cin, url) || url.empty()) {
            fmt::print("URL bos, vazgecildi.\n");
            return false;
        }
        pc.base_url = url;
    }

    if (pc.api_key.empty()) {
        fmt::print("API anahtarini yapistir: ");
        std::string key;
        if (!std::getline(std::cin, key) || key.empty()) {
            fmt::print("Anahtar bos, vazgecildi.\n");
            return false;
        }
        pc.api_key = key;
    }

    if (pc.default_model.empty()) pc.default_model = model;
    ctx.app.appConfig.save();

    fmt::print("[uyari] Anahtar config.json'a duz metin olarak kaydedildi - bu dosyayi paylasma.\n");
    return true;
}

}  // namespace

CommandResult CmdModel::execute(CommandContext& ctx) {
    if (ctx.rawArgs.empty()) {
        fmt::print("Mevcut model: {}\n", ctx.app.currentModelSpec);
        fmt::print("Yapilandirilmis saglayicilar: {}\n",
                    fmt::join(config::ProviderFactory::availableProviderIds(ctx.app.appConfig), ", "));
        fmt::print("Degistirmek icin: /model <saglayici>:<model>  (orn. /model openai:gpt-4o-mini)\n");
        fmt::print("Baska bir saglayici da yazabilirsin (orn. /model groq:llama-3.3-70b-versatile) - "
                    "tanimli degilse sana sorup kaydeder.\n");
        return CommandResult::Handled;
    }

    std::string error;
    auto resolved = config::ProviderFactory::create(ctx.rawArgs, ctx.app.appConfig, error);

    if (!resolved && tryInteractiveProviderSetup(ctx, ctx.rawArgs, error)) {
        resolved = config::ProviderFactory::create(ctx.rawArgs, ctx.app.appConfig, error);
    }

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
