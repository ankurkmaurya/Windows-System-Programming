
#include "logger.h"

#include <windows.h>
#include <direct.h>
#include <stdarg.h>
#include <time.h>
#include <share.h>
#include <pathcch.h>
#include <string>
#include <utility.h>


static LogMode g_logMode = LOG_CONSOLE;
static FILE* g_logFile = nullptr;
static int g_currentDay = -1;


static void OpenTodaysLog()
{
    SYSTEMTIME st;
    GetLocalTime(&st);

    if (g_logFile && g_currentDay == st.wDay)
        return;

    if (g_logFile)
    {
        fclose(g_logFile);
        g_logFile = nullptr;
    }


    wchar_t dbFilePathW[MAX_PATH];
    const wchar_t* logFileNamePattern = L"application_activity_%04d-%02d-%02d.log";
    const wchar_t* logsFolderName = L"logs";

    GetModuleFileNameW(NULL, dbFilePathW, MAX_PATH);
    PathCchRemoveFileSpec(dbFilePathW, MAX_PATH);
    PathCchAppend(dbFilePathW, MAX_PATH, logsFolderName);

    // Convert and extract the const char*
    std::string logsFolderPath = Utility::ConvertPwstrToUtf8(dbFilePathW);
    int dir_created = _mkdir(logsFolderPath.c_str());

    PathCchAppend(dbFilePathW, MAX_PATH, L"\\");
    PathCchAppend(dbFilePathW, MAX_PATH, logFileNamePattern);

    wchar_t fileName[MAX_PATH];
    swprintf(fileName,
        _countof(fileName),
        dbFilePathW,
        st.wYear,
        st.wMonth,
        st.wDay);

    //_wfopen_s(&g_logFile, fileName, L"a+, ccs=UTF-8");

    g_logFile = _wfsopen(
        fileName,
        L"a+, ccs=UTF-8",
        _SH_DENYWR);      // Others can read, but not write


    g_currentDay = st.wDay;
}


void InitLogger(LogMode mode, const char* fileName)
{
    g_logMode = mode;

    if (mode == LOG_FILE || mode == LOG_BOTH) {
        OpenTodaysLog();
    }
       
}


void CloseLogger()
{
    if (g_logFile)
    {
        fclose(g_logFile);
        g_logFile = nullptr;
    }
}


void WriteLog(const char* fmt, ...)
{
    va_list args;

    // Console
    if (g_logMode == LOG_CONSOLE || g_logMode == LOG_BOTH)
    {
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }

    // File
    if ((g_logMode == LOG_FILE || g_logMode == LOG_BOTH) &&
        g_logFile != nullptr)
    {
        va_start(args, fmt);
        vfprintf(g_logFile, fmt, args);
        fflush(g_logFile);
        va_end(args);
    }
}

void WriteLog(const wchar_t* fmt, ...)
{
    if (g_logMode == LOG_NONE)
        return;

    if (g_logMode == LOG_FILE || g_logMode == LOG_BOTH)
        OpenTodaysLog();

    wchar_t buffer[4096];

    va_list args;
    va_start(args, fmt);
    vswprintf(buffer, _countof(buffer), fmt, args);
    va_end(args);

    if (g_logMode == LOG_CONSOLE || g_logMode == LOG_BOTH)
    {
        wprintf(L"%ls", buffer);
    }

    if ((g_logMode == LOG_FILE || g_logMode == LOG_BOTH) &&
        g_logFile)
    {
        fwprintf(g_logFile, L"%ls", buffer);
        fflush(g_logFile);
    }
}