#include "core/workflow/WorkflowSpec.h"

namespace aicpp::workflow {

namespace {

std::string modeToString(StageSpec::Mode m) { return m == StageSpec::Mode::Parallel ? "parallel" : "sequential"; }

StageSpec::Mode modeFromString(const std::string& s) {
    return s == "sequential" ? StageSpec::Mode::Sequential : StageSpec::Mode::Parallel;
}

}  // namespace

nlohmann::json workflowSpecToJson(const WorkflowSpec& spec) {
    nlohmann::json stages = nlohmann::json::array();
    for (const auto& stage : spec.stages) {
        nlohmann::json agents = nlohmann::json::array();
        for (const auto& a : stage.agents) {
            agents.push_back({{"id", a.id},
                               {"systemPrompt", a.systemPrompt},
                               {"promptTemplate", a.promptTemplate},
                               {"modelOverride", a.modelOverride}});
        }
        stages.push_back({{"id", stage.id},
                           {"mode", modeToString(stage.mode)},
                           {"maxConcurrency", stage.maxConcurrency},
                           {"agents", agents}});
    }
    return {{"name", spec.name}, {"stages", stages}, {"synthesis", {{"promptTemplate", spec.synthesis.promptTemplate}}}};
}

std::optional<WorkflowSpec> workflowSpecFromJson(const nlohmann::json& j, std::string& error) {
    if (!j.is_object() || !j.contains("stages") || !j["stages"].is_array() || j["stages"].empty()) {
        error = "Spec bir 'stages' dizisi icermeli ve bos olmamali.";
        return std::nullopt;
    }

    WorkflowSpec spec;
    spec.name = j.value("name", std::string("adhoc"));

    for (const auto& stageJson : j["stages"]) {
        if (!stageJson.contains("agents") || !stageJson["agents"].is_array() || stageJson["agents"].empty()) {
            error = "Her stage en az bir agent icermeli.";
            return std::nullopt;
        }

        StageSpec stage;
        stage.id = stageJson.value("id", std::string("stage"));
        stage.mode = modeFromString(stageJson.value("mode", std::string("parallel")));
        stage.maxConcurrency = stageJson.value("maxConcurrency", 4);

        for (const auto& agentJson : stageJson["agents"]) {
            if (!agentJson.contains("id") || !agentJson.contains("promptTemplate")) {
                error = "Her agent 'id' ve 'promptTemplate' icermeli.";
                return std::nullopt;
            }
            SubAgentSpec agent;
            agent.id = agentJson.value("id", std::string());
            agent.systemPrompt = agentJson.value("systemPrompt", std::string());
            agent.promptTemplate = agentJson.value("promptTemplate", std::string());
            agent.modelOverride = agentJson.value("modelOverride", std::string());
            stage.agents.push_back(std::move(agent));
        }

        spec.stages.push_back(std::move(stage));
    }

    if (j.contains("synthesis") && j["synthesis"].is_object()) {
        spec.synthesis.promptTemplate = j["synthesis"].value("promptTemplate", std::string());
    }
    if (spec.synthesis.promptTemplate.empty()) {
        spec.synthesis.promptTemplate = "Combine the sub-agent results below into one clear, prioritized answer "
                                         "for the user's original request: \"{{input}}\"\n\n{{" +
                                         spec.stages.back().id + ".*}}";
    }

    return spec;
}

}  // namespace aicpp::workflow
