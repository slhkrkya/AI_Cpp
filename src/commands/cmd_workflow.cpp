#include "commands/cmd_workflow.h"

#include <algorithm>
#include <optional>

#include <fmt/core.h>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "cli/theme.h"
#include "core/agent/AgentSession.h"
#include "core/permissions/ReadOnlyPermissionManager.h"
#include "core/workflow/WorkflowEngine.h"
#include "core/workflow/WorkflowStore.h"

namespace aicpp::commands {

namespace {

// LLM-facing instruction, not user-facing UI text - deliberately left in
// English regardless of the interface language (see i18n scope note).
constexpr const char* kMetaSystemPrompt =
    "You decompose a task into a small JSON workflow spec for parallel sub-agent investigation. "
    "Output ONLY a single JSON object (no prose, no markdown fences) matching exactly this shape: "
    "{\"name\": string, \"stages\": [{\"id\": string, \"mode\": \"parallel\", \"agents\": "
    "[{\"id\": string, \"systemPrompt\": string, \"promptTemplate\": string (must contain "
    "the literal text {{input}})}]}], \"synthesis\": {\"promptTemplate\": string (should "
    "reference {{<stageId>.*}})}}. Use 2 to 5 agents in a single stage, each investigating a "
    "distinct angle of the task.";

std::optional<workflow::WorkflowSpec> decomposeAdHoc(CommandContext& ctx, const std::string& description,
                                                       std::string& error) {
    permissions::ReadOnlyPermissionManager roPerm;
    agent::AgentSession metaSession(ctx.app.session.provider(), ctx.app.session.model(), kMetaSystemPrompt,
                                     nullptr, &roPerm);

    std::string raw;
    metaSession.runTurn(fmt::format("Task: {}", description), [&](const llm::StreamEvent& ev) {
        if (ev.type == llm::StreamEvent::Type::TextDelta) raw += ev.text;
    });

    size_t start = raw.find('{');
    size_t end = raw.rfind('}');
    if (start == std::string::npos || end == std::string::npos || end <= start) {
        error = i18n::t("workflow.meta.no_json");
        return std::nullopt;
    }
    std::string jsonText = raw.substr(start, end - start + 1);

    nlohmann::json j;
    try {
        j = nlohmann::json::parse(jsonText);
    } catch (const nlohmann::json::parse_error& e) {
        error = fmt::format(fmt::runtime(i18n::t("workflow.meta.parse_failed")), e.what());
        return std::nullopt;
    }

    auto spec = workflow::workflowSpecFromJson(j, error);
    if (spec) {
        size_t total = 0;
        for (const auto& s : spec->stages) total += s.agents.size();
        if (total > 5) {
            error = i18n::t("workflow.meta.too_many_agents");
            return std::nullopt;
        }
    }
    return spec;
}

}  // namespace

CommandResult CmdWorkflow::execute(CommandContext& ctx) {
    if (ctx.rawArgs.empty() || ctx.rawArgs == "list") {
        auto names = workflow::listSavedSpecs();
        if (names.empty()) {
            cli::theme::info(i18n::t("workflow.none_saved"));
        } else {
            cli::theme::sectionHeader(i18n::t("workflow.saved_header"));
            for (const auto& n : names) fmt::print("  {}\n", n);
        }
        fmt::print("\n{}\n", i18n::t("workflow.usage_hint"));
        return CommandResult::Handled;
    }

    auto savedNames = workflow::listSavedSpecs();
    std::string firstToken = ctx.args.empty() ? std::string() : ctx.args[0];
    bool matchesSaved = std::find(savedNames.begin(), savedNames.end(), firstToken) != savedNames.end();

    std::optional<workflow::WorkflowSpec> spec;
    std::string input;
    std::string error;

    if (matchesSaved) {
        spec = workflow::loadSpec(firstToken, error);
        size_t pos = ctx.rawArgs.find(firstToken);
        input = ctx.rawArgs.substr(pos + firstToken.size());
        size_t s = input.find_first_not_of(' ');
        input = (s == std::string::npos) ? "" : input.substr(s);
        if (input.empty()) input = ctx.rawArgs;
    } else {
        fmt::print("\n");
        cli::theme::info(fmt::format("[workflow] {}", i18n::t("workflow.decomposing")));
        input = ctx.rawArgs;
        spec = decomposeAdHoc(ctx, input, error);
    }

    if (!spec) {
        cli::theme::error(fmt::format(fmt::runtime(i18n::t("workflow.run_failed")), error));
        return CommandResult::ShowError;
    }

    workflow::WorkflowEngine engine(ctx.app.session.provider(), ctx.app.session.model(), &ctx.app.toolRegistry,
                                     ctx.app.appConfig);
    std::string synthesisPrompt =
        engine.run(*spec, input, [](const std::string& line) { fmt::print("{}\n", line); });

    fmt::print("\n");
    cli::theme::info(fmt::format("[workflow] {}", i18n::t("workflow.synthesizing")));
    fmt::print("\n");
    ctx.app.pendingFollowUp = synthesisPrompt;
    return CommandResult::Handled;
}

}  // namespace aicpp::commands
