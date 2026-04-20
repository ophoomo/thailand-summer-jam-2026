#ifndef E1913A36_8EB4_41B6_8D43_B53F6CD6CB39
#define E1913A36_8EB4_41B6_8D43_B53F6CD6CB39

#include <cstdint>
#include <format>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

enum class Language : uint8_t { English = 0, Thai = 1 };

class Localization
{
  public:
    ~Localization() = delete;

    static void load(Language lang, nlohmann::json j);

    static void setLanguage(Language lang);

    [[nodiscard]] static Language getLanguage()
    {
        return s_language;
    }
    [[nodiscard]] static bool isThai()
    {
        return s_language == Language::Thai;
    }

    [[nodiscard]] static const std::string &get(const std::string &key);
    [[nodiscard]] static const std::string &get(const char *key);

    template <typename... Args>
    [[nodiscard]] static std::string fmt(const std::string &key, Args &&...args)
    {
        return std::vformat(get(key), std::make_format_args(args...));
    }

    template <typename... Args>
    [[nodiscard]] static std::string fmt(const char *key, Args &&...args)
    {
        return fmt(std::string(key), std::forward<Args>(args)...);
    }

  private:
    using StringMap = std::unordered_map<std::string, std::string>;

    static Language s_language;
    static std::string s_langDir;
    static StringMap s_maps[2];
    static bool s_loaded[2];
    static std::string s_missing;
};

static void setLanguage(Language lang);

#endif /* E1913A36_8EB4_41B6_8D43_B53F6CD6CB39 */
