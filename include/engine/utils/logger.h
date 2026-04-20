#ifndef D746A849_208A_4B6D_B027_ED383528E70F
#define D746A849_208A_4B6D_B027_ED383528E70F

#include <memory>
#include <spdlog/spdlog.h>

class Logger
{
  public:
    static void init();

    static void shutdown();

    static std::shared_ptr<spdlog::logger> &core();

    static std::shared_ptr<spdlog::logger> &client();

  private:
    static std::shared_ptr<spdlog::logger> s_coreLogger;
    static std::shared_ptr<spdlog::logger> s_clientLogger;
};

#define LOG_CORE_TRACE(...) ::Logger::core()->trace(__VA_ARGS__)
#define LOG_CORE_INFO(...) ::Logger::core()->info(__VA_ARGS__)
#define LOG_CORE_WARN(...) ::Logger::core()->warn(__VA_ARGS__)
#define LOG_CORE_ERROR(...) ::Logger::core()->error(__VA_ARGS__)
#define LOG_CORE_FATAL(...) ::Logger::core()->critical(__VA_ARGS__)

#define LOG_TRACE(...) ::Logger::client()->trace(__VA_ARGS__)
#define LOG_INFO(...) ::Logger::client()->info(__VA_ARGS__)
#define LOG_WARN(...) ::Logger::client()->warn(__VA_ARGS__)
#define LOG_ERROR(...) ::Logger::client()->error(__VA_ARGS__)
#define LOG_FATAL(...) ::Logger::client()->critical(__VA_ARGS__)

#endif /* D746A849_208A_4B6D_B027_ED383528E70F */
