
#include "inputmonitor.h"
#include <cmath>

#include <deque>


static INPUT_STATS g_Stats = {};
static std::deque<INPUT_EVENT> g_InputEvents;
static POINT g_LastMouse = { 0,0 };
static HHOOK g_MouseHook = NULL;
static HHOOK g_KeyHook = NULL;



static void AddEvent(const INPUT_EVENT& e)
{
    constexpr size_t MAX_EVENTS = 50000;

    if (g_InputEvents.size() >= MAX_EVENTS)
    {
        g_InputEvents.pop_front();
    }

    g_InputEvents.push_back(e);
}


static LRESULT CALLBACK MouseProc(
    int code,
    WPARAM wParam,
    LPARAM lParam)
{
    if (code != HC_ACTION)
        return CallNextHookEx(NULL, code, wParam, lParam);

    MSLLHOOKSTRUCT* ms = (MSLLHOOKSTRUCT*)lParam;

    bool injected = (ms->flags & LLMHF_INJECTED) != 0;

    if (injected)
        g_Stats.InjectedMouse++;

    INPUT_EVENT e = {};
    e.Tick = GetTickCount64();
    e.Position = ms->pt;
    e.Key = 0;
    e.DeltaX = 0;
    e.DeltaY = 0;
    e.WheelDelta = 0;
    e.Injected = injected;

    switch (wParam)
    {
        case WM_MOUSEMOVE:
        {
            LONG dx = ms->pt.x - g_LastMouse.x;
            LONG dy = ms->pt.y - g_LastMouse.y;

            g_Stats.MouseDistance += abs(dx) + abs(dy);
            g_Stats.MouseMoves++;
            e.Type = INPUT_EVENT_TYPE::MouseMove;
            e.DeltaX = dx;
            e.DeltaY = dy;
            AddEvent(e);
            g_LastMouse = ms->pt;
            break;
        }

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
            g_Stats.MouseClicks++;
            e.Type = INPUT_EVENT_TYPE::MouseClick;
            AddEvent(e);
            break;
        
        case WM_MOUSEWHEEL:

            g_Stats.MouseWheel++;
            e.Type = INPUT_EVENT_TYPE::MouseWheel;
            e.WheelDelta = GET_WHEEL_DELTA_WPARAM(ms->mouseData);
            AddEvent(e);
            break;
    }
    return CallNextHookEx(NULL, code, wParam, lParam);
}


static LRESULT CALLBACK KeyboardProc(
    int code,
    WPARAM wParam,
    LPARAM lParam)
{
    if (code != HC_ACTION)
        return CallNextHookEx(NULL, code, wParam, lParam);

    KBDLLHOOKSTRUCT* kb = (KBDLLHOOKSTRUCT*)lParam;

    bool injected = (kb->flags & LLKHF_INJECTED) != 0;
    if (injected)
        g_Stats.InjectedKeyboard++;

    if (wParam == WM_KEYDOWN)
    {
        g_Stats.KeyPresses++;

        INPUT_EVENT e;
        e.Tick = GetTickCount64();
        e.Type = INPUT_EVENT_TYPE::KeyPress;
        e.Position = { 0,0 };
        e.DeltaX = 0;
        e.DeltaY = 0;
        e.WheelDelta = 0;
        e.Key = kb->vkCode;
        e.Injected = injected;
        AddEvent(e);
    }
    return CallNextHookEx(NULL, code, wParam, lParam);
}


bool StartInputMonitor()
{
    GetCursorPos(&g_LastMouse);

    g_MouseHook =
        SetWindowsHookEx(
            WH_MOUSE_LL,
            MouseProc,
            GetModuleHandle(NULL),
            0);

    if (!g_MouseHook)
        return false;

    g_KeyHook =
        SetWindowsHookEx(
            WH_KEYBOARD_LL,
            KeyboardProc,
            GetModuleHandle(NULL),
            0);

    if (!g_KeyHook)
    {
        UnhookWindowsHookEx(g_MouseHook);
        g_MouseHook = NULL;
        return false;
    }

    return true;
}


void StopInputMonitor()
{
    if (g_MouseHook)
    {
        UnhookWindowsHookEx(g_MouseHook);
        g_MouseHook = NULL;
    }

    if (g_KeyHook)
    {
        UnhookWindowsHookEx(g_KeyHook);
        g_KeyHook = NULL;
    }
}


void ResetInputStatistics()
{
    ZeroMemory(&g_Stats, sizeof(g_Stats));

    g_InputEvents.clear();

    GetCursorPos(&g_LastMouse);
}


INPUT_STATS GetInputStatistics()
{
    return g_Stats;
}


std::vector<INPUT_EVENT>
GetInputEvents()
{
    return std::vector<INPUT_EVENT>(
        g_InputEvents.begin(),
        g_InputEvents.end());
}


