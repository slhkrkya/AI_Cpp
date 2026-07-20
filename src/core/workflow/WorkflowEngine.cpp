#include "core/workflow/WorkflowEngine.h"

#include <algorithm>
#include <chrono>
#include <future>

#include <fmt/format.h>

#include "core/agent/AgentSession.h"
#include "core/config/ProviderFactory.h"
#include "core/permissions/ReadOnlyPermissionManager.h"

namespace aicpp::workflow {

namespace {

std::string replaceAll(std::string s, const std::string& from, const std::string& to) {
    if (from.empty()) return s;
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos) {
        s.replace(pos, from.size(), to);
        pos += to.size();
    }
    return s;
}

std::string applyTemplate(const std::string& tmpl, const std::string& input,
                           const std::map<std::string, std::vector<SubAgentResult>>& results) {
    std::string out = replaceAll(tmpl, "{{input}}", input);
    for (const auto& [stageId, agentResults] : results) {
        std::string joined;
        for (const auto& r : agentResults) {
            std::string text = r.ok ? r.output_text : ("[hata] " + r.error);
            joined += fmt::format("### {}\n{}\n\n", r.agent_id, text);
            out = replaceAll(out, "{{" + stageId + "." + r.agent_id + "}}", text);
        }
        out = replaceAll(out, "{{" + stageId + ".*}}", joined);
    }
    return out;
}

}  // namespace

WorkflowEngine::WorkflowEngine(llm::ProviderPtr defaultProvider, std::string defaultModel,
                               tools::ToolRegistry* toolRegistry, const config::AppConfig& appConfig)
    : defaultProvider_(std::move(defaultProvider)),
      defaultModel_(std::move(defaultModel)),
      toolRegistry_(toolRegistry),
      appConfig_(appConfig) {}

SubAgentResult WorkflowEngine::runSubAgent(const SubAgentSpec& agentSpec, const std::string& resolvedPrompt) {
    SubAgentResult result;
    result.agent_id = agentSpec.id;

    llm::ProviderPtr provider = defaultProvider_;
    std::string model = defaultModel_;
    if (!agentSpec.modelOverride.empty()) {
        std::string err;
        auto resolved = config::ProviderFactory::create(agentSpec.modelOverride, appConfig_, err);
        if (resolved) {
            provider = resolved->provider;
            model = resolved->model;
        }
    }

    permissions::ReadOnlyPermissionManager roPerm;
    std::string systemPrompt =
        agentSpec.systemPrompt.empty()
            ? "You are a focused sub-agent helping with one specific task as part of a larger workflow. Be concise."
            : agentSpec.systemPrompt;

    agent::AgentSession subSession(provider, model, systemPrompt, toolRegistry_, &roPerm);

    std::string accumulated;
    std::string errorText;
    subSession.runTurn(resolvedPrompt, [&](const llm::StreamEvent& ev) {
        if (ev.type == llm::StreamEvent::Type::TextDelta) {
            accumulated += ev.text;
        } else if (ev.type == llm::StreamEvent::Type::Error) {
            errorText += ev.text;
        }
    });

    if (accumulated.empty() && !errorText.empty()) {
        result.ok = false;
        result.error = errorText;
    } else {
        result.output_text = accumulated;
    }
    return result;
}

std::vector<SubAgentResult> WorkflowEngine::runStage(
    const StageSpec& stage, const std::string& input,
    const std::map<std::string, std::vector<SubAgentResult>>& priorResults, const ProgressCallback& progress) {
    std::vector<SubAgentResult> results(stage.agents.size());

    if (stage.mode == StageSpec::Mode::Sequential) {
        for (size_t i = 0; i < stage.agents.size(); ++i) {
            std::string prompt = applyTemplate(stage.agents[i].promptTemplate, input, priorResults);
            progress(fmt::format("[workflow] {}: calisiyor...", stage.agents[i].id));
            auto start = std::chrono::steady_clock::now();
            results[i] = runSubAgent(stage.agents[i], prompt);
            double secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
            progress(fmt::format("[workflow] {}: {} ({:.1f}s)", stage.agents[i].id,
                                  results[i].ok ? "tamamlandi" : "hata", secs));
        }
        return results;
    }

    const size_t maxConcurrency = stage.maxConcurrency > 0 ? static_cast<size_t>(stage.maxConcurrency) : 4;
    size_t idx = 0;
    while (idx < stage.agents.size()) {
        size_t batchEnd = std::min(idx + maxConcurrency, stage.agents.size());

        std::vector<std::future<SubAgentResult>> futures;
        std::vector<std::chrono::steady_clock::time_point> starts;
        for (size_t i = idx; i < batchEnd; ++i) {
            progress(fmt::format("[workflow] {}: calisiyor...", stage.agents[i].id));
            std::string prompt = applyTemplate(stage.agents[i].promptTemplate, input, priorResults);
            starts.push_back(std::chrono::steady_clock::now());
            const SubAgentSpec agentSpec = stage.agents[i];
            futures.push_back(std::async(std::launch::async,
                                          [this, agentSpec, prompt]() { return runSubAgent(agentSpec, prompt); }));
        }
        for (size_t i = idx; i < batchEnd; ++i) {
            results[i] = futures[i - idx].get();
            double secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - starts[i - idx]).count();
            progress(fmt::format("[workflow] {}: {} ({:.1f}s)", stage.agents[i].id,
                                  results[i].ok ? "tamamlandi" : "hata", secs));
        }
        idx = batchEnd;
    }
    return results;
}

std::string WorkflowEngine::run(const WorkflowSpec& spec, const std::string& input, const ProgressCallback& progress) {
    std::map<std::string, std::vector<SubAgentResult>> allResults;
    for (const auto& stage : spec.stages) {
        progress(fmt::format("\n[workflow] asama basliyor: {} ({} agent)", stage.id, stage.agents.size()));
        allResults[stage.id] = runStage(stage, input, allResults, progress);
    }
    return applyTemplate(spec.synthesis.promptTemplate, input, allResults);
}

}  // namespace aicpp::workflow
