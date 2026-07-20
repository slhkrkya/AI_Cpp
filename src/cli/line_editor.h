#pragma once

#include <optional>
#include <string>

#include <replxx.hxx>

namespace aicpp::cli {

// Thin wrapper around replxx: line editing, history, UTF-8 input.
class LineEditor {
public:
    LineEditor();

    // Returns std::nullopt on EOF (Ctrl+D / Ctrl+Z).
    std::optional<std::string> readLine(const std::string& prompt);

private:
    replxx::Replxx rx_;
};

}  // namespace aicpp::cli
