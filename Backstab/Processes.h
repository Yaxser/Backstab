#pragma once

#include "common.h"
#include <Psapi.h>
#include <tlhelp32.h>

/* https://github.com/outflanknl/Ps-Tools/blob/master/Src/Outflank-PsX-rDLL/PsX/ReflectiveDll.cpp */

/*
typedef   void      (*FunctionFunc)  ( );
//         ^                ^         ^
//     return type      type name  arguments
src: https://stackoverflow.com/questions/4295432/typedef-function-pointer/4295495
*/

#define STATUS_INFO_LENGTH_MISMATCH 0xc0000004
#define CONST_SYSTEM_HANDLE_INFORMATION 16
#define CONST_OBJECT_BASIC_INFORMATION 0
#define CONST_OBJECT_NAME_INFORMATION 1
#define CONST_OBJECT_TYPE_INFORMATION 2


typedef NTSTATUS(WINAPI* fNtQuerySystemInformation)(
	SYSTEM_INFORMATION_CLASS SystemInformationClass,
	PVOID SystemInformation,
	ULONG SystemInformationLength,
	PULONG ReturnLength);

typedef NTSTATUS(NTAPI* fNtDuplicateObject)(
	HANDLE SourceProcessHandle,
	HANDLE SourceHandle,
	HANDLE TargetProcessHandle,
	PHANDLE TargetHandle,
	ACCESS_MASK DesiredAccess,
	ULONG Attributes,
	ULONG Options
	);


typedef NTSTATUS(NTAPI* fNtQueryObject)(
	HANDLE ObjectHandle,
	ULONG ObjectInformationClass,
	PVOID ObjectInformation,
	ULONG ObjectInformationLength,
	PULONG ReturnLength
	);



typedef enum _POOL_TYPE
{
	NonPagedPool,
	PagedPool,
	NonPagedPoolMustSucceed,
	DontUseThisType,
	NonPagedPoolCacheAligned,
	PagedPoolCacheAligned,
	NonPagedPoolCacheAlignedMustS
}
POOL_TYPE, * PPOOL_TYPE;


typedef struct _OBJECT_TYPE_INFORMATION
{
	UNICODE_STRING Name;
	ULONG TotalNumberOfObjects;
	ULONG TotalNumberOfHandles;
	ULONG TotalPagedPoolUsage;
	ULONG TotalNonPagedPoolUsage;
	ULONG TotalNamePoolUsage;
	ULONG TotalHandleTableUsage;
	ULONG HighWaterNumberOfObjects;
	ULONG HighWaterNumberOfHandles;
	ULONG HighWaterPagedPoolUsage;
	ULONG HighWaterNonPagedPoolUsage;
	ULONG HighWaterNamePoolUsage;
	ULONG HighWaterHandleTableUsage;
	ULONG InvalidAttributes;
	GENERIC_MAPPING GenericMapping;
	ULONG ValidAccess;
	BOOLEAN SecurityRequired;
	BOOLEAN MaintainHandleCount;
	USHORT MaintainTypeList;
	POOL_TYPE PoolType;
	ULONG PagedPoolUsage;
	ULONG NonPagedPoolUsage;
} OBJECT_TYPE_INFORMATION, * POBJECT_TYPE_INFORMATION;


typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO
{
	ULONG ProcessId;
	BYTE ObjectTypeNumber;
	BYTE Flags;
	USHORT Handle;
	PVOID Object;			//Pointer to the object, the object resides in kernel space
	ACCESS_MASK GrantedAccess;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO, * PSYSTEM_HANDLE_TABLE_ENTRY_INFO;

typedef struct _SYSTEM_HANDLE_INFORMATION
{
	ULONG HandleCount;
	SYSTEM_HANDLE_TABLE_ENTRY_INFO Handles[ANYSIZE_ARRAY];
}  SYSTEM_HANDLE_INFORMATION, * PSYSTEM_HANDLE_INFORMATION;

typedef struct _THREAD_CONTEXT
{
	HANDLE hDup;
	char   szFileName[MAX_PATH];
}
THREAD_CONTEXT, * PTHREAD_CONTEXT;

typedef struct
{
	char   szFileName[MAX_PATH];
	char   szProcessName[MAX_PATH];
	HANDLE FileHandle;
	ULONG  ProcessId;
}
HANDLE_INFO, * PHANDLE_INFO;




/* Functions To Be Dynamically Resolved */
fNtQueryObject _NtQueryObject;
fNtDuplicateObject _NtDuplicateObject;
fNtQuerySystemInformation _NtQuerySystemInformation;




/// <summary>
/// Duplicates a handle from remote process to the current process. It is the calling process responsibility to provide a handle
/// opened with access mask PROCESS_DUP_HANDLE and to transorm the hex value to a USHORT.
/// </summary>
/// <param name="hSrcProcess">Handle to source process opened with access mask PROCESS_DUP_HANDLE</param>
/// <param name="usHandleValue">The value of the handle to be copied to the current process</param>
/// <returns></returns>
HANDLE _DuplicateHandle(HANDLE hSrcProcess, USHORT usHandleValue);


/* Used To Show The User Info About The Enumerated Handle */
VOID PrintHandleInformation(HANDLE hContainingProcess, USHORT usHandleValue);


/// <summary>
/// Enumerate system handles to find handles associated with a given process
/// The handle passed must have PROCESS_DUP_HANDLE 
/// </summary>
/// <param name="hProcess"></param>
VOID ListProcessHandles(HANDLE hProcess);



/// <summary>
/// Retrieves the process PID based on a given name. On success, the PID is stored in lpPID. If there are multiple processes with the same name it returns false and sets lpPID to 1. If name not found, it sets lpPID to 2
/// </summary>
/// <param name="szProcessName"> The process name to be found </param>
/// <param name="lpPID">on success stores the target process PID, on failure stores error indicator</param>
/// <returns></returns>
BOOL GetProcessPIDFromName(LPWSTR szProcessName, PDWORD lpPID);


/// <summary>
/// Given a PID and handle, it will return the handle's object address in kernel. This is used by ProcExp because some functions
/// require the object address with the handle and the containing PID
/// </summary>
/// <param name="PID">PID of the process containing the handle</param>
/// <param name="targetHandle">target handle</param>
/// <returns></returns>
PVOID GetObjectAddressFromHandle(DWORD dwPID, USHORT usTargetHandle);


/// <summary>
/// a proxy to NtQuerySystemInformation
/// </summary>
/// <returns></returns>
PSYSTEM_HANDLE_INFORMATION GetHandleInformationTable();

PSYSTEM_HANDLE_INFORMATION ReAllocateHandleInfoTableSize(ULONG ulTable_size, PSYSTEM_HANDLE_INFORMATION handleInformationTable);


/// <summary>
/// Enumerate system to find all handles related to a process and pass these handles to ProcExpKillHandle
/// </summary>
/// <param name="hProcess">Protected process handle</param>
VOID KillProcessHandles(HANDLE hProcess);




/*********** Processes Not Used In Production Code ************/
HANDLE GetHandleToProcessByPID(DWORD pid);
VOID ListProcessHandles(HANDLE hProcess);

/// <summary>
/// Provides string representation of the object type of a given handle
/// </summary>
/// <param name="phObject">Pointer to the handle to be inspected </param>
/// <param name="type">on success, contains the string representation of the object type</param>
BOOL _GetObjectType(PHANDLE phObject, LPWSTR szType); /* requires access to the protected handle */

/// <summary>
/// Provides the object name of a given handle or "unnamed" if it is an unnamed handle
/// </summary>
/// <param name="phObject">Pointer to the handle to be inspected </param>
/// <param name="type">on success, contains the object name</param>
BOOL GetObjectName(PHANDLE phObject, LPWSTR szName);	/* requires access to the protected handle */