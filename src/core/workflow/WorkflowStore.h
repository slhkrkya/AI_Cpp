#pragma once

#include <optional>
#include <string>
#include <vector>

#include "core/workflow/WorkflowSpec.h"

namespace aicpp::workflow {

// Lists the base names (without .json) of saved specs under ~/.aicpp/workflows/.
std::vector<std::string> listSavedSpecs();

std::optional<WorkflowSpec> loadSpec(const std::string& name, std::string& error);

// Writes a bundled "code-review-panel" example spec if the workflows dir has
// no specs yet, so /workflow has something runnable out of the box.
void ensureExampleSpec();

}  // namespace aicpp::workflow
