#include "cli/line_editor.h"

#include <cerrno>

namespace aicpp::cli {

LineEditor::LineEditor() { rx_.set_max_history_size(1000); }

std::optional<std::string> LineEditor::readLine(const std::string& prompt) {
    char const* line = nullptr;
    do {
        line = rx_.input(prompt.c_str());
    } while (line == nullptr && errno == EAGAIN);

    if (line == nullptr) {
        return std::nullopt;
    }

    std::string result(line);
    if (!result.empty()) {
        rx_.history_add(result);
    }
    return result;
}

}  // namespace aicpp::cli
