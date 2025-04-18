/*
    HashingDataCNG.cpp :
   
    To create a hash using CNG, perform the following steps:
    1. Open an algorithm provider that supports the desired algorithm. 
       Typical hashing algorithms include MD2, MD4, MD5, SHA-1, and SHA256. 
       Call the BCryptOpenAlgorithmProvider function and specify the appropriate 
       algorithm identifier in the pszAlgId parameter. 
       The function returns a handle to the provider.

    2. Perform the following steps to create the hashing object:
        a. Obtain the size of the object by calling the BCryptGetProperty function 
           to retrieve the BCRYPT_OBJECT_LENGTH property.
        b. Allocate memory to hold the hash object.
        c. Create the object by calling the BCryptCreateHash function.

    3. Hash the data. This involves calling the BCryptHashData function one or more 
       times. Each call appends the specified data to the hash.

    4. Perform the following steps to obtain the hash value:
        a. Retrieve the size of the value by calling the BCryptGetProperty function 
           to get the BCRYPT_HASH_LENGTH property.
        b. Allocate memory to hold the value.
        c. Retrieve the hash value by calling the BCryptFinishHash function. 
           After this function has been called, the hash object is no longer valid.

    5. To complete this procedure, you must perform the following cleanup steps:
        a. Close the hash object by passing the hash handle to the BCryptDestroyHash function.
        b. Free the memory you allocated for the hash object.
        c. If you will not be creating any more hash objects, close the algorithm provider by 
           passing the provider handle to the BCryptCloseAlgorithmProvider function. 
           If you will be creating more hash objects, we suggest you reuse the algorithm provider 
           rather than creating and destroying the same type of algorithm provider many times.
        d. When you have finished using the hash value memory, free it.

*/

#include <windows.h>
#include <stdio.h>
#include <bcrypt.h>
#include <wincrypto.h>

#define NT_SUCCESS(Status)  (((NTSTATUS)(Status)) >= 0)


PCWCH HASHING_ALGORITHM = NULL;
BCRYPT_ALG_HANDLE algHandle = NULL;
BCRYPT_HASH_HANDLE hHash = NULL;
NTSTATUS status = 0;

DWORD cbData = 0,
cbHash = 0,
cbHashObject = 0;

UCHAR pbOutput = 0;
ULONG pcbResult = 0;

PBYTE pbHashObject = NULL;
PBYTE pbHash = NULL;


static void cleanup(bool cleanHashBuffer);
static BOOL initializeHashHandler(PCWCH HASHING_ALGORITHM);

bool enableHashLogger = true;

BOOL initializeHasher(char* hashAlgorithm, bool enableHashLog) {
    enableHashLogger = enableHashLog;
    if (strcmp(hashAlgorithm, HASH_SHA_256) == 0) {
        HASHING_ALGORITHM = BCRYPT_SHA256_ALGORITHM;
        return initializeHashHandler(BCRYPT_SHA256_ALGORITHM);
    } 
    else if (strcmp(hashAlgorithm, HASH_SHA_1) == 0) {
        HASHING_ALGORITHM = BCRYPT_SHA1_ALGORITHM;
        return initializeHashHandler(BCRYPT_SHA1_ALGORITHM);
    }
    else {
        return 0;
    }
}

static BOOL initializeHashHandler(PCWCH HASHING_ALGORITHM) {
    if (enableHashLogger) {
        wprintf(L"------------------ HASHING %s ----------------------\n", HASHING_ALGORITHM);
    }

    //Open an algorithm provider that supports the desired algorithm
    status = BCryptOpenAlgorithmProvider(&algHandle,
                                         HASHING_ALGORITHM,
                                         NULL,
                                         0);
    if (!NT_SUCCESS(status)) {
        wprintf(L"**** Error 0x%x returned by BCryptOpenAlgorithmProvider\n", status);
        cleanup(true);
        return 0;
    }
    if (enableHashLogger) {
        wprintf(L"AlgorithmProvider Opened for %s\n", HASHING_ALGORITHM);
    }

    //Create the hashing object
    //Obtain the size of the buffer to hold the hash object
    status = BCryptGetProperty(algHandle,
                                BCRYPT_OBJECT_LENGTH,
                                (PBYTE) &cbHashObject,
                                sizeof(DWORD),
                                &pcbResult,
                                0);
    if (!NT_SUCCESS(status)) {
        wprintf(L"**** Error 0x%x returned by BCryptGetProperty\n", status);
        cleanup(true);
        return 0;
    }
    if (enableHashLogger) {
        printf("BCRYPT_OBJECT_LENGTH is %lu bytes\n", cbHashObject);
    }

    //Allocate memory to hold the hash object
    pbHashObject = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbHashObject);
    if (NULL == pbHashObject)
    {
        wprintf(L"**** Memory allocation failed.\n");
        cleanup(true);
        return 0;
    }

    //Retrieve the size of the buffer to hold the hash value
    status = BCryptGetProperty(algHandle,
                                BCRYPT_HASH_LENGTH,
                                (PBYTE)&cbHash,
                                sizeof(DWORD),
                                &cbData,
                                0);
    if (!NT_SUCCESS(status)) {
        wprintf(L"**** Error 0x%x returned by BCryptGetProperty\n", status);
        cleanup(true);
        return 0;
    }
    if (enableHashLogger) {
        printf("BCRYPT_HASH_LENGTH is %lu bytes\n", cbHash);
    }

    //Allocate the hash buffer on the heap to hold the hash value.
    pbHash = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbHash);
    if (NULL == pbHash)
    {
        wprintf(L"**** Memory allocation failed\n");
        cleanup(true);
        return 0;
    }

    //Create the hashing object
    status = BCryptCreateHash(algHandle,
                                &hHash,
                                pbHashObject,
                                cbHashObject,
                                NULL,
                                0,
                                0);
    if (!NT_SUCCESS(status)) {
        wprintf(L"**** Error 0x%x returned by BCryptCreateHash\n", status);
        cleanup(true);
        return 0;
    }

    return 1;
}


