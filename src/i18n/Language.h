#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace aicpp::i18n {

enum class Language { Tr, En };

inline std::optional<Language> parseLanguage(std::string_view code) {
    if (code == "tr") return Language::Tr;
    if (code == "en") return Language::En;
    return std::nullopt;
}

inline std::string languageCode(Language lang) {
    switch (lang) {
        case Language::Tr:
            return "tr";
        case Language::En:
            return "en";
    }
    return "tr";
}

inline std::string languageNativeName(Language lang) {
    switch (lang) {
        case Language::Tr:
            return "Türkçe";
        case Language::En:
            return "English";
    }
    return "?";
}

}  // namespace aicpp::i18n
