#include "pch.h"

#include <windows.h>
#include <winnt.h>
#include <stdio.h>
#include <tchar.h>


int WinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd)
//int main()
{

    printf("Going to Start Executable......\n");

    LPCSTR executableImage0 = "C:/Program Files/Eclipse Adoptium/jdk-11.0.15.10-hotspot/bin/java.exe\0";

    char command0Buff[1000] = " -jar \"D:/My Git/Tools/Podman/MinimalTCPServer/Image/MinimalTCPServer-1.0.jar\"\0";
    LPSTR command0 = command0Buff;

    char command1Buff[50] = " --version\0";
    LPSTR command1 = command1Buff;

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    si.wShowWindow = 0; //Hide the Process Window
    si.dwFlags = 1; //Hide the Process Window - STARTF_USESHOWWINDOW

    // Start the child process. 
    if (!CreateProcessA(executableImage0,   // Null terminated executable file path
                        command0,        // Null terminated command to be executed by the executable
                        NULL,           // Process handle not inheritable
                        NULL,           // Thread handle not inheritable
                        FALSE,          // Set handle inheritance to FALSE
                        0,              // No creation flags
                        NULL,           // Use parent's environment block
                        NULL,           // Use parent's starting directory 
                        &si,            // Pointer to STARTUPINFO structure
                        &pi)           // Pointer to PROCESS_INFORMATION structure
                        )
    {
        printf("CreateProcess failed (%d).\n", GetLastError());
        return 1;
    }

    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}

