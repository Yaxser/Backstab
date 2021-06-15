#include "Processes.h"
#include "ProcExp.h"

PSYSTEM_HANDLE_INFORMATION ReAllocateHandleInfoTableSize(ULONG ulTable_size, PSYSTEM_HANDLE_INFORMATION handleInformationTable) {

	HANDLE hHeap = GetProcessHeap();
	BOOL ret = HeapFree(hHeap, HEAP_NO_SERIALIZE, handleInformationTable); //first call handleInformationTable will be NULL, which is OK according to the documentation

	handleInformationTable =
		(PSYSTEM_HANDLE_INFORMATION)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, ulTable_size);
	return handleInformationTable;
}


PSYSTEM_HANDLE_INFORMATION GetHandleInformationTable() {

	NTSTATUS status;
	PSYSTEM_HANDLE_INFORMATION handleInformationTable = NULL;

	ULONG ulSystemInfoLength = sizeof(SYSTEM_HANDLE_INFORMATION) + (sizeof(SYSTEM_HANDLE_TABLE_ENTRY_INFO) * 100) - 2300;

	//getting the address of NtQuerySystemInformation procedure, using the predefined type fNtQuerySystemInformation


	handleInformationTable = ReAllocateHandleInfoTableSize(ulSystemInfoLength, handleInformationTable);
	while ((status = _NtQuerySystemInformation(
		CONST_SYSTEM_HANDLE_INFORMATION,
		handleInformationTable,
		ulSystemInfoLength,
		NULL
	)) == STATUS_INFO_LENGTH_MISMATCH)
	{
		handleInformationTable = ReAllocateHandleInfoTableSize(ulSystemInfoLength *= 2, handleInformationTable);
	}


	if (!NT_SUCCESS(status))
		printf("ReAllocateHandleInfoTableSize: %d", GetLastError());


	return handleInformationTable;
}



VOID ListProcessHandles(HANDLE hProcess) {

	DWORD		PID = GetProcessId(hProcess);
	ULONG		returnLenght = 0;
	SYSTEM_HANDLE_TABLE_ENTRY_INFO handleInfo = { 0 };
	PSYSTEM_HANDLE_INFORMATION handleTableInformation = NULL;

	handleTableInformation = GetHandleInformationTable();

	for (ULONG i = 0; i < handleTableInformation->HandleCount; i++)
	{
		handleInfo = handleTableInformation->Handles[i];

		if (handleInfo.ProcessId == PID) //meaning that the handle is within our process of interest
		{
			//	printf_s("Handle 0x%x at 0x%p, PID: %x\n", handleInfo.Handle, handleInfo.Object, handleInfo.ProcessId);
			/*	if ((handleInfo.GrantedAccess == 0x0012019f)
					|| (handleInfo.GrantedAccess == 0x001a019f)
					|| (handleInfo.GrantedAccess == 0x00120189)
					|| (handleInfo.GrantedAccess == 0x00100000)) {
					continue;
				}*/
			PrintProtectedHandleInformation(PID, handleInfo.Handle, handleInfo.Object);
		}
	}
}


PVOID GetObjectAddressFromHandle(DWORD dwPID, USHORT usTargetHandle)
{
	ULONG ulReturnLenght = 0;

	PSYSTEM_HANDLE_INFORMATION handleTableInformation = GetHandleInformationTable();

	for (ULONG i = 0; i < handleTableInformation->HandleCount; i++)
	{
		SYSTEM_HANDLE_TABLE_ENTRY_INFO handleInfo = handleTableInformation->Handles[i];

		if (handleInfo.ProcessId == dwPID) //meaning that the handle is within our process of interest
		{
			if (handleInfo.Handle == usTargetHandle)
			{
				return handleInfo.Object;
			}
		}
	}
	return NULL;
}

BOOL GetProcessPIDFromName(LPWSTR szProcessName, PDWORD lpPID) {
	HANDLE			hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	BOOL			bRet = FALSE;
	DWORD			dwMatchCount = 0;
	PROCESSENTRY32	pe32;


	if (hSnapshot == INVALID_HANDLE_VALUE)
		return Error("CreateToolhelp32Snapshot");

	pe32.dwSize = sizeof(PROCESSENTRY32);

	bRet = Process32First(hSnapshot, &pe32);
	if (!bRet)
		return Error("GetProcessNameFromPID.Process32First");


	do {
		if (wcscmp(szProcessName, pe32.szExeFile) == 0)
		{
			dwMatchCount++;
			*lpPID = pe32.th32ProcessID;
		}
	} while (Process32Next(hSnapshot, &pe32));


	if (dwMatchCount > 1)
	{
		*lpPID = 1;
		return FALSE;
	}

	if (dwMatchCount == 0)
	{
		*lpPID = 2;
		return FALSE;
	}

	CloseHandle(hSnapshot);
	return TRUE;
}


HANDLE _DuplicateHandle(HANDLE hSrcProcess, USHORT usHandleValue) {

	HANDLE hTargetProcess = NULL;

	_NtDuplicateObject(
		hSrcProcess,
		(HANDLE)usHandleValue,
		GetCurrentProcess(),
		&hTargetProcess,
		0,
		FALSE,
		0
	);


	if (hTargetProcess == NULL)
		printf("failed to duplicate handle: %d\n", GetLastError());

	return hTargetProcess;
}

