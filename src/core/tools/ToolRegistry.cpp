#include "core/tools/ToolRegistry.h"

namespace aicpp::tools {

void ToolRegistry::registerTool(std::shared_ptr<ITool> tool) {
    byName_[tool->name()] = tool.get();
    tools_.push_back(std::move(tool));
}

ITool* ToolRegistry::find(const std::string& name) const {
    auto it = byName_.find(name);
    return it == byName_.end() ? nullptr : it->second;
}

std::vector<ITool*> ToolRegistry::all() const {
    std::vector<ITool*> result;
    result.reserve(tools_.size());
    for (const auto& t : tools_) result.push_back(t.get());
    return result;
}

std::vector<llm::ToolDefinition> ToolRegistry::allDefinitions() const {
    std::vector<llm::ToolDefinition> defs;
    defs.reserve(tools_.size());
    for (const auto& t : tools_) {
        defs.push_back({t->name(), t->description(), t->parametersSchema()});
    }
    return defs;
}

}  // namespace aicpp::tools
