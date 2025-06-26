#pragma once
#include <atomic>
#include <cstdint>
#include <mutex>
#define INTERNAL_LOG_CATEGORY(cat)   ELogCategory::cat
#define INTERNAL_LOG_VERBOSITY(verb) ELogVerbosity::verb

#define LOG(Category, Verbosity, Format, ...)                                   \
g_theLoggerSubsystem->Log(                                                        \
INTERNAL_LOG_CATEGORY(Category),                                        \
INTERNAL_LOG_VERBOSITY(Verbosity),                                      \
__FILE__,                                                               \
__LINE__,                                                               \
Format,                                                                 \
##__VA_ARGS__)                                                          \
/**/

struct LoggerConfig
{
};

enum class ELogCategory : uint32_t
{
    LogTemp = 1 << 0,
    LogRender = 1 << 1,
    LogGame = 1 << 2,
    LogAudio = 1 << 3,
    LogNetwork = 1 << 4,
    LogSystem = 1 << 5,
    LogResource = 1 << 6,
    LogWidget = 1 << 7,
    LogActor = 1 << 8,
    // ...
};

enum class ELogVerbosity : uint8_t
{
    Info = 0,
    Warning = 1,
    Error = 2,
};

class LoggerSubsystem
{
public:
    LoggerSubsystem()                       = delete;
    LoggerSubsystem(const LoggerSubsystem&) = delete;
    LoggerSubsystem(LoggerConfig& config);

    void Startup();
    void Shutdown();
    void SetCategoryMask(uint32_t mask); // On/Off category
    void EnableCategory(ELogCategory c) { m_categoryMask.fetch_or(static_cast<uint32_t>(c)); }
    void DisableCategory(ELogCategory c) { m_categoryMask.fetch_and(~static_cast<uint32_t>(c)); }

    void SetMinVerbosity(ELogVerbosity v) { m_minVerbosity.store(v); }

    void Log(ELogCategory cat, ELogVerbosity v,
             const char*  file, int          line,
             const char*  fmt, ...) noexcept;

private:
    LoggerConfig& m_config;

    std::atomic<uint32_t>      m_categoryMask{static_cast<uint32_t>(ELogCategory::LogTemp)}; // By default, only LogTemp is enabled
    std::atomic<ELogVerbosity> m_minVerbosity{ELogVerbosity::Info};
    std::mutex                 m_printMutex; // Simple thread safety; ring buffer can be used for high-frequency logs
};
