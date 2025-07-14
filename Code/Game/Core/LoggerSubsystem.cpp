#include "LoggerSubsystem.hpp"

#include <cstdarg>

#include "Game/GameCommon.hpp"


LoggerSubsystem::LoggerSubsystem(LoggerConfig& config) : m_config(config)
{
}

void LoggerSubsystem::Startup()
{
    EnableCategory(ELogCategory::LogSystem);
    EnableCategory(ELogCategory::LogActor);
    SetMinVerbosity(ELogVerbosity::Info);
    LOG(LogSystem, Info, "Start up Logger subsystem...");
}

void LoggerSubsystem::Shutdown()
{
}

void LoggerSubsystem::SetCategoryMask(uint32_t mask)
{
    m_categoryMask.store(mask);
}

static const char* BaseName(const char* path) noexcept
{
    const char* s1 = strrchr(path, '/');
    const char* s2 = strrchr(path, '\\');
    const char* s  = s1 ? s1 : s2; // Windows / Unix compatible
    return s ? (s + 1) : path;
}

void LoggerSubsystem::Log(ELogCategory cat, ELogVerbosity v, const char* file, int line, const char* fmt, ...) noexcept
{
    // filter
    if ((m_categoryMask.load() & static_cast<uint32_t>(cat)) == 0) return;
    if (v < m_minVerbosity.load()) return;

    // varargs → string
    char    buffer[2048];
    va_list args;
    va_start(args, fmt);
    _vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, fmt, args);
    va_end(args);

    // Added headers (Category, Verbosity, Thread, FileName:Line)
    const char* catStr[]  = {"Temp", "Render", "Game", "Audio", "Network", "System", "Resource", "Widget", "Actor"};
    const char* verbStr[] = {"INFO", "WARNING", "ERROR"};

    const char* fileName = BaseName(file); // Only File name not the whole path

    std::lock_guard<std::mutex> guard(m_printMutex); // thread safe
    printf("Log%s %s: @(%s:%d) %s\n",
           catStr[static_cast<int>(log2(static_cast<uint32_t>(cat)))], // mapping
           verbStr[static_cast<int>(v)],
           fileName, line,
           buffer);
}
