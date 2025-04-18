#pragma once

#include <windows.h>


BOOL openFile(LPCWSTR filePath, bool enableLog);
DWORD readBinaryData(PBYTE readBuffer, DWORD numberOfBytesToRead);
BOOL closeFile(void);


