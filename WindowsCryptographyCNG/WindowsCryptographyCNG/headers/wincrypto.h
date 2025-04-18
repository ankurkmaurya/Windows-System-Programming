#pragma once

#include <windows.h>

#define HASH_SHA_256 "SHA-256"
#define HASH_SHA_1 "SHA-1"

BOOL initializeHasher(char* hashAlgorithm, bool enableLog);
BOOL hashData(PBYTE inputData, ULONG dataSize);
DWORD getHashValue(PBYTE* pbHashValuePtr);
DWORD getHashHexValue(PBYTE* pbHashHexValue);
void freeHashValueBuffer(bool cleanHashBuffer);
void freeHashHexValueBuffer(PBYTE * pbHashHexValuePtr);

