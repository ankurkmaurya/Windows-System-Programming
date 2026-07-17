
// WindowsActivityLoggerWithSQLite.cpp : This file contains the 'main' function. Program execution begins and ends there.


#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <sqlite3.h>
#include <pathcch.h>
#include <string>

#include "inputmonitor.h"
#include "logger.h"
#include <utility.h>


#pragma comment(lib,"User32.lib")
#pragma comment(lib, "Pathcch.lib")


sqlite3* g_db = nullptr;
HWINEVENTHOOK g_hook = nullptr;

ULONGLONG g_AppStartTick = 0;

DWORD g_CurrentPID = 0;

std::wstring g_CurrentExe;
std::wstring g_CurrentTitle;

SYSTEMTIME g_AppStartTime;
SYSTEMTIME g_AppEndTime;

DWORD g_IdleSecondsForCurrentApp = 0;

bool g_FirstWindow = true;

HWND g_LastWindow = NULL;

bool g_IsIdle = false;

ULONGLONG g_IdleStartTick = 0;

const DWORD IDLE_THRESHOLD = 5;


static DWORD GetIdleSeconds()
{
    LASTINPUTINFO li{};
    li.cbSize = sizeof(li);

    if (!GetLastInputInfo(&li))
    {
        LOG(L"GetLastInputInfo failed\n");
        return 0;
    }

    DWORD idle = (DWORD)(GetTickCount64() - li.dwTime) / 1000;
    //printf("Current Idle = %u\n", idle);
    return idle;
}

static void CheckIdle()
{
    DWORD idle = GetIdleSeconds();
    //printf("Idle Seconds = %u\r", idle);

    if (!g_IsIdle && idle >= IDLE_THRESHOLD)
    {
        g_IsIdle = true;
        g_IdleStartTick = GetTickCount64();
        LOG(L"User became IDLE\n");
    }
    else if (g_IsIdle && idle < 2)
    {
        DWORD duration = (DWORD)((GetTickCount64() - g_IdleStartTick) / 1000);

        g_IdleSecondsForCurrentApp += duration;
        LOG(L"User Active Again. Idle=%u sec\n", duration);
        g_IsIdle = false;
    }
}



static bool InitDatabase()
{
    wchar_t dbFilePathW[MAX_PATH];
    const wchar_t* dbFileName = L"activity.db";

    GetModuleFileNameW(NULL, dbFilePathW, MAX_PATH);

    PathCchRemoveFileSpec(dbFilePathW, MAX_PATH);

    PathCchAppend(dbFilePathW, MAX_PATH, dbFileName);


    // Convert and extract the const char*
    std::string dbFilePath = Utility::ConvertPwstrToUtf8(dbFilePathW);
    if (sqlite3_open(dbFilePath.c_str(), &g_db) != SQLITE_OK)
    {
        LOG(L"Cannot open database\n");
        return false;
    }
    const char* sql =
        "CREATE TABLE IF NOT EXISTS AppUsage("
        "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "StartTime TEXT,"
        "EndTime TEXT,"
        "ProcessId INTEGER,"
        "ProcessName TEXT,"
        "WindowTitle TEXT,"
        "ForegroundSeconds INTEGER,"
        "IdleSeconds INTEGER,"
        "ActiveSeconds INTEGER"
        ");";

    char* err = nullptr;

    if (sqlite3_exec(g_db, sql, nullptr, nullptr, &err) != SQLITE_OK)
    {
        LOG(L"%s\n", err);
        sqlite3_free(err);
        return false;
    }

    return true;
}

static void InsertApplicationUsage(
    DWORD pid,
    const wchar_t* exe,
    const wchar_t* title,
    DWORD foreground,
    DWORD idle,
    DWORD active)
{
    sqlite3_stmt* stmt;

    const char* sql =
        "INSERT INTO AppUsage("
        "StartTime,"
        "EndTime,"
        "ProcessId,"
        "ProcessName,"
        "WindowTitle,"
        "ForegroundSeconds,"
        "IdleSeconds,"
        "ActiveSeconds)"
        "VALUES(datetime(?),datetime(?),?,?,?,?,?,?);";

    sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);

    char start[32];
    char end[32];

    sprintf_s(start,
        "%04d-%02d-%02d %02d:%02d:%02d",
        g_AppStartTime.wYear,
        g_AppStartTime.wMonth,
        g_AppStartTime.wDay,
        g_AppStartTime.wHour,
        g_AppStartTime.wMinute,
        g_AppStartTime.wSecond);

    sprintf_s(end,
        "%04d-%02d-%02d %02d:%02d:%02d",
        g_AppEndTime.wYear,
        g_AppEndTime.wMonth,
        g_AppEndTime.wDay,
        g_AppEndTime.wHour,
        g_AppEndTime.wMinute,
        g_AppEndTime.wSecond);

    sqlite3_bind_text(stmt, 1, start, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, end, -1, SQLITE_TRANSIENT);

    sqlite3_bind_int(stmt, 3, pid);

    sqlite3_bind_text16(stmt, 4, exe, -1, SQLITE_TRANSIENT);

    sqlite3_bind_text16(stmt, 5, title, -1, SQLITE_TRANSIENT);

    sqlite3_bind_int(stmt, 6, foreground);
    sqlite3_bind_int(stmt, 7, idle);
    sqlite3_bind_int(stmt, 8, active);

    sqlite3_step(stmt);

    sqlite3_finalize(stmt);
}


