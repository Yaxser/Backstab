#pragma once

#include "common.h"
#include "Processes.h"

#define IOCTL_CLOSE_HANDLE 2201288708
#define IOCTL_OPEN_PROTECTED_PROCESS_HANDLE 2201288764
#define IOCTL_GET_HANDLE_NAME 2201288776
#define IOCTL_GET_HANDLE_TYPE 2201288780



typedef struct _ioControl
{
	ULONGLONG ulPID;
	PVOID lpObjectAddress;
	ULONGLONG ulSize;
	ULONGLONG ulHandle;
} PROCEXP_DATA_EXCHANGE, *PPROCEXP_DATA_EXCHANGE;


/// <summary>
/// Returns a handle to a protected process which is powerful enough to allow duplication of the protected process handles
/// This method does NOT work on ANTI-MALWARE protected processes
/// </summary>
/// <param name="PID"></param>
/// <param name="handle"></param>
/// <returns></returns>
HANDLE DuplicateHandleOfProtectedProcess(DWORD dwPID, USHORT usHandle);


/// <summary>
/// Provides a handle with access mask PROCESS_DUP_HANDLE
/// </summary>
/// <param name="PID">PID for target process </param>
/// <returns>Handle with PROCESS_DUP_HANDLE access right</returns>
HANDLE ProcExpOpenProtectedProcess(ULONGLONG ulPID);



/// <summary>
/// Converts the user (hex representation of handle) to ULONG
/// </summary>
/// <param name="szHandle">user input string, can be either "9FC" or "0x9FC"</param>
/// <returns>ULONG representation of the given string</returns>
ULONGLONG ConvertInputHandleToULONG(LPSTR szHandle);


/// <summary>
/// Kills a protected process handle
/// </summary>
/// <param name="PID">PID of protected process</param>
/// <param name="usHandle">handle to be killed</param>
/// <returns></returns>
BOOL ProcExpKillHandle(DWORD PID, ULONGLONG usHandle);


/// <summary>
/// Connects to ProcExp device for further communication, since the device name is set by the driver, there is nothing to customize here
/// </summary>
/// <returns></returns>
BOOL ConnectToProcExpDevice();


/// <summary>
/// Prints the object type and name of a handle of a protected process to screen
/// </summary>
/// <param name="PID">Protected process PID</param>
/// <param name="hProtectedHandle">Handle of interest</param>
/// <param name="ObjectAddress">The address of the handle of interest</param>
/// <returns></returns>
BOOL PrintProtectedHandleInformation(ULONGLONG ulPID, ULONGLONG ulProtectedHandle, PVOID lpObjectAddress);

/// <summary>
/// communicates with ProcExp to retrieve protected handle information
/// </summary>
/// <param name="data"></param>
/// <param name="IOCTL">type of requested information, refers to either object type or object handle</param>
/// <param name="info">string to store the info of the protected handle</param>
/// <returns></returns>
BOOL ProcExpGetObjectInformation(PROCEXP_DATA_EXCHANGE data, DWORD dwIOCTL, LPWSTR szInfo);

HANDLE hProcExpDevice;