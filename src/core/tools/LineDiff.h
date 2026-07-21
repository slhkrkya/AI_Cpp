#pragma once

#include <optional>
#include <string>
#include <vector>

#include "core/llm/Schema.h"

namespace aicpp::tools {

// Computes a unified-diff-style line-level diff between oldText and newText
// using the Myers O((N+M)D) algorithm, with unchanged runs longer than a few
// lines collapsed to keep output readable for large files.
//
// Returns:
//  - std::nullopt if a diff couldn't/shouldn't be computed (binary content,
//    or the input is too large/too different - see LineDiff.cpp for the
//    exact limits). Callers should show a brief "diff not available" note.
//  - an empty vector if oldText == newText (nothing to show).
//  - a non-empty vector of DiffLine otherwise.
//
// Cost is dominated by the actual edit distance D, not by file size - a
// small change in a huge file stays cheap; the size/distance caps exist only
// to bound the rare "near-total rewrite of a large file" case.
std::optional<std::vector<llm::DiffLine>> computeLineDiff(const std::string& oldText, const std::string& newText);

}  // namespace aicpp::tools
