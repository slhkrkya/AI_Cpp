#include "cli/terminal.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <csignal>
#endif

#include "core/net/CancelToken.h"

namespace aicpp::cli {

void enableAnsiSupport() {
#ifdef _WIN32
    // Without this, the multi-byte UTF-8 source literals in the i18n catalogs
    // (Turkish diacritics, box-drawing/bullet glyphs used by cli::theme) print
    // as mojibake or '?' on a stock Windows console - CP_UTF8 must be set on
    // both the output and input code pages before anything is printed/read.
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;

    DWORD mode = 0;
    if (!GetConsoleMode(hOut, &mode)) return;

    SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
}

#ifdef _WIN32

namespace {

BOOL WINAPI ctrlHandler(DWORD ctrlType) {
    if (ctrlType != CTRL_C_EVENT && ctrlType != CTRL_BREAK_EVENT) return FALSE;
    if (net::streamInFlight().load()) {
        net::cancelRequested().store(true);
        return TRUE;  // handled - let the write callback abort the transfer instead of dying
    }
    return FALSE;  // not streaming right now - fall through to the default (terminate)
}

}  // namespace

void installCtrlCHandler() { SetConsoleCtrlHandler(ctrlHandler, TRUE); }

#else

namespace {

void sigintHandler(int signum) {
    if (net::streamInFlight().load()) {
        net::cancelRequested().store(true);
        return;
    }
    // Not streaming right now - restore the default handler and re-raise so
    // the process terminates exactly as it would have without our handler.
    std::signal(SIGINT, SIG_DFL);
    std::raise(signum);
}

}  // namespace

void installCtrlCHandler() { std::signal(SIGINT, sigintHandler); }

#endif

}  // namespace aicpp::cli
