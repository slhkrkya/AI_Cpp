#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/llm/Schema.h"
#include "core/tools/ITool.h"

namespace aicpp::tools {

class ToolRegistry {
public:
    void registerTool(std::shared_ptr<ITool> tool);

    ITool* find(const std::string& name) const;
    std::vector<ITool*> all() const;
    std::vector<llm::ToolDefinition> allDefinitions() const;

private:
    std::vector<std::shared_ptr<ITool>> tools_;
    std::unordered_map<std::string, ITool*> byName_;
};

}  // namespace aicpp::tools
