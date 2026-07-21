#include "cli/theme.h"

#include <cstdlib>

#ifdef _WIN32
#include <io.h>
#define AICPP_ISATTY _isatty
#define AICPP_FILENO _fileno
#else
#include <unistd.h>
#define AICPP_ISATTY isatty
#define AICPP_FILENO fileno
#endif

#include <fmt/format.h>

#include "i18n/Translator.h"

namespace aicpp::cli::theme {

namespace {

constexpr std::string_view kReset = "\x1b[0m";
constexpr std::string_view kBold = "\x1b[1m";
constexpr std::string_view kDim = "\x1b[2m";
constexpr std::string_view kRed = "\x1b[31m";
constexpr std::string_view kGreen = "\x1b[32m";
constexpr std::string_view kYellow = "\x1b[33m";
constexpr std::string_view kCyan = "\x1b[36m";
constexpr std::string_view kBoldRed = "\x1b[1;31m";
constexpr std::string_view kBoldYellow = "\x1b[1;33m";
constexpr std::string_view kBoldCyan = "\x1b[1;36m";
constexpr std::string_view kBoldGreen = "\x1b[1;32m";

bool g_noColor = false;

}  // namespace

void init() { g_noColor = std::getenv("NO_COLOR") != nullptr; }

bool richOutputEnabled(std::FILE* stream) {
    if (g_noColor) return false;
    return AICPP_ISATTY(AICPP_FILENO(stream)) != 0;
}

void info(std::string_view text) {
    if (richOutputEnabled(stdout)) {
        fmt::print("{}{}{}\n", kCyan, text, kReset);
    } else {
        fmt::print("{}\n", text);
    }
}

void success(std::string_view text) {
    if (richOutputEnabled(stdout)) {
        fmt::print("{}{}{}\n", kGreen, text, kReset);
    } else {
        fmt::print("{}\n", text);
    }
}

void warn(std::string_view text) {
    if (richOutputEnabled(stdout)) {
        fmt::print("{}{}{}\n", kYellow, text, kReset);
    } else {
        fmt::print("{}\n", text);
    }
}

void error(std::string_view text) {
    if (richOutputEnabled(stderr)) {
        fmt::print(stderr, "{}{}{}\n", kBoldRed, text, kReset);
    } else {
        fmt::print(stderr, "{}\n", text);
    }
}

void sectionHeader(std::string_view text) {
    if (richOutputEnabled(stdout)) {
        fmt::print("{}{}● {}{}\n", kBold, kCyan, text, kReset);
    } else {
        fmt::print("* {}\n", text);
    }
}

void planBanner(std::string_view body) {
    if (richOutputEnabled(stdout)) {
        fmt::print("\n{}● {}{}\n\n", kBoldYellow, body, kReset);
    } else {
        fmt::print("\n* {}\n\n", body);
    }
}

void permissionPrompt(std::string_view header, std::string_view summary) {
    if (richOutputEnabled(stdout)) {
        fmt::print("\n{}⚠ {}{}\n  {}\n", kBoldYellow, header, kReset, summary);
    } else {
        fmt::print("\n[!] {}\n  {}\n", header, summary);
    }
}

std::string coloredPrompt(std::string_view plainPrompt) {
    if (!richOutputEnabled(stdout)) return std::string(plainPrompt);
    return fmt::format("{}{}{}", kBoldGreen, plainPrompt, kReset);
}

std::string promptText(std::string_view text) {
    if (!richOutputEnabled(stdout)) return std::string(text);
    return fmt::format("{}{}{}", kBold, text, kReset);
}

void toolBanner(std::string_view toolName, ToolPhase phase) {
    std::string label = phase == ToolPhase::Start ? fmt::format(fmt::runtime(i18n::t("tool.start")), toolName)
                                                    : fmt::format(fmt::runtime(i18n::t("tool.end")), toolName);
    const char* glyph = phase == ToolPhase::Start ? "▶" : "✓";
    const char* leading = phase == ToolPhase::Start ? "\n" : "";

    if (richOutputEnabled(stdout)) {
        fmt::print("{}{}{} {}{}\n", leading, kDim, glyph, label, kReset);
    } else {
        fmt::print("{}[{}]\n", leading, label);
    }
}

void beginAssistantTurn() {
    if (richOutputEnabled(stdout)) {
        fmt::print("{}● {}{}\n{}", kBoldCyan, i18n::t("role.assistant"), kReset, kCyan);
    } else {
        fmt::print("{}\n", i18n::t("role.assistant"));
    }
}

void resumeAssistantColor() {
    if (richOutputEnabled(stdout)) fmt::print("{}", kCyan);
}

void pauseAssistantColor() {
    if (richOutputEnabled(stdout)) fmt::print("{}", kReset);
}

void clearScreen() {
    if (richOutputEnabled(stdout)) fmt::print("\x1b[2J\x1b[H");
}

void diffBlock(const std::vector<llm::DiffLine>& lines) {
    if (lines.empty()) return;

    bool rich = richOutputEnabled(stdout);
    std::string out;
    for (const auto& line : lines) {
        switch (line.type) {
            case llm::DiffLineType::Added:
                out += rich ? fmt::format("{}+ {}{}\n", kGreen, line.text, kReset) : fmt::format("+ {}\n", line.text);
                break;
            case llm::DiffLineType::Removed:
                out += rich ? fmt::format("{}- {}{}\n", kRed, line.text, kReset) : fmt::format("- {}\n", line.text);
                break;
            case llm::DiffLineType::Collapsed:
                out += rich ? fmt::format("{}{}{}\n", kDim, line.text, kReset) : fmt::format("{}\n", line.text);
                break;
            case llm::DiffLineType::Context:
            default:
                out += fmt::format("  {}\n", line.text);
                break;
        }
    }
    fmt::print("{}", out);
}

}  // namespace aicpp::cli::theme
