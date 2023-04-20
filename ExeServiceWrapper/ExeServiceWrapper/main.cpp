#include "pch.h"

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

#pragma comment(lib, "advapi32.lib")

PROCESS_INFORMATION pi;
SERVICE_STATUS          gSvcStatus;
SERVICE_STATUS_HANDLE   gSvcStatusHandle;
HANDLE                  ghSvcStopEvent = NULL;

VOID SvcInstall(void);
VOID SvcUnInstall(void);
BOOL isServiceRunning(SC_HANDLE);
VOID WINAPI SvcCtrlHandler(DWORD);
VOID WINAPI SvcMain(DWORD, LPTSTR*);

VOID ReportSvcStatus(DWORD, DWORD, DWORD);
VOID SvcInit(DWORD, LPTSTR*);
VOID SvcReportEvent(LPTSTR);

wchar_t serviceNameBuff[18] = L"ExeServiceWrapper";
LPWSTR serviceName = serviceNameBuff;

wchar_t serviceNameDisplayBuff[20] = L"Exe Service Wrapper";
LPWSTR serviceNameDisplay = serviceNameDisplayBuff;

char serviceDescBuff[50] = "Wrapper for Executable as a Windows Service.";

//
// Purpose: 
//   Entry point for the process
//
// Parameters:
//   None
// 
// Return value:
//   None, defaults to 0 (zero)
//
int __cdecl _tmain(int argc, TCHAR* argv[])
{
    // If command-line parameter is "install", Install the service. 
    // If command-line parameter is "uninstall", UnInstall the service. 
    // Otherwise, the service is probably being started by the SCM.

    if (lstrcmpi(argv[1], TEXT("install")) == 0)
    {
        SvcInstall();
        return 0;
    }

    if (lstrcmpi(argv[1], TEXT("uninstall")) == 0)
    {
        SvcUnInstall();
        return 0;
    }

    // TO_DO: Add any additional services for the process to this table.
    SERVICE_TABLE_ENTRY DispatchTable[] =
    {
        { serviceName, (LPSERVICE_MAIN_FUNCTION)SvcMain },
        { NULL, NULL }
    };

    // This call returns when the service has stopped. 
    // The process should simply terminate when the call returns.

    if (!StartServiceCtrlDispatcher(DispatchTable))
    {
        SvcReportEvent((LPTSTR)TEXT("StartServiceCtrlDispatcher"));
    }
}

/*
Purpose: 
Installs a service in the SCM database

Parameters:
None
 
Return value:
None
*/
VOID SvcInstall()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    TCHAR szUnquotedPath[MAX_PATH];

    if (!GetModuleFileName(NULL, szUnquotedPath, MAX_PATH))
    {
        printf("Cannot install Service (%d)\n", GetLastError());
        return;
    }

    /*
    In case the path contains a space, it must be quoted so that it is correctly interpreted.
    For example,
    "d:\my share\myservice.exe" should be specified as ""d:\my share\myservice.exe"".
    */ 
    TCHAR szPath[MAX_PATH];
    StringCbPrintf(szPath, MAX_PATH, TEXT("\"%s\""), szUnquotedPath);

    // Get a handle to the SCM database. 
    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 

    if (NULL == schSCManager)
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    // Create the service
    schService = CreateService(
        schSCManager,              // SCM database 
        serviceName,               // name of service 
        serviceNameDisplay,        // service name to display 
        SERVICE_ALL_ACCESS,        // desired access 
        SERVICE_WIN32_OWN_PROCESS, // service type 
        SERVICE_AUTO_START,        // start type 
        SERVICE_ERROR_NORMAL,      // error control type 
        szPath,                    // path to service's binary 
        NULL,                      // no load ordering group 
        NULL,                      // no tag identifier 
        NULL,                      // no dependencies 
        NULL,                      // LocalSystem account 
        NULL);                     // no password 

    if (schService == NULL)
    {
        printf("CreateService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }
    else {
        SERVICE_DESCRIPTIONA serviceDescription;
        ZeroMemory(&serviceDescription, sizeof(serviceDescription));
        serviceDescription.lpDescription = serviceDescBuff;

        if (!ChangeServiceConfig2A(schService, SERVICE_CONFIG_DESCRIPTION, &serviceDescription)) {
            printf("Service Description change failed\n");
        }

        printf("Service Installed Successfully\n");
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}


/*
Purpose:
UnInstalls(Delete) a service from SCM database

Parameters:
None

Return value:
None
*/
VOID SvcUnInstall()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    // Get a handle to the SCM database. 
    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 

    if (NULL == schSCManager)
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    // Get handle to the existing service. 
    schService = OpenServiceW(schSCManager, serviceName, SERVICE_ALL_ACCESS);
    if (schService == NULL)
    {
        printf("Open Service failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }
    else {
        //Removing the Service
        SERVICE_STATUS sStatus;
        ZeroMemory(&sStatus, sizeof(sStatus));

        //First stop the service if it is Running currently
        if (isServiceRunning(schService) && !ControlService(schService, SERVICE_CONTROL_STOP, &sStatus)) {
            printf("Failed to send Service Stop command.\n");
            return;
        }
        Sleep(500);
        if (sStatus.dwCurrentState == SERVICE_STOPPED || sStatus.dwCurrentState == SERVICE_STOP_PENDING)
        {
            printf("Service Stopped Successfully.\n");
        }

        if (!DeleteService(schService)) {
            printf("Failed to Remove Service.\n");
            return;
        }
        printf("Service Removed Successfully.\n");
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

/*
 Purpose:
 Check if the service is running

 Parameters:
 Handle to the service in query

 Return value:
 Boolean - TRUE if Running, Otherwise FALSE
*/
BOOL isServiceRunning(SC_HANDLE schService) {
    DWORD dwBytesNeeded;
    SERVICE_STATUS_PROCESS ssp;

    if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE) &ssp, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
        printf("Service Status Query Failed.\n");
        return FALSE;
    }

    if (ssp.dwCurrentState == SERVICE_RUNNING)
    {
        printf("Service is Running.\n");
        return TRUE;
    }
    else {
        printf("Service is not Running.\n");
        return FALSE;
    }
}

/*
 Purpose: 
   Entry point for the service

 Parameters:
   dwArgc   - Number of arguments in the lpszArgv array
   lpszArgv - Array of strings. The first string is the name of
              the service and subsequent strings are passed by the process
              that called the StartService function to start the service.
 
 Return value:
   None.
*/
VOID WINAPI SvcMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    // Register the handler function for the service

    gSvcStatusHandle = RegisterServiceCtrlHandler(
        serviceName,
        SvcCtrlHandler);

    if (!gSvcStatusHandle)
    {
        SvcReportEvent((LPTSTR)TEXT("RegisterServiceCtrlHandler"));
        return;
    }

    // These SERVICE_STATUS members remain as set here
    gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    gSvcStatus.dwServiceSpecificExitCode = 0;

    // Report initial status to the SCM
    ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

    // Perform service-specific initialization and work.
    SvcInit(dwArgc, lpszArgv);
}

/*
 Purpose:
   The service code

 Parameters:
   dwArgc   - Number of arguments in the lpszArgv array
   lpszArgv - Array of strings. The first string is the name of
              the service and subsequent strings are passed by the process
              that called the StartService function to start the service.

 Return value:
   None
*/
VOID SvcInit(DWORD dwArgc, LPTSTR* lpszArgv)
{
    /* TO_DO: 
       Declare and set any required variables.
       Be sure to periodically call ReportSvcStatus() with 
       SERVICE_START_PENDING. If initialization fails, call
       ReportSvcStatus with SERVICE_STOPPED.
    */

    /*
       Create an event.The control handler function, SvcCtrlHandler,
       signals this event when it receives the stop control code.
    */
    ghSvcStopEvent = CreateEvent(
        NULL,    // default security attributes
        TRUE,    // manual reset event
        FALSE,   // not signaled
        NULL);   // no name

    if (ghSvcStopEvent == NULL)
    {
        ReportSvcStatus(SERVICE_STOPPED, GetLastError(), 0);
        return;
    }

    // Report running status when initialization is complete.
    ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);

    printf("Going to Start Executable......\n");

    LPCSTR executableImage0 = "C:/Program Files/Eclipse Adoptium/jdk-11.0.15.10-hotspot/bin/java.exe\0";

    char command0Buff[500] = " -jar \"D:/My Git/Tools/Podman/MinimalTCPServer/Image/MinimalTCPServer-1.0.jar\"\0";
    LPSTR command0 = command0Buff;

    char command1Buff[50] = " --version\0";
    LPSTR command1 = command1Buff;

    STARTUPINFOA si;

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
    }

    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);


    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
    return;

    // TO_DO: Perform work until service stops.
    /*
        while (1)
        {
            // Check whether to stop the service.
            WaitForSingleObject(ghSvcStopEvent, INFINITE);

            ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
            return;
        }
    */
}

/*
 Purpose:
   Sets the current service status and reports it to the SCM.

 Parameters:
   dwCurrentState  - The current state (see SERVICE_STATUS)
   dwWin32ExitCode - The system error code
   dwWaitHint      - Estimated time for pending operation, in milliseconds

 Return value:
   None
*/
VOID ReportSvcStatus(DWORD dwCurrentState,
    DWORD dwWin32ExitCode,
    DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.
    gSvcStatus.dwCurrentState = dwCurrentState;
    gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    gSvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        gSvcStatus.dwControlsAccepted = 0;
    else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ((dwCurrentState == SERVICE_RUNNING) ||
        (dwCurrentState == SERVICE_STOPPED))
        gSvcStatus.dwCheckPoint = 0;
    else gSvcStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the SCM.
    SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
}

/*
  Purpose: 
   Called by SCM whenever a control code is sent to the service
   using the ControlService function.

 Parameters:
   dwCtrl - control code
 
 Return value:
   None
*/
VOID WINAPI SvcCtrlHandler(DWORD dwCtrl)
{
    // Handle the requested control code. 
    switch (dwCtrl)
    {
        case SERVICE_CONTROL_STOP:
            ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

            // First terminate the running process
            if (!TerminateProcess(pi.hProcess, 0)) {
                printf("Failed to Terminate Process.\n");
            }

            // Close process and thread handles
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            // Signal the service to stop.
            ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
            return;

        case SERVICE_CONTROL_INTERROGATE:
            break;

        default:
            break;
    }
}

/*
  Purpose: 
   Logs messages to the event log

 Parameters:
   szFunction - name of function that failed
 
 Return value:
   None

 Remarks:
   The service must have an entry in the Application event log.
*/
VOID SvcReportEvent(LPTSTR szFunction)
{
    HANDLE hEventSource;
    LPCTSTR lpszStrings[2];
    TCHAR Buffer[80];

    hEventSource = RegisterEventSource(NULL, serviceName);

    if (NULL != hEventSource)
    {
        StringCchPrintf(Buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());

        lpszStrings[0] = serviceName;
        lpszStrings[1] = Buffer;

        ReportEvent(hEventSource,        // event log handle
                    EVENTLOG_ERROR_TYPE, // event type
                    0,                   // event category
                    NULL,                // event identifier
                    NULL,                // no security identifier
                    2,                   // size of lpszStrings array
                    0,                   // no binary data
                    lpszStrings,         // array of strings
                    NULL);               // no binary data

        DeregisterEventSource(hEventSource);
    }
}
