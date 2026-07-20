#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "core/config/AppConfig.h"
#include "core/llm/ILLMProvider.h"
#include "core/tools/ToolRegistry.h"
#include "core/workflow/WorkflowSpec.h"

namespace aicpp::workflow {

struct SubAgentResult {
    std::string agent_id;
    std::string output_text;
    std::string error;
    bool ok = true;
};

using ProgressCallback = std::function<void(const std::string&)>;

// Executes a WorkflowSpec: each stage fans its sub-agents out (parallel via
// std::async, or sequential), sub-agents run in isolated read-only sessions,
// and results feed {{stageId.agentId}} / {{stageId.*}} template substitution
// for later stages and the final synthesis prompt.
class WorkflowEngine {
public:
    WorkflowEngine(llm::ProviderPtr defaultProvider, std::string defaultModel,
                   tools::ToolRegistry* toolRegistry, const config::AppConfig& appConfig);

    // Runs all stages and returns the RESOLVED synthesis prompt (not executed
    // here) - the caller submits it through its own normally-permissioned,
    // normally-streamed session, exactly like any other turn.
    std::string run(const WorkflowSpec& spec, const std::string& input, const ProgressCallback& progress);

private:
    std::vector<SubAgentResult> runStage(const StageSpec& stage, const std::string& input,
                                          const std::map<std::string, std::vector<SubAgentResult>>& priorResults,
                                          const ProgressCallback& progress);
    SubAgentResult runSubAgent(const SubAgentSpec& agentSpec, const std::string& resolvedPrompt);

    llm::ProviderPtr defaultProvider_;
    std::string defaultModel_;
    tools::ToolRegistry* toolRegistry_;
    const config::AppConfig& appConfig_;
};

}  // namespace aicpp::workflow
