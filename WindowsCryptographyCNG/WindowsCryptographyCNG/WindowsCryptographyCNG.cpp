/*

    WindowsCryptographyCNG.cpp

    This file contains the 'main' function.Program execution begins and ends there.

*/


#include <iostream>
#include <wincrypto.h>
#include <winfile.h>


static int getFileSHA256Hash(LPCWSTR filePath) {
    BYTE readBuffer[1024] = { 0 };
    char sha256[] = "SHA-256";

    BOOL hashInit = initializeHasher(sha256, false);
    if (hashInit == 0) {
        wprintf(L"Failed to initialize Hasher.\n");
        return 0;
    }

    wprintf(L"Getting Hash for file '%s'\n", filePath);
    BOOL fileOpened = openFile(filePath, false);
    if (!fileOpened) {
        wprintf(L"Failed to Create/Open file\n");
    }

    DWORD lpNumberOfBytesRead = -1;
    while ((lpNumberOfBytesRead = readBinaryData(readBuffer, sizeof(readBuffer))) > 0) {
        BOOL dataHashed = hashData(readBuffer, lpNumberOfBytesRead);
        if (!dataHashed) {
            break;
        }
    }

    BOOL fileClosed = closeFile();

    PBYTE pbHashHexValue = NULL;
    DWORD hashLength = getHashHexValue(&pbHashHexValue);
    if (NULL == pbHashHexValue) {
        printf("Failed to fetch Hash Value.");
        return 0;
    }
    printf("Hash Algo.: %s\n", sha256);
    printf("Hash Length: %d\n", hashLength);
    printf("Hash Value: ");
    for (DWORD i = 0; i < hashLength; ++i) {
        printf("%c", *(pbHashHexValue + i));
    }
    printf("\n");
    freeHashHexValueBuffer(&pbHashHexValue);
}


int main() {
    //LPCWSTR fileToBeHashedPath = L"D:/10012025105717.zip";
    //getFileSHA256Hash(fileToBeHashedPath);

    encrypt();




}



