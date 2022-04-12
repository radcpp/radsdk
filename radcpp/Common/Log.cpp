#include "radcpp/Common/Log.h"
#include "radcpp/Common/File.h"

#include <chrono>
#include <mutex>

File g_logFile;

const char* g_logLevelStrings[UnderlyingCast(LogLevel::Count)] =
{
    "Undefined",
    "Verbose",
    "Debug",
    "Info",
    "Warn",
    "Error",
    "Critical",
};

std::string ToString(LogLevel level)
{
    return g_logLevelStrings[UnderlyingCast(level)];
}

void LogOutput(const char* data, size_t sizeInBytes);

void LogPrint(const char* category, LogLevel level, const char* format, ...)
{
    using Clock = std::chrono::system_clock;
    Clock::time_point timestamp = std::chrono::system_clock::now();

    thread_local std::string message;
    va_list args;
    va_start(args, format);
    StrFormatInPlaceArgList(message, format, args);
    va_end(args);

    long long milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp.time_since_epoch()).count() % 1000;
    std::time_t timepoint = std::chrono::system_clock::to_time_t(timestamp);
    std::tm datetime = {};
    localtime_s(&datetime, &timepoint);

    thread_local std::string buffer;
    int charsPrinted = StrFormatInPlace(buffer, "[%02d:%02d:%02d.%03lld] %s: %s: %s\n",
        datetime.tm_hour, datetime.tm_min, datetime.tm_sec, milliseconds,
        category, g_logLevelStrings[UnderlyingCast(level)], message.data());

    if (charsPrinted > 0)
    {
        LogOutput(buffer.data(), charsPrinted);

        if (level >= LogLevel::Warn)
        {
            g_logFile.Flush();
        }
    }
}

static std::mutex s_outputMutex;

void LogOutput(const char* data, size_t sizeInBytes)
{
    std::lock_guard lockGuard(s_outputMutex);

    if (!g_logFile.IsOpen())
    {
        std::string processName = FileSystem::GetProcessName();
        g_logFile.Open(processName + ".log", FileOpenWrite);
    }

    if (g_logFile.IsOpen())
    {
        g_logFile.Write(data, sizeInBytes);
    }

    fprintf(stderr, "%s", data);
}