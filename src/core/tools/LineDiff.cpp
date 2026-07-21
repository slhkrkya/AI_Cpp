#include "core/tools/LineDiff.h"

#include <algorithm>

#include <fmt/format.h>

#include "i18n/Translator.h"

namespace aicpp::tools {

using llm::DiffLine;
using llm::DiffLineType;

namespace {

// Upfront bound on N+M so we never even allocate the Myers `V` array for a
// pathologically huge input.
constexpr size_t kMaxTotalLines = 20000;

// Live bound on the edit distance D, checked inside the d-loop. This is the
// bound that actually matters in practice: cost is O((N+M)*D), so a small
// change in a huge file (small D) stays fast regardless of kMaxTotalLines;
// this cap only kicks in for the rare "file was almost entirely rewritten"
// case, bounding worst-case time/memory (trace snapshots) deterministically.
constexpr int kMaxEditDistance = 800;

// Lines of unchanged context kept on each side of a change, matching `git
// diff`'s default.
constexpr int kContextSize = 3;

// Individual lines longer than this are truncated for display (minified/huge
// single-line files) - a rendering guard, unrelated to the diff algorithm's
// own cost.
constexpr size_t kMaxLineLen = 500;

std::vector<std::string> splitLines(const std::string& text) {
    // Truly empty content (e.g. a brand-new file) is zero lines, not one
    // empty line - otherwise a new file's diff would show a spurious leading
    // "- " (removed empty line) before all the real Added lines.
    if (text.empty()) return {};

    std::string_view sv = text;
    if (!sv.empty() && sv.back() == '\n') sv.remove_suffix(1);  // ignore a single trailing newline

    std::vector<std::string> lines;
    size_t pos = 0;
    while (true) {
        size_t nl = sv.find('\n', pos);
        std::string_view line = (nl == std::string_view::npos) ? sv.substr(pos) : sv.substr(pos, nl - pos);
        if (!line.empty() && line.back() == '\r') line.remove_suffix(1);  // treat CRLF == LF for diffing
        if (line.size() > kMaxLineLen) line = line.substr(0, kMaxLineLen);
        lines.emplace_back(line);
        if (nl == std::string_view::npos) break;
        pos = nl + 1;
    }
    return lines;
}

// Reconstructs the edit script from the Myers `trace` snapshots, in
// old-to-new order. `foundD` is the edit distance found by the forward pass.
std::vector<DiffLine> backtrack(const std::vector<std::string>& a, const std::vector<std::string>& b,
                                  const std::vector<std::vector<int>>& trace, int offset, int foundD) {
    std::vector<DiffLine> result;
    int x = static_cast<int>(a.size());
    int y = static_cast<int>(b.size());

    for (int depth = foundD; depth >= 0; --depth) {
        if (depth == 0) {
            // Base case: no synthetic "previous state" exists at depth 0 (the
            // seeded V[offset+1]=0 used to bootstrap the forward pass is NOT
            // a real prior x - reading it here would fabricate a bogus edit,
            // confirmed even on the trivial "identical inputs" case). Just
            // walk whatever pure-diagonal (unchanged) prefix remains down to
            // the true origin.
            while (x > 0 && y > 0 && a[x - 1] == b[y - 1]) {
                result.push_back({DiffLineType::Context, a[x - 1]});
                --x;
                --y;
            }
            break;
        }

        const auto& v = trace[depth];
        int k = x - y;
        int prevK = (k == -depth || (k != depth && v[offset + k - 1] < v[offset + k + 1])) ? k + 1 : k - 1;
        int prevX = v[offset + prevK];
        int prevY = prevX - prevK;

        while (x > prevX && y > prevY) {
            result.push_back({DiffLineType::Context, a[x - 1]});
            --x;
            --y;
        }

        if (x == prevX) {
            result.push_back({DiffLineType::Added, b[y - 1]});
            --y;
        } else {
            result.push_back({DiffLineType::Removed, a[x - 1]});
            --x;
        }

        x = prevX;
        y = prevY;
    }

    std::reverse(result.begin(), result.end());
    return result;
}

std::vector<DiffLine> collapseContext(const std::vector<DiffLine>& raw, int contextSize) {
    std::vector<DiffLine> out;
    size_t i = 0;
    while (i < raw.size()) {
        if (raw[i].type != DiffLineType::Context) {
            out.push_back(raw[i]);
            ++i;
            continue;
        }

        size_t runStart = i;
        while (i < raw.size() && raw[i].type == DiffLineType::Context) ++i;
        size_t runLen = i - runStart;

        bool atStart = (runStart == 0);
        bool atEnd = (i == raw.size());

        if (atStart && atEnd) {
            // No changes anywhere in this diff at all (shouldn't normally
            // happen - callers short-circuit identical content earlier -
            // but keep everything rather than collapsing a diff with no
            // hunks to anchor context against).
            for (size_t j = runStart; j < i; ++j) out.push_back(raw[j]);
        } else if (atStart) {
            size_t keepFrom = (runLen > static_cast<size_t>(contextSize)) ? i - contextSize : runStart;
            if (keepFrom > runStart) {
                out.push_back({DiffLineType::Collapsed,
                                fmt::format(fmt::runtime(i18n::t("diff.unchanged_lines")), keepFrom - runStart)});
            }
            for (size_t j = keepFrom; j < i; ++j) out.push_back(raw[j]);
        } else if (atEnd) {
            size_t keepTo = std::min(runStart + static_cast<size_t>(contextSize), i);
            for (size_t j = runStart; j < keepTo; ++j) out.push_back(raw[j]);
            if (keepTo < i) {
                out.push_back({DiffLineType::Collapsed,
                                fmt::format(fmt::runtime(i18n::t("diff.unchanged_lines")), i - keepTo)});
            }
        } else if (runLen <= static_cast<size_t>(2 * contextSize)) {
            for (size_t j = runStart; j < i; ++j) out.push_back(raw[j]);
        } else {
            for (size_t j = runStart; j < runStart + static_cast<size_t>(contextSize); ++j) out.push_back(raw[j]);
            size_t hidden = runLen - 2 * static_cast<size_t>(contextSize);
            out.push_back(
                {DiffLineType::Collapsed, fmt::format(fmt::runtime(i18n::t("diff.unchanged_lines")), hidden)});
            for (size_t j = i - contextSize; j < i; ++j) out.push_back(raw[j]);
        }
    }
    return out;
}

}  // namespace

std::optional<std::vector<DiffLine>> computeLineDiff(const std::string& oldText, const std::string& newText) {
    if (oldText == newText) return std::vector<DiffLine>{};

    if (oldText.find('\0') != std::string::npos || newText.find('\0') != std::string::npos) {
        return std::nullopt;
    }

    std::vector<std::string> a = splitLines(oldText);
    std::vector<std::string> b = splitLines(newText);
    if (a.empty() && b.empty()) return std::vector<DiffLine>{};  // defensive; splitLines shouldn't produce this

    if (a.size() + b.size() > kMaxTotalLines) return std::nullopt;

    const int n = static_cast<int>(a.size());
    const int m = static_cast<int>(b.size());
    const int maxD = n + m;
    const int offset = maxD;

    std::vector<int> v(2 * maxD + 1, 0);
    std::vector<std::vector<int>> trace;
    trace.reserve(static_cast<size_t>(std::min(maxD, kMaxEditDistance)) + 1);

    bool found = false;
    int foundD = 0;

    for (int d = 0; d <= maxD; ++d) {
        if (d > kMaxEditDistance) return std::nullopt;

        trace.push_back(v);
        for (int k = -d; k <= d; k += 2) {
            int x;
            if (k == -d || (k != d && v[offset + k - 1] < v[offset + k + 1])) {
                x = v[offset + k + 1];
            } else {
                x = v[offset + k - 1] + 1;
            }
            int y = x - k;
            while (x < n && y < m && a[x] == b[y]) {
                ++x;
                ++y;
            }
            v[offset + k] = x;
            if (x >= n && y >= m) {
                found = true;
                foundD = d;
                break;
            }
        }
        if (found) break;
    }

    if (!found) return std::nullopt;  // defensive; the loop covers up to maxD so this shouldn't trigger

    auto raw = backtrack(a, b, trace, offset, foundD);
    return collapseContext(raw, kContextSize);
}

}  // namespace aicpp::tools
