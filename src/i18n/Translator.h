#pragma once

#include <string>
#include <string_view>

#include "i18n/Language.h"

namespace aicpp::i18n {

// Initializes the active language. Call once at startup, after
// AppConfig::load() (so the saved language preference is known). In debug
// builds this also runs a one-time catalog self-check (see Translator.cpp).
void init(Language lang);

// Switches the active language immediately. t() always does a live lookup
// (never caches), so this affects every subsequent call across the whole
// process - including ICommand objects that were constructed once at startup
// and live for the process lifetime (their description()/usage() are
// parameterless virtuals with no access to app state, so a global, always-
// live lookup is the only design that can reach them).
void setLanguage(Language lang);

Language currentLanguage();

// Looks up `key` in the active language's catalog and returns the raw fmt
// template - placeholders like {} are NOT substituted here, the caller passes
// them to fmt::print/fmt::format exactly as it would a string literal.
// Fallback chain: active language -> Turkish (the reference/base language
// every key is guaranteed to exist in) -> a visible "[[key]]" sentinel, which
// should be unreachable in practice (see catalog self-check).
std::string t(std::string_view key);

}  // namespace aicpp::i18n
