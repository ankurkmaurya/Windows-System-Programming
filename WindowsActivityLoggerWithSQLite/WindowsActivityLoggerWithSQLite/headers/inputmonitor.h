#pragma once

#include <Windows.h>
#include <vector>

enum class INPUT_EVENT_TYPE
{
    MouseMove,
    MouseClick,
    MouseWheel,
    KeyPress
};

struct INPUT_EVENT
{
    // Timestamp of the event
    ULONGLONG Tick;

    // Event type
    INPUT_EVENT_TYPE Type;

    // Mouse position
    POINT Position;

    // Mouse movement
    LONG DeltaX;
    LONG DeltaY;

    // Virtual key code (keyboard only)
    DWORD Key;

    // Wheel delta (wheel only)
    SHORT WheelDelta;

    // Whether Windows marked the event as injected
    bool Injected;
};

struct INPUT_STATS
{
    ULONGLONG MouseMoves = 0;
    ULONGLONG MouseDistance = 0;
    ULONGLONG MouseClicks = 0;
    ULONGLONG MouseWheel = 0;
    ULONGLONG KeyPresses = 0;
    ULONGLONG InjectedMouse = 0;
    ULONGLONG InjectedKeyboard = 0;
};

bool StartInputMonitor();
void StopInputMonitor();
void ResetInputStatistics();
INPUT_STATS GetInputStatistics();
std::vector<INPUT_EVENT> GetInputEvents();