BOOL hashData(PBYTE inputData, ULONG dataSize) {
    //Hash the data
    NTSTATUS status = BCryptHashData(hHash,
        (PBYTE)inputData,
        dataSize,
        //sizeof(inputData),
        0);
    if (!NT_SUCCESS(status)) {
        wprintf(L"**** Error 0x%x returned by BCryptHashData\n", status);
        return 0;
    }
    return 1;
}

DWORD getHashValue(PBYTE* pbHashValuePtr) {
    //Retrieve the hash value by calling the BCryptFinishHash function
    status = BCryptFinishHash(hHash,
        pbHash,
        cbHash,
        0);
    if (!NT_SUCCESS(status)) {
        wprintf(L"**** Error 0x%x returned by BCryptFinishHash\n", status);
        cleanup(true);
        return 0;
    }
    if (enableHashLogger) {
        wprintf(L"Hash Data Obtained Successfully!\n");
        wprintf(L"Converting Hash Data to Hex Values\n");
    }
    *pbHashValuePtr = pbHash;

    cleanup(false); //Cleanup the hashing provider
    return cbHash;
}


DWORD getHashHexValue(PBYTE* pbHashHexValuePtr) {
    PBYTE pbHash;  
    int hashBufferSize = getHashValue(&pbHash);
    int hexBufferSize = hashBufferSize*2;
    char hexBuffer[3];
    int cx = 0;

    //Allocate the hash Hex buffer on the heap to hold the hash hex converted values.
    PBYTE pbHashHexValue = (PBYTE)HeapAlloc(GetProcessHeap(), 0, hexBufferSize);
    if (NULL == pbHashHexValue)
    {
        wprintf(L"**** Memory allocation failed\n");
        cleanup(true);
        return 0;
    }
    if (enableHashLogger) {
        printf("HASH VALUE : ");
    }
    int hex = 0;
    for (DWORD i = 0; i < cbHash; ++i) {
        cx = snprintf(hexBuffer, 3, "%02x", *(pbHash + i));
        *(pbHashHexValue + hex) = (BYTE)hexBuffer[0];
        *(pbHashHexValue + hex + 1) = (BYTE)hexBuffer[1];

        if (enableHashLogger) {
            printf("%s", hexBuffer);
        }
        hex += cx;
    }
    if (enableHashLogger) {
        printf("\n");
    }
    *pbHashHexValuePtr = pbHashHexValue;

    freeHashValueBuffer(true); //Cleanup the hash buffer
    return hexBufferSize;
}

static void cleanup(bool cleanHashBuffer) {
    if (enableHashLogger){
        wprintf(L"\nCLEANUP :\n");
        wprintf(L"Starting Cleanup after Hashing.\n");
    }
    if (algHandle){
        BCryptCloseAlgorithmProvider(algHandle, 0);
        if (enableHashLogger) {
            wprintf(L"AlgorithmProvider Closed for %s\n", HASHING_ALGORITHM);
        }
    }   

    if (hHash){   
        BCryptDestroyHash(hHash);
        if (enableHashLogger) {
            wprintf(L"Hashing Object Destroyed\n");
        }
    }   

    if (pbHashObject){
        HeapFree(GetProcessHeap(), 0, pbHashObject);
        if (enableHashLogger) {
            wprintf(L"Memory Released for Hashing Object\n");
        }
    }

    freeHashValueBuffer(cleanHashBuffer);
    
    if (enableHashLogger){
        wprintf(L"Cleanup process completed.\n");
    }

}

void freeHashValueBuffer(bool cleanHashBuffer) {
    if (pbHash && cleanHashBuffer) {
        HeapFree(GetProcessHeap(), 0, pbHash);
        if (enableHashLogger) {
            wprintf(L"Memory Released for Hashed Data Buffer\n");
        }
    }
}

void freeHashHexValueBuffer(PBYTE* pbHashHexValuePtr) {
    if (pbHashHexValuePtr) {
        HeapFree(GetProcessHeap(), 0, *pbHashHexValuePtr);
        if (enableHashLogger) {
            wprintf(L"Memory Released for Hashed Hex Data Buffer\n");
        }
    }
}