static void CALLBACK WinEventProc(
    HWINEVENTHOOK,
    DWORD event,
    HWND hwnd,
    LONG,
    LONG,
    DWORD,
    DWORD)
{
    if (event != EVENT_SYSTEM_FOREGROUND)
        return;

    if (hwnd == g_LastWindow)
        return;

    g_LastWindow = hwnd;

    wchar_t title[512] = L"";
    GetWindowTextW(hwnd, title, 512);

    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);

    wchar_t exe[MAX_PATH] = L"";

    HANDLE hProcess =
        OpenProcess(
            PROCESS_QUERY_LIMITED_INFORMATION,
            FALSE,
            pid);

    if (hProcess)
    {
        DWORD size = MAX_PATH;

        QueryFullProcessImageNameW(
            hProcess,
            0,
            exe,
            &size);

        CloseHandle(hProcess);
    }

    if (!g_FirstWindow)
    {
        GetLocalTime(&g_AppEndTime);

        DWORD foreground =
            (DWORD)((GetTickCount64() - g_AppStartTick) / 1000);

        if (foreground <= 10)
        {
            return;
        }

        DWORD active = foreground - g_IdleSecondsForCurrentApp;


       // INPUT_STATS stats = GetInputStatistics();
        //auto events = GetInputEvents();

        InsertApplicationUsage(
            g_CurrentPID,
            g_CurrentExe.c_str(),
            g_CurrentTitle.c_str(),
            foreground,
            g_IdleSecondsForCurrentApp,
            active);

        LOG(L"Session Finished\n\n");
        /*
        LOG(L"Mouse Moves       : %llu\n", stats.MouseMoves);
        LOG(L"Mouse Distance    : %llu\n", stats.MouseDistance);
        LOG(L"Mouse Clicks      : %llu\n", stats.MouseClicks);
        LOG(L"Mouse Wheel       : %llu\n", stats.MouseWheel);
        LOG(L"Keyboard          : %llu\n", stats.KeyPresses);
        LOG(L"Injected Mouse    : %llu\n", stats.InjectedMouse);
        LOG(L"Injected Keyboard : %llu\n", stats.InjectedKeyboard);
        LOG(L"Events Recorded   : %zu\n", GetInputEvents().size());
        */

        LOG(L"\nForeground : %u sec\n", foreground);
        LOG(L"Idle       : %u sec\n", g_IdleSecondsForCurrentApp);
        LOG(L"Active     : %u sec\n", active);
    }

    SYSTEMTIME st;
    GetLocalTime(&st);

    LOG(L"\n==============================\n");
    LOG(L"%04d-%02d-%02d %02d:%02d:%02d\n",
        st.wYear,
        st.wMonth,
        st.wDay,
        st.wHour,
        st.wMinute,
        st.wSecond);

    LOG(L"PID   : %lu\n", pid);
    LOG(L"Title : %ls\n", title);
    LOG(L"Exe   : %ls\n", exe);

    g_CurrentPID = pid;
    g_CurrentExe = exe;
    g_CurrentTitle = title;

    GetLocalTime(&g_AppStartTime);
     
    g_AppStartTick = GetTickCount64();

    ResetInputStatistics();

    g_IdleSecondsForCurrentApp = 0;

    g_FirstWindow = false;
}


int main(int argc, char* argv[])
{
    if (!InitDatabase())
        return 0;

    boolean enableConsoleLogger = false;
    boolean enableFileLogger = false;
    boolean disableConsole = false;


    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-logtoconsole") == 0)
        {
            enableConsoleLogger = true;
        }
        else if (strcmp(argv[i], "-logtofile") == 0)
        {
            enableFileLogger = true;
        }
        else if (strcmp(argv[i], "-logtoboth") == 0)
        {
            enableConsoleLogger = true;
            enableFileLogger = true;
        }
        else if (strcmp(argv[i], "-disableconsole") == 0)
        {
            disableConsole = true;
        }
        else
        {
            enableConsoleLogger = true;
            printf("Unknown argument: %s\n", argv[i]);
        }
    }

    //Disable the Console Completely
    if (disableConsole) {
        enableConsoleLogger = false;
        enableFileLogger = true;

        FreeConsole();
        HWND hwnd = GetConsoleWindow();
        ShowWindow(hwnd, SW_HIDE);
    }


    if (enableConsoleLogger && enableFileLogger) {
        InitLogger(LOG_BOTH);
    }
    else if (enableConsoleLogger) {
        InitLogger(LOG_CONSOLE);
    }
    else if (enableFileLogger) {
        InitLogger(LOG_FILE);
    }

   
    g_hook =
        SetWinEventHook(
            EVENT_SYSTEM_FOREGROUND,
            EVENT_SYSTEM_FOREGROUND,
            NULL,
            WinEventProc,
            0,
            0,
            WINEVENT_OUTOFCONTEXT |
            WINEVENT_SKIPOWNPROCESS);

    if (!g_hook)
    {
        LOG(L"Hook installation failed.\n");
        return 0;
    }

    /*
    if (!StartInputMonitor())
    {
        LOG(L"Input monitor Hook failed.\n");
        return 0;
    }
    */

    LOG(L"Monitoring started...\n");

    SetTimer(NULL, 1, 1000, NULL);

    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == WM_TIMER)
        {
            CheckIdle();
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }


    UnhookWinEvent(g_hook);

    if (!g_FirstWindow)
    {
        GetLocalTime(&g_AppEndTime);

        DWORD foreground =
            (DWORD)((GetTickCount64() - g_AppStartTick) / 1000);

        DWORD active =
            foreground - g_IdleSecondsForCurrentApp;

        InsertApplicationUsage(
            g_CurrentPID,
            g_CurrentExe.c_str(),
            g_CurrentTitle.c_str(),
            foreground,
            g_IdleSecondsForCurrentApp,
            active);
    }

    StopInputMonitor();

    sqlite3_close(g_db);

    CloseLogger();

    return 0;
}

