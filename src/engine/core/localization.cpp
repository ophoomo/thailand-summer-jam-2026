
#include "core/localization.h"
#include "utils/logger.h"

namespace {

// Recursively walk JSON and populate flat dot-notation map.
// { "menu": { "start": "Start" } }  →  "menu.start" = "Start"
void flatten(const nlohmann::json &j, const std::string &prefix,
             std::unordered_map<std::string, std::string> &out)
{
    for (const auto &[rawKey, value] : j.items()) {
        std::string fullKey = prefix.empty() ? rawKey : prefix + '.' + rawKey;
        if (value.is_object()) {
            flatten(value, fullKey, out);
        } else if (value.is_string()) {
            out[fullKey] = value.get<std::string>();
        }
    }
}

} // namespace

// ============================================================
// Static member definitions
// ============================================================

Language Localization::s_language = Language::English;
std::string Localization::s_langDir;
Localization::StringMap Localization::s_maps[2];
bool Localization::s_loaded[2] = {false, false};
std::string Localization::s_missing;

// ============================================================
// Public Methods
// ============================================================

void Localization::load(Language lang, nlohmann::json j)
{
    const int idx = static_cast<int>(lang);
    if (s_loaded[idx])
        return;

    flatten(j, "", s_maps[idx]);
    s_loaded[idx] = true;

    LOG_CORE_TRACE("[Localization] Loaded {} keys, {} ",
                   lang == Language::Thai ? "Thai" : "English",
                   s_maps[static_cast<int>(lang)].size());
}

void Localization::setLanguage(Language lang)
{
    if (s_language == lang)
        return;
    s_language = lang;
    LOG_CORE_TRACE("[Localization] Language switched to {}",
                   lang == Language::Thai ? "Thai" : "English");
}

const std::string &Localization::get(const std::string &key)
{
    const int idx = static_cast<int>(s_language);
    const int fallback = static_cast<int>(Language::English);

    auto it = s_maps[idx].find(key);
    if (it != s_maps[idx].end())
        return it->second;

    // Fall back to English.
    auto fb = s_maps[fallback].find(key);
    if (fb != s_maps[fallback].end()) {
        LOG_CORE_WARN("[Localization] Key '{}' missing in active language, using fallback", key);
        return fb->second;
    }

    // Key is entirely missing — return key verbatim so the bug is visible.
    LOG_CORE_WARN("[Localization] Key '{}' not found in any language", key);
    s_missing = key;
    return s_missing;
}

const std::string &Localization::get(const char *key)
{
    return get(std::string(key));
}
