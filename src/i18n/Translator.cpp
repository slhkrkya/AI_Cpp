#include "i18n/Translator.h"

#include <atomic>

#include "i18n/catalogs.h"

#ifndef NDEBUG
#include <cstdio>
#include <cstdlib>
#endif

namespace aicpp::i18n {

namespace {

std::atomic<Language> g_currentLanguage{Language::Tr};

#ifndef NDEBUG
int countPlaceholders(const std::string& s) {
    int count = 0;
    for (size_t i = 0; i + 1 < s.size(); ++i) {
        if (s[i] == '{' && s[i + 1] == '}') ++count;
    }
    return count;
}

// Debug-only: catches a mismatched TR/EN catalog (missing key or a differing
// {}-placeholder count) at startup instead of a user hitting fmt::format_error
// or a silent "[[key]]" sentinel at runtime.
void validateCatalogs() {
    const auto& tr = catalog_tr();
    const auto& en = catalog_en();

    for (const auto& [key, trValue] : tr) {
        auto it = en.find(key);
        if (it == en.end()) {
            std::fprintf(stderr, "[i18n] missing key in EN catalog: %s\n", key.c_str());
            std::abort();
        }
        if (countPlaceholders(trValue) != countPlaceholders(it->second)) {
            std::fprintf(stderr, "[i18n] placeholder count mismatch for key '%s' (tr=%d, en=%d)\n", key.c_str(),
                         countPlaceholders(trValue), countPlaceholders(it->second));
            std::abort();
        }
    }
    for (const auto& [key, enValue] : en) {
        (void)enValue;
        if (tr.find(key) == tr.end()) {
            std::fprintf(stderr, "[i18n] missing key in TR catalog: %s\n", key.c_str());
            std::abort();
        }
    }
}
#endif

}  // namespace

void init(Language lang) {
#ifndef NDEBUG
    validateCatalogs();
#endif
    g_currentLanguage.store(lang);
}

void setLanguage(Language lang) { g_currentLanguage.store(lang); }

Language currentLanguage() { return g_currentLanguage.load(); }

std::string t(std::string_view key) {
    std::string keyStr(key);

    const auto& active = currentLanguage() == Language::En ? catalog_en() : catalog_tr();
    if (auto it = active.find(keyStr); it != active.end()) return it->second;

    const auto& base = catalog_tr();
    if (auto it = base.find(keyStr); it != base.end()) return it->second;

    return "[[" + keyStr + "]]";
}

}  // namespace aicpp::i18n
