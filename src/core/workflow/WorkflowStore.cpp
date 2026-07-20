#include "core/workflow/WorkflowStore.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include <fmt/format.h>

#include "core/config/Paths.h"

namespace aicpp::workflow {

std::vector<std::string> listSavedSpecs() {
    std::vector<std::string> names;
    std::error_code ec;
    auto dir = config::workflowsDir();
    if (!std::filesystem::exists(dir, ec)) return names;

    for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
        if (entry.path().extension() == ".json") {
            names.push_back(entry.path().stem().string());
        }
    }
    return names;
}

std::optional<WorkflowSpec> loadSpec(const std::string& name, std::string& error) {
    auto path = config::workflowsDir() / (name + ".json");
    std::ifstream in(path);
    if (!in) {
        error = fmt::format("'{}' bulunamadi.", name);
        return std::nullopt;
    }

    nlohmann::json j;
    try {
        in >> j;
    } catch (const nlohmann::json::parse_error& e) {
        error = fmt::format("Gecersiz JSON: {}", e.what());
        return std::nullopt;
    }

    return workflowSpecFromJson(j, error);
}

void ensureExampleSpec() {
    auto dir = config::workflowsDir();
    auto path = dir / "code-review-panel.json";

    std::error_code ec;
    if (std::filesystem::exists(path, ec)) return;

    nlohmann::json spec = {
        {"name", "code-review-panel"},
        {"stages",
         nlohmann::json::array(
             {{{"id", "review"},
               {"mode", "parallel"},
               {"agents",
                nlohmann::json::array(
                    {{{"id", "security"},
                      {"systemPrompt", "You are a security-focused code reviewer."},
                      {"promptTemplate", "Review the following for security issues. Be specific and concise:\n\n{{input}}"}},
                     {{"id", "performance"},
                      {"systemPrompt", "You are a performance-focused code reviewer."},
                      {"promptTemplate", "Review the following for performance issues. Be specific and concise:\n\n{{input}}"}},
                     {{"id", "style"},
                      {"systemPrompt", "You are a code-style and readability reviewer."},
                      {"promptTemplate", "Review the following for style/readability issues. Be specific and concise:\n\n{{input}}"}}})}}})},
        {"synthesis",
         {{"promptTemplate",
           "Combine the security/performance/style reviews below into one prioritized list of "
           "concrete action items for this input:\n\n\"{{input}}\"\n\n{{review.*}}"}}},
    };

    std::ofstream out(path);
    if (out) out << spec.dump(2);
}

}  // namespace aicpp::workflow
