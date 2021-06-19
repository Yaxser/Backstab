#pragma once

#include <Windows.h>
#include <stdio.h>
#include <winternl.h>


#define STATUS_SUCCESS   ((NTSTATUS)0x00000000L)
#define MAX_BUF 2056 + 1 /* based on ProcExp max object name size */


/// <summary>
/// Standard error printing
/// </summary>
/// <param name="method">The method name that caused the error; be as specific as possible</param>
/// <returns></returns>
BOOL Error(LPSTR szMethod);
BOOL Info(LPSTR szMethod);
BOOL Success(LPSTR szMethod);


/// <summary>
/// a proxy to GetProcAddr
/// </summary>
/// <param name="LibraryName"></param>
/// <param name="ProcName"></param>
/// <returns></returns>
PVOID GetLibraryProcAddress(PSTR LibraryName, PSTR ProcName);


/// <summary>
/// Initializes all necessary NT addresses by calling GetProcAddress. If it fails, the program exit
/// because it won't function properly without these methods
/// </summary>
/// <returns></returns>
BOOL InitializeNecessaryNtAddresses();


/// <summary>
/// Converts narrow string to wide string
/// </summary>
/// <param name="szSource">narrow string to be converted</param>
/// <returns>wide string representation of given string</returns>
LPWSTR charToWChar(LPCSTR szSource);


