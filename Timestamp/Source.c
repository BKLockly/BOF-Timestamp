#include <stdio.h>
#include <Windows.h>
#include "beacon.h"

DECLSPEC_IMPORT BOOL WINAPI KERNEL32$SetFileTime(HANDLE, const FILETIME*, const FILETIME*, const FILETIME*);
DECLSPEC_IMPORT HANDLE WINAPI KERNEL32$CreateFileA(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
DECLSPEC_IMPORT BOOL WINAPI KERNEL32$CloseHandle(HANDLE);
DECLSPEC_IMPORT BOOL WINAPI KERNEL32$GetFileTime(HANDLE, LPFILETIME, LPFILETIME, LPFILETIME);
DECLSPEC_IMPORT DWORD WINAPI KERNEL32$GetLastError();

void go(char* buff, int len) {
    datap parser;
    char* sourceFile;
    char* targetFile;

    BeaconDataParse(&parser, buff, len);
    sourceFile = BeaconDataExtract(&parser, NULL);
    targetFile = BeaconDataExtract(&parser, NULL);

    if (!sourceFile || !targetFile) {
        BeaconPrintf(CALLBACK_ERROR, "[!] Error: Two file paths required\n");
        BeaconPrintf(CALLBACK_ERROR, "[-] Usage: inline-execute timestamp.o \"source_file\" \"target_file\"\n");
        return;
    }

    BeaconPrintf(CALLBACK_OUTPUT, "[-] Source: %s\n", sourceFile);
    BeaconPrintf(CALLBACK_OUTPUT, "[-] Target: %s\n", targetFile);

    HANDLE hSourceFile = KERNEL32$CreateFileA(
        sourceFile,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hSourceFile == INVALID_HANDLE_VALUE) {
        BeaconPrintf(CALLBACK_ERROR, "[!] Failed to open source file (Error: %d)\n", KERNEL32$GetLastError());
        return;
    }

    FILETIME ftCreation, ftLastAccess, ftLastWrite;
    if (!KERNEL32$GetFileTime(hSourceFile, &ftCreation, &ftLastAccess, &ftLastWrite)) {
        BeaconPrintf(CALLBACK_ERROR, "[!] Failed to get source timestamps (Error: %d)\n", KERNEL32$GetLastError());
        KERNEL32$CloseHandle(hSourceFile);
        return;
    }

    KERNEL32$CloseHandle(hSourceFile);

    HANDLE hTargetFile = KERNEL32$CreateFileA(
        targetFile,
        FILE_WRITE_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hTargetFile == INVALID_HANDLE_VALUE) {
        BeaconPrintf(CALLBACK_ERROR, "[!] Failed to open target file (Error: %d)\n", KERNEL32$GetLastError());
        return;
    }

    if (!KERNEL32$SetFileTime(hTargetFile, &ftCreation, &ftLastAccess, &ftLastWrite)) {
        BeaconPrintf(CALLBACK_ERROR, "[!] Failed to modify timestamps (Error: %d)\n", KERNEL32$GetLastError());
    }
    else {
        BeaconPrintf(CALLBACK_OUTPUT, "[+] Successfully modified timestamps\n");
    }

    KERNEL32$CloseHandle(hTargetFile);
}