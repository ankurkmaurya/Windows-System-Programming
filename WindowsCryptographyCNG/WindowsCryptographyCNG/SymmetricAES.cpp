// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) Microsoft. All rights reserved.
/*++

Abstract:
    Sample program for AES-CBC encryption using CNG

--*/

#include <windows.h>
#include <stdio.h>
#include <bcrypt.h>
#include <wincrypto.h>
#pragma comment(lib, "bcrypt.lib")

#define NT_SUCCESS(Status)          (((NTSTATUS)(Status)) >= 0)

#define STATUS_UNSUCCESSFUL         ((NTSTATUS)0xC0000001L)


#define DATA_TO_ENCRYPT  "Test Data"


/*
 const BYTE rgbPlaintext[] = 
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};
*/

 const BYTE rgbPlaintext[] =
 {
     0x41, 0x6E, 0x6B, 0x75, 0x72, 0x20, 0x4D, 0x61,
     0x75, 0x72, 0x79, 0x61
 };

static const BYTE rgbIV[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B
};

static const BYTE rgbAES128Key[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

BYTE origNonce[] = { 0,0,0,0,0,0,0,0,0,0,0,0 };



void PrintBytes(
    IN BYTE* pbPrintData,
    IN DWORD    cbDataLen)
{
    DWORD dwCount = 0;

    for (dwCount = 0; dwCount < cbDataLen; dwCount++)
    {
        printf("%02x", pbPrintData[dwCount]);

        //if (0 == (dwCount + 1) % 10) putchar('\n');
    }
    putchar('\n');

}

void encrypt()
{
    DWORD keySize = 16; // 256-bit key
    BYTE keyBuffer[16];

    BCRYPT_ALG_HANDLE       hAesAlg = NULL;
    BCRYPT_KEY_HANDLE       hKey = NULL;
    NTSTATUS                status = STATUS_UNSUCCESSFUL;
    DWORD                   cbCipherText = 0,
                            cbPlainText = 0,
                            cbData = 0,
                            cbKeyObject = 0,
                            cbBlockLen = 0,
                            cbBlob = 0;
    PBYTE                   pbCipherText = NULL,
                            pbPlainText = NULL,
                            pbKeyObject = NULL,
                            pbIV = NULL,
                            pbBlob = NULL;
    PBYTE authTag = NULL;
    NTSTATUS bcryptResult = 0;
    DWORD bytesDone = 0;

    // Open an algorithm handle.
    if (!NT_SUCCESS(status = BCryptOpenAlgorithmProvider(
        &hAesAlg,
        BCRYPT_AES_ALGORITHM,
        NULL,
        0)))
    {
        wprintf(L"**** Error 0x%x returned by BCryptOpenAlgorithmProvider\n", status);
        goto Cleanup;
    }

    if (!NT_SUCCESS(status = BCryptSetProperty(
        hAesAlg,
        BCRYPT_CHAINING_MODE,
        (PBYTE)BCRYPT_CHAIN_MODE_GCM,
        sizeof(BCRYPT_CHAIN_MODE_GCM),
        0)))
    {
        wprintf(L"**** Error 0x%x returned by BCryptSetProperty\n", status);
        goto Cleanup;
    }

    printf("BCRYPT_AES_ALGORITHM -> BCRYPT_CHAIN_MODE_GCM\n");

    // This tells us the length of the authentication tag:
    BCRYPT_AUTH_TAG_LENGTHS_STRUCT authTagLengths;
    bcryptResult = BCryptGetProperty(hAesAlg, BCRYPT_AUTH_TAG_LENGTH, (BYTE*)&authTagLengths, sizeof(authTagLengths), &bytesDone, 0);
    printf("Auth Tag Length (min): %d\n", authTagLengths.dwMinLength);
    printf("Auth Tag Length (max): %d\n", authTagLengths.dwMaxLength);

    // Calculate the size of the buffer to hold the KeyObject.
    if (!NT_SUCCESS(status = BCryptGetProperty(
        hAesAlg,
        BCRYPT_OBJECT_LENGTH,
        (PBYTE)&cbKeyObject,
        sizeof(DWORD),
        &cbData,
        0)))
    {
        wprintf(L"**** Error 0x%x returned by BCryptGetProperty\n", status);
        goto Cleanup;
    }

    // Allocate the key object on the heap.
    pbKeyObject = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbKeyObject);
    if (NULL == pbKeyObject)
    {
        wprintf(L"**** memory allocation failed\n");
        goto Cleanup;
    }

    // Calculate the block length for the IV.
    if (!NT_SUCCESS(status = BCryptGetProperty(
        hAesAlg,
        BCRYPT_BLOCK_LENGTH,
        (PBYTE)&cbBlockLen,
        sizeof(DWORD),
        &cbData,
        0)))
    {
        wprintf(L"**** Error 0x%x returned by BCryptGetProperty\n", status);
        goto Cleanup;
    }

    // Determine whether the cbBlockLen is not longer than the IV length.
    //if (cbBlockLen > sizeof(rgbIV))
    //{
       // wprintf(L"**** block length is longer than the provided IV length\n");
       // goto Cleanup;
    //}

    // Allocate a buffer for the IV. The buffer is consumed during the 
    // encrypt/decrypt process.
    pbIV = (PBYTE)HeapAlloc(GetProcessHeap(), 0, 12);
    if (NULL == pbIV)
    {
        wprintf(L"**** memory allocation failed\n");
        goto Cleanup;
    }

    memcpy(pbIV, rgbIV, 12);

    // Generate the key from supplied input key bytes.
    if (!NT_SUCCESS(
        /*status = BCryptGenerateSymmetricKey(
        hAesAlg,
        &hKey,
        pbKeyObject,
        cbKeyObject,
        (PBYTE)rgbAES128Key,
        sizeof(rgbAES128Key),
        0))*/
        status = BCryptGenRandom(hAesAlg, keyBuffer, keySize, BCRYPT_USE_SYSTEM_PREFERRED_RNG))
        )
    {
        wprintf(L"**** Error 0x%x returned by BCryptGenerateSymmetricKey\n", status);
        goto Cleanup;
    }

    
   


    // Save another copy of the key for later.
    if (!NT_SUCCESS(status = BCryptExportKey(
        hKey,
        NULL,
        BCRYPT_KEY_DATA_BLOB,
        NULL,
        0,
        &cbBlob,
        0)))
    {
        wprintf(L"**** Error 0x%x returned by BCryptExportKey\n", status);
        goto Cleanup;
    }

    // Allocate the buffer to hold the BLOB.
    pbBlob = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbBlob);
    if (NULL == pbBlob)
    {
        wprintf(L"**** memory allocation failed\n");
        goto Cleanup;
    }

    if (!NT_SUCCESS(status = BCryptExportKey(
        hKey,
        NULL,
        BCRYPT_KEY_DATA_BLOB,
        pbBlob,
        cbBlob,
        &cbBlob,
        0)))
    {
        wprintf(L"**** Error 0x%x returned by BCryptExportKey\n", status);
        goto Cleanup;
    }

    wprintf(L"Key Blob \n");
    PrintBytes(pbBlob, cbBlob);

    cbPlainText = sizeof(rgbPlaintext);
    pbPlainText = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbPlainText);
    if (NULL == pbPlainText)
    {
        wprintf(L"**** memory allocation failed\n");
        goto Cleanup;
    }

    memcpy(pbPlainText, rgbPlaintext, cbPlainText);

    //
    // Get the output buffer size.
    //
    if (!NT_SUCCESS(status = BCryptEncrypt(
        hKey,
        pbPlainText,
        cbPlainText,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        &cbCipherText,
        0)))
    {
        wprintf(L"**** Error 0x%x returned by BCryptEncrypt\n", status);
        goto Cleanup;
    }

    pbCipherText = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbCipherText);
    if (NULL == pbCipherText)
    {
        wprintf(L"**** memory allocation failed\n");
        goto Cleanup;
    }


    authTag = (PBYTE)HeapAlloc(GetProcessHeap(), 0, authTagLengths.dwMaxLength);
    if (NULL == pbCipherText)
    {
        wprintf(L"**** memory allocation failed\n");
        goto Cleanup;
    }

    // This must always be 96 bits (12 bytes):
    //const size_t GCM_NONCE_SIZE = 12;
   

    // This sets up our nonce and GCM authentication tag parameters:
    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO authInfo;
    BCRYPT_INIT_AUTH_MODE_INFO(authInfo);
    authInfo.pbNonce = (PUCHAR)&origNonce[0]; // A nonce is required for GCM
    authInfo.cbNonce = 12; // The size of the nonce is provided here
    authInfo.pbTag = &authTag[0]; // The buffer that will gain the authentication tag
    authInfo.cbTag = authTagLengths.dwMaxLength; // The size of the authentication tag
    authInfo.dwFlags = 0;

    // Use the key to encrypt the plaintext buffer.
    // For block sized messages, block padding will add an extra block.
    if (!NT_SUCCESS(status = BCryptEncrypt(
        hKey,
        pbPlainText,
        cbPlainText,
        &authInfo, 
        pbIV,
        12,
        pbCipherText,
        cbCipherText,
        &cbData,
        0)))
    {
        wprintf(L"**** Error 0x%x returned by BCryptEncrypt\n", status);
        goto Cleanup;
    }

    // Destroy the key and reimport from saved BLOB.
    if (!NT_SUCCESS(status = BCryptDestroyKey(hKey)))
    {
        wprintf(L"**** Error 0x%x returned by BCryptDestroyKey\n", status);
        goto Cleanup;
    }
    hKey = 0;

    wprintf(L"Plain Text \n");
    PrintBytes(pbPlainText, cbPlainText);

    wprintf(L"Cipher Text \n");
    PrintBytes(pbCipherText, cbCipherText);

    wprintf(L"Nonce Text \n");
    PrintBytes(origNonce, 12);

    wprintf(L"Auth Tag \n");
    PrintBytes(authTag, authTagLengths.dwMaxLength);



    if (pbPlainText)
    {
        HeapFree(GetProcessHeap(), 0, pbPlainText);
    }

    pbPlainText = NULL;

    // We can reuse the key object.
    memset(pbKeyObject, 0, cbKeyObject);

    // Reinitialize the IV because encryption would have modified it.
    memcpy(pbIV, rgbIV, cbBlockLen);



    if (!NT_SUCCESS(status = BCryptImportKey(
        hAesAlg,
        NULL,
        BCRYPT_KEY_DATA_BLOB,
        &hKey,
        pbKeyObject,
        cbKeyObject,
        pbBlob,
        cbBlob,
        0)))
    {
        wprintf(L"**** Error 0x%x returned by BCryptGenerateSymmetricKey\n", status);
        goto Cleanup;
    }



    //
    // Get the output buffer size.
    //
    if (!NT_SUCCESS(status = BCryptDecrypt(
        hKey,
        pbCipherText,
        cbCipherText,
        &authInfo,
        pbIV,
        12,
        NULL,
        0,
        &cbPlainText,
        0)))
    {
        wprintf(L"**** Error 0x%x returned by BCryptDecrypt\n", status);
        goto Cleanup;
    }

    pbPlainText = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbPlainText);
    if (NULL == pbPlainText)
    {
        wprintf(L"**** memory allocation failed\n");
        goto Cleanup;
    }

    if (!NT_SUCCESS(status = BCryptDecrypt(
        hKey,
        pbCipherText,
        cbCipherText,
        &authInfo,
        pbIV,
        12,
        pbPlainText,
        cbPlainText,
        &cbPlainText,
        0)))
    {


        wprintf(L"**** Error 0x%x returned by BCryptDecrypt\n", status);
        goto Cleanup;
    }


    if (0 != memcmp(pbPlainText, (PBYTE)rgbPlaintext, sizeof(rgbPlaintext)))
    {
        wprintf(L"Expected decrypted text comparison failed.\n");
        goto Cleanup;
    }

    wprintf(L"Decrypted Text \n");
    PrintBytes(pbPlainText, cbPlainText);

    wprintf(L"Success!\n");


Cleanup:

    if (hAesAlg)
    {
        BCryptCloseAlgorithmProvider(hAesAlg, 0);
    }

    if (hKey)
    {
        BCryptDestroyKey(hKey);
    }

    if (pbCipherText)
    {
        HeapFree(GetProcessHeap(), 0, pbCipherText);
    }

    if (pbPlainText)
    {
        HeapFree(GetProcessHeap(), 0, pbPlainText);
    }

    if (pbKeyObject)
    {
        HeapFree(GetProcessHeap(), 0, pbKeyObject);
    }

    if (pbIV)
    {
        HeapFree(GetProcessHeap(), 0, pbIV);
    }

}