VOID KillProcessHandles(HANDLE hProcess) {

	DWORD dwPID = GetProcessId(hProcess);
	ULONG ulReturnLenght = 0;

	//allocating memory for the SYSTEM_HANDLE_INFORMATION structure in the heap

	PSYSTEM_HANDLE_INFORMATION handleTableInformation = GetHandleInformationTable();

	for (ULONG i = 0; i < handleTableInformation->HandleCount; i++)
	{
		SYSTEM_HANDLE_TABLE_ENTRY_INFO handleInfo = handleTableInformation->Handles[i];

		if (handleInfo.ProcessId == dwPID) //meaning that the handle is within our process of interest
		{
			/* Check if the process is already killed every 15 closed handles (otherwise we'll keep trying to close handles that are already closed) */
			if (i % 15 == 0)
			{
				DWORD dwProcStatus = 0;
				GetExitCodeProcess(hProcess, &dwProcStatus);
				if (dwProcStatus != STILL_ACTIVE)
				{
					return;
				}			
			}
			ProcExpKillHandle(dwPID, handleInfo.Handle);
		}
	}
}

/********** Below code is not used in production but kept for learning purposes ***********/

//BOOL _GetObjectType(PHANDLE phObject, LPWSTR szType) {
//
//	POBJECT_TYPE_INFORMATION objectTypeInfo = { 0 };
//	NTSTATUS status = 0;
//
//
//	objectTypeInfo = (POBJECT_TYPE_INFORMATION)malloc(0x1000);
//	if (!objectTypeInfo)
//		return Error("_GetObjectType.malloc");
//
//	status = _NtQueryObject(*phObject, CONST_OBJECT_TYPE_INFORMATION, objectTypeInfo, 0x1000, NULL);
//
//	if (status != STATUS_SUCCESS)
//	{
//		free(objectTypeInfo);
//		return Error("GetObjectType.NtQueryObject");
//	}
//
//	wcscpy_s(szType, MAX_BUF, objectTypeInfo->Name.Buffer);
//	free(objectTypeInfo);
//	return TRUE;
//}
//
//
//BOOL GetObjectName(PHANDLE hObject, LPWSTR szName) {
//
//	PVOID lpObjectNameInfo = NULL;
//	UNICODE_STRING usObjectName = { 0 };
//	ULONG returnLength = 0;
//	NTSTATUS status;
//
//	lpObjectNameInfo = malloc(0x1000);
//	if (!lpObjectNameInfo)
//		return Error("GetObjectName.malloc");
//
//
//	status = _NtQueryObject(*hObject, CONST_OBJECT_NAME_INFORMATION, lpObjectNameInfo, 0x1000, &returnLength);
//	if (status != STATUS_SUCCESS) //could be insufficient size so we have to try again
//	{
//		PVOID lpTempHolder = lpObjectNameInfo;
//		lpObjectNameInfo = realloc(lpTempHolder, returnLength);
//		if (!lpObjectNameInfo) //out of memory
//		{
//			return Error("GetObjectName.realloc");
//		}
//		//size is reallocated based on the returned length, try again
//		status = _NtQueryObject(*hObject, CONST_OBJECT_NAME_INFORMATION, lpObjectNameInfo, returnLength, NULL);
//	}
//
//	if (status != STATUS_SUCCESS) //in this case the failure is not caused by the size
//	{
//		free(lpObjectNameInfo);
//		return Error("GetObjectName._NtQueryObject");
//	}
//
//	/* Cast our buffer into an UNICODE_STRING. */
//	usObjectName = *(PUNICODE_STRING)lpObjectNameInfo;
//
//	if (usObjectName.Length)
//	{
//		wcscpy_s(szName, MAX_BUF, usObjectName.Buffer);
//	}
//	else
//	{
//		wcscpy_s(szName, MAX_BUF, L"unnamed");
//		return FALSE; //will use this to skip printing unnamed handles
//	}
//
//	free(lpObjectNameInfo);
//	return TRUE;
//}
//
//

//
//HANDLE GetHandleToProcessByPID(DWORD pid) {
//	HANDLE hProcess;
//	hProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, pid);
//	return hProcess;
//}


//HANDLE _DuplicateHandle(HANDLE hSrcProcess, USHORT usHandleValue) {
//
//	HANDLE hTargetProcess = NULL;
//
//	_NtDuplicateObject(
//		hSrcProcess,
//		(HANDLE)usHandleValue,
//		GetCurrentProcess(),
//		&hTargetProcess,
//		0,
//		FALSE,
//		0
//	);
//
//
//	if (hTargetProcess == NULL)
//	{
//		printf("failed to duplicate handle: %d\n", GetLastError());
//	}
//
//	return hTargetProcess;
//}
//
//BOOLEAN GetProcessName(DWORD processID, LPWSTR szName)
//{
//	BOOLEAN bResult;
//
//	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
//
//	bResult = GetProcessImageFileName(hProcess, szName, MAX_PATH) != 0;
//
//	return bResult;
//}

//VOID PrintHandleInformation(HANDLE hContainingProcess, USHORT usHandleValue) {
//
//	HANDLE dupHandle = NULL;
//
//	dupHandle = _DuplicateHandle(hContainingProcess, usHandleValue);
//
//	if (dupHandle == NULL)
//	{
//		return;
//	}
//
//	WCHAR type[MAX_BUF], name[MAX_BUF];
//
//
//	if (GetObjectName(&dupHandle, name) && _GetObjectType(&dupHandle, type))
//	{
//		printf("[%#x]: [%ws] %ws\n", usHandleValue, type, name);
//	}
//	CloseHandle(dupHandle);
//}