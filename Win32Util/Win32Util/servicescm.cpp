
#include <windows.h>
#include <stdio.h>
#include <servicescm.h>



BOOL getServiceBinaryPath(SC_HANDLE hSCM, LPCWSTR serviceName, LPWSTR* binaryPath)
{
    SC_HANDLE hService = OpenServiceW(hSCM, serviceName, SERVICE_QUERY_CONFIG);
    if (!hService)
        return FALSE;


    DWORD bytesNeeded = 0;
    BOOL bufferSizeNeeded = QueryServiceConfigW(hService, NULL, 0, &bytesNeeded);
    if (bytesNeeded==0)
    {
        printf("Buffer Size Query failed (%lu)\n", GetLastError());
        CloseServiceHandle(hService);
        return FALSE;
    }

    QUERY_SERVICE_CONFIGW* config = (QUERY_SERVICE_CONFIGW*)malloc(bytesNeeded);
    if (!config)
    {
        printf("Buffer Allocation failed (%lu)\n", GetLastError());
        CloseServiceHandle(hService);
        return FALSE;
    }

    if (!QueryServiceConfigW(
        hService,
        config,
        bytesNeeded,
        &bytesNeeded))
    {
        printf("Query Service Config failed (%lu)\n", GetLastError());
        free(config);
        CloseServiceHandle(hService);
        return FALSE;
    }

    *binaryPath = _wcsdup(config->lpBinaryPathName);

    free(config);
    CloseServiceHandle(hService);
    return TRUE;
}


LPCWSTR getServiceStateString(DWORD state)
{
    switch (state)
    {
    case SERVICE_STOPPED:
        return L"STOPPED";
    case SERVICE_START_PENDING:
        return L"START PENDING";
    case SERVICE_STOP_PENDING:
        return L"STOP PENDING";
    case SERVICE_RUNNING:
        return L"RUNNING";
    case SERVICE_CONTINUE_PENDING:
        return L"CONTINUE PENDING";
    case SERVICE_PAUSE_PENDING:
        return L"PAUSE PENDING";
    case SERVICE_PAUSED:
        return L"PAUSED";
    default:
        return L"UNKNOWN";
    }
}


void printServiceDetailsHorizontally(ENUM_SERVICE_STATUS_PROCESS* services, DWORD serviceCount, SC_HANDLE scHandle) {
    for (DWORD i = 0; i < serviceCount; i++)
    {
        LPWSTR binPath = NULL;
        wprintf(L"Service Name : %ls\n", services[i].lpServiceName);
        wprintf(L"Display Name : %ls\n", services[i].lpDisplayName);
        wprintf(L"State        : %ls\n", getServiceStateString(services[i].ServiceStatusProcess.dwCurrentState));
        //printServiceState(services[i].ServiceStatusProcess.dwCurrentState);
        //printf("\n");


        if (getServiceBinaryPath(scHandle, services[i].lpServiceName, &binPath))
        {
            wprintf(L"Binary       : %ls\n", binPath);
            free(binPath);
        }
        wprintf(L"Process ID   : %lu\n", services[i].ServiceStatusProcess.dwProcessId);
        wprintf(L"\n");
    }
}


void ServicesSCM::getAllServiceDetails() {

    SC_HANDLE scHandle = OpenSCManager(
        NULL,                    // local machine
        NULL,                    // SERVICES_ACTIVE_DATABASE
        SC_MANAGER_ENUMERATE_SERVICE
    );

    if (!scHandle)
    {
        printf("OpenSCManager failed (%lu)\n", GetLastError());
        return;
    }

    DWORD bytesNeeded = 0;
    DWORD serviceCount = 0;
    DWORD resumeHandle = 0;

    // First call to get required buffer size
    EnumServicesStatusEx(
        scHandle,
        SC_ENUM_PROCESS_INFO,
        SERVICE_WIN32,
        SERVICE_STATE_ALL,
        NULL,
        0,
        &bytesNeeded,
        &serviceCount,
        &resumeHandle,
        NULL
    );

    DWORD lastError = GetLastError();
    if (lastError != ERROR_MORE_DATA)
    {
        printf("EnumServicesStatusEx failed (%lu)\n", lastError);
        CloseServiceHandle(scHandle);
        return;
    }

    BYTE* buffer = (BYTE*)malloc(bytesNeeded);
    if (!buffer)
    {
        printf("Memory allocation failed\n");
        CloseServiceHandle(scHandle);
        return;
    }

    // Second call to actually get services
    if (!EnumServicesStatusEx(
        scHandle,
        SC_ENUM_PROCESS_INFO,
        SERVICE_WIN32,
        SERVICE_STATE_ALL,
        buffer,
        bytesNeeded,
        &bytesNeeded,
        &serviceCount,
        &resumeHandle,
        NULL))
    {
        printf("EnumServicesStatusEx failed (%lu)\n", GetLastError());
        free(buffer);
        CloseServiceHandle(scHandle);
        return;
    }

    printf("Total Services: %lu\n\n", serviceCount);
    ENUM_SERVICE_STATUS_PROCESS* services = (ENUM_SERVICE_STATUS_PROCESS*)buffer;
    printServiceDetailsHorizontally(services, serviceCount, scHandle);

    free(buffer);
    CloseServiceHandle(scHandle);
    return;

}








