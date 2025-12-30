
#include <windows.h>
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>
#include <wmiquery.h>

#pragma comment(lib, "wbemuuid.lib")


std::string WMIQuery::getWMIQueryFromOption(char* option) {
    std::string wmiQuery = "SELECT * FROM ";

    if (strcmp(option, "cpu") == 0)
    {
        wmiQuery.append("Win32_Processor");
    } 
    else if (strcmp(option, "logical.disk") == 0)
    {
        wmiQuery.append("Win32_LogicalDisk");
    } 
    else if (strcmp(option, "disk.drive") == 0)
    {
        wmiQuery.append("Win32_DiskDrive");
    }
    else if (strcmp(option, "disk.partition") == 0)
    {
        wmiQuery.append("Win32_DiskPartition");
    }
    else if (strcmp(option, "physical.memory") == 0)
    {
        wmiQuery.append("Win32_PhysicalMemory");
    }
    else if (strcmp(option, "computer.system") == 0)
    {
        wmiQuery.append("Win32_ComputerSystem");
    }
    else if (strcmp(option, "operating.system") == 0)
    {
        wmiQuery.append("Win32_OperatingSystem");
    }
    else if (strcmp(option, "os.hot.fixes") == 0)
    {
        wmiQuery.append("Win32_QuickFixEngineering");
    }
    else if (strcmp(option, "network.interface") == 0)
    {
        wmiQuery.append("Win32_NetworkAdapter");
    }
    else if (strcmp(option, "bios") == 0)
    {
        wmiQuery.append("Win32_BIOS");
    }
    else if (strcmp(option, "system.users") == 0)
    {
        wmiQuery.append("Win32_SystemUsers");
    }
    return wmiQuery;
}


void WMIQuery::readVTArrayData(VARIANT vtProp) {
    if (vtProp.vt != (VT_ARRAY | VT_I4) &&
        vtProp.vt != (VT_ARRAY | VT_BSTR)) {
        return;
    }

    SAFEARRAY* psa = vtProp.parray;
    long lBound, uBound;
    HRESULT hr = SafeArrayGetLBound(psa, 1, &lBound);
    hr = SafeArrayGetUBound(psa, 1, &uBound);

    std::cout << "{";
    for (long i = lBound; i <= uBound; i++)
    {
        if (i > 0) {
            std::cout << ",";
        }

        if (vtProp.vt == (VT_ARRAY | VT_I4)) {
            LONG value = 0;   // VT_I4 → LONG
            hr = SafeArrayGetElement(psa, &i, &value);
            if (SUCCEEDED(hr))
            {
                std::cout << value;
            }
        }
        else if (vtProp.vt == (VT_ARRAY | VT_BSTR)) {
            BSTR bstrValue = nullptr;
            hr = SafeArrayGetElement(psa, &i, &bstrValue);
            if (SUCCEEDED(hr) && bstrValue != nullptr)
            {
                std::wcout << "\"" << bstrValue << "\"";
                SysFreeString(bstrValue);
            }
        }
    }
    std::cout << "}";
}


void WMIQuery::printWMIQueryResults(std::string wmiQuery) {
    HRESULT hres;

    // Initialize COM
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        std::wcout << "Error : Failed to Initialize COM";
        return;
    }
        

    // Initialize security
    hres = CoInitializeSecurity(NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, NULL);
    if (FAILED(hres)) {
        std::wcout << "Error : Failed to Initialize COM Security";
        return;
    }

    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);
    if (FAILED(hres)) {
        std::wcout << "Error : Failed to Create COM Instance";
        return;
    }

    IWbemServices* pSvc = NULL;
    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
    if (FAILED(hres)) {
        std::wcout << "Error : Failed to Connect to CIMV2 Server";
        return;
    }

    CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE);

    /*
     * 
      CPU                    -> SELECT * FROM Win32_Processor
      Logical Disk           -> SELECT * FROM Win32_LogicalDisk
      Disk Drive             -> SELECT * FROM Win32_DiskDrive
      Disk Partition         -> SELECT * FROM Win32_DiskPartition
      Physical Memory        -> SELECT * FROM Win32_PhysicalMemory
      Computer System        -> SELECT * FROM Win32_ComputerSystem
      Operating System (OS)  -> SELECT * FROM Win32_OperatingSystem
      HotFixes               -> SELECT * FROM Win32_QuickFixEngineering
      NetworkInterface       -> SELECT * FROM Win32_NetworkAdapter
      BIOS                   -> SELECT * FROM Win32_BIOS
      SystemUsers            -> SELECT * FROM Win32_SystemUsers

      std::string wmiQuery = "SELECT * FROM Win32_SystemUsers";
    *
    */

    // Query Information
    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t(wmiQuery.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL, &pEnumerator);
    if (FAILED(hres)) {
        std::wcout << "Error : Failed to Execute WIM Query";
        return;
    }


    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;
    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (0 == uReturn) break;

        // Get all property names
        SAFEARRAY* pNames = NULL;
        hr = pclsObj->GetNames(NULL, WBEM_FLAG_ALWAYS | WBEM_FLAG_NONSYSTEM_ONLY, NULL, &pNames);
        if (SUCCEEDED(hr) && pNames != NULL) {
            long lBound, uBound;
            SafeArrayGetLBound(pNames, 1, &lBound);
            SafeArrayGetUBound(pNames, 1, &uBound);

            for (long i = lBound; i <= uBound; i++) {
                BSTR bstrName;
                SafeArrayGetElement(pNames, &i, &bstrName);

                VARIANT vtProp;
                VariantInit(&vtProp);

                hr = pclsObj->Get(bstrName, 0, &vtProp, 0, 0);
                if (SUCCEEDED(hr)) {
                    // Print property name and value
                    std::wcout << bstrName << L"=";
                    switch (vtProp.vt) {
                    case VT_BSTR: std::wcout << vtProp.bstrVal; break;
                    case VT_I4: std::wcout << vtProp.intVal; break;
                    case VT_UI4: std::wcout << vtProp.uintVal; break;
                    case VT_BOOL: std::wcout << (vtProp.boolVal ? L"TRUE" : L"FALSE"); break;
                    case VT_ARRAY | VT_I4: readVTArrayData(vtProp); break;
                    case VT_ARRAY | VT_BSTR: readVTArrayData(vtProp); break;
                    default: std::wcout << L""; break;
                    }
                    std::wcout << std::endl;
                }
                VariantClear(&vtProp);
                SysFreeString(bstrName);
            }
            SafeArrayDestroy(pNames);
        }

        pclsObj->Release();
    }

    // Cleanup
    pEnumerator->Release();
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();

    return;
}




