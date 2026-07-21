#pragma once

#include <string>
#include <unordered_map>

namespace aicpp::i18n {

// Each returns a function-local static map (not a namespace-scope global) to
// avoid static-initialization-order issues across translation units.
const std::unordered_map<std::string, std::string>& catalog_tr();
const std::unordered_map<std::string, std::string>& catalog_en();

}  // namespace aicpp::i18n
