#pragma once

#include "common.h"


#define STATUS_IMAGE_ALREADY_LOADED 0xC000010E
#define STATUS_OBJECT_NAME_COLLISION 0xC0000035

/* Functions To Be Dynamically Resolved */
typedef void (WINAPI* fRtlInitUnicodeString)(PUNICODE_STRING, PCWSTR);
typedef NTSTATUS(*fNtLoadDriver)(IN PUNICODE_STRING DriverServiceName);
typedef NTSTATUS(*fNtUnLoadDriver)(IN PUNICODE_STRING DriverServiceName);

/* Global Variables For Dynamically Resolved Functions */
fRtlInitUnicodeString _RtlInitUnicodeString;
fNtLoadDriver _NtLoadDriver;
fNtUnLoadDriver _NtUnLoadDriver;

/// <summary>
/// Loading a given driver without creating a service
/// </summary>
/// <param name="szPath"></param>
/// <param name="szServiceName"></param>
/// <returns></returns>
BOOL LoadDriver(LPWSTR szPath, LPWSTR szServiceName);

/// <summary>
/// Unload driver
/// </summary>
/// <param name="szServiceName"></param>
/// <returns></returns>
BOOL UnloadDriver(LPWSTR szDriverPath, LPWSTR szServiceName);


/// Delete driver from Disk
BOOL WriteResourceToDisk(LPWSTR szPath);