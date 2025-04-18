
/*
    FileReadWrite.cpp :

    1. CreateFileW :
        The CreateFile function can create a new file or open an existing file. 
        You must specify the file name, creation instructions, and other attributes. 
        When an application creates a new file, the operating system adds it to the specified directory.
        The operating system assigns a unique identifier, called a handle, to each file that is 
        opened or created using CreateFile. 
        An application can use this handle with functions that read from, write to, and describe the file. 
        It is valid until all references to that handle are closed.

    2. ReadFile
        Reads data from the specified file or input/output (I/O) device. 
        Reads occur at the position specified by the file pointer if supported by the device.


*/

#include <windows.h>
#include <stdio.h>
#include <winfile.h>


HANDLE fileHandle = NULL;
bool enableFileLogger = false;
LPCWSTR openedFilePath = NULL;


BOOL openFile(LPCWSTR filePath, bool enableFileLog) {
    openedFilePath = filePath;
    enableFileLogger = enableFileLog;
    fileHandle =  CreateFileW(filePath,
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            wprintf(L"**** Error File Not Found : %s\n", filePath);
        }
        else {
            wprintf(L"**** Failed to create/open file, Error : %lu\n", GetLastError());
        }
        return 0;
    }
    if (enableFileLogger) {
        wprintf(L"File '%s' opened successfully.\n", filePath);
    }
    return 1;
}


DWORD readBinaryData(PBYTE readBuffer, DWORD numberOfBytesToRead) {
    DWORD lpNumberOfBytesRead = 0;

    BOOL readSuccess = ReadFile(fileHandle,
        readBuffer,
        numberOfBytesToRead,
        &lpNumberOfBytesRead,
        NULL);
    if (!readSuccess) {
        wprintf(L"**** Failed to read file, Error : %lu\n", GetLastError());
        return lpNumberOfBytesRead;
    }
    if (enableFileLogger) {
       wprintf(L"Bytes Read - %lu\n", lpNumberOfBytesRead);
    }
   
    return lpNumberOfBytesRead;
}


BOOL closeFile() {
    BOOL fileClosed = CloseHandle(fileHandle);
    if (enableFileLogger) {
        wprintf(L"File Closed - %s\n", openedFilePath);
    }
    return fileClosed;
}



