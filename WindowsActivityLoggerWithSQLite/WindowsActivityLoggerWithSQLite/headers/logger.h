#pragma once

#include <stdio.h>

#define LOG(...) WriteLog(__VA_ARGS__)



enum LogMode
{
    LOG_NONE,
    LOG_CONSOLE,
    LOG_FILE,
    LOG_BOTH
};

void InitLogger(LogMode mode, const char* fileName = nullptr);
void CloseLogger();
void WriteLog(const wchar_t* fmt, ...);

