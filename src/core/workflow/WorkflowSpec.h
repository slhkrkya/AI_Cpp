#pragma once

#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace aicpp::workflow {

struct SubAgentSpec {
    std::string id;
    std::string systemPrompt;
    std::string promptTemplate;
    std::string modelOverride;  // "<provider>:<model>"; empty = use the caller's default
};

struct StageSpec {
    enum class Mode { Parallel, Sequential };

    std::string id;
    Mode mode = Mode::Parallel;
    std::vector<SubAgentSpec> agents;
    int maxConcurrency = 4;
};

struct SynthesisSpec {
    std::string promptTemplate;
};

struct WorkflowSpec {
    std::string name;
    std::vector<StageSpec> stages;
    SynthesisSpec synthesis;
};

nlohmann::json workflowSpecToJson(const WorkflowSpec& spec);
std::optional<WorkflowSpec> workflowSpecFromJson(const nlohmann::json& j, std::string& error);

}  // namespace aicpp::workflow
