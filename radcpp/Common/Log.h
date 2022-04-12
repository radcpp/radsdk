#ifndef RADCPP_LOG_H
#define RADCPP_LOG_H
#pragma once

#include "radcpp/Common/Common.h"
#include "radcpp/Common/String.h"

enum class LogLevel
{
    Undefined,
    Verbose,
    Debug,
    Info,
    Warn,
    Error,
    Critical,
    Count
};

std::string ToString(LogLevel level);

void LogPrint(const char* category, LogLevel level, const char* format, ...);

#endif // RADCPP_LOG_H