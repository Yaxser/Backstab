#include "ProcExp.h"


HANDLE ConnectToProcExpDevice()
{
	hProcExpDevice = CreateFileA("\\\\.\\PROCEXP152", GENERIC_ALL, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hProcExpDevice == INVALID_HANDLE_VALUE)
		return NULL;

	return hProcExpDevice;
}


HANDLE DuplicateHandleOfProtectedProcess(DWORD dwPID, USHORT usHandle)
{
	HANDLE hProtectedProcess = ProcExpOpenProtectedProcess(dwPID);
	HANDLE ret = _DuplicateHandle(hProtectedProcess, usHandle);
	return ret;
}


HANDLE ProcExpOpenProtectedProcess(ULONGLONG ulPID)
{
	HANDLE hProtectedProcess = NULL;
	DWORD dwBytesReturned = 0;
	BOOL ret = FALSE;


	ret = DeviceIoControl(hProcExpDevice, IOCTL_OPEN_PROTECTED_PROCESS_HANDLE, (LPVOID)&ulPID, sizeof(ulPID),
		&hProtectedProcess,
		sizeof(HANDLE),
		&dwBytesReturned,
		NULL);

	
	if (dwBytesReturned == 0 || !ret)
	{
		printf("ProcExpOpenProtectedProcess.DeviceIoControl: %d\n", GetLastError());
		return NULL;
	}

	return hProtectedProcess;
}

BOOL ProcExpKillHandle(DWORD dwPID, ULONGLONG usHandle) {

	PVOID lpObjectAddressToClose = NULL;
	PROCEXP_DATA_EXCHANGE ctrl = { 0 };
	BOOL bRet = FALSE;

	
	/* find the object address */
	lpObjectAddressToClose = GetObjectAddressFromHandle(dwPID, (USHORT)usHandle);


	/* populate the data structure */
	ctrl.ulPID = dwPID;
	ctrl.ulSize = 0;
	ctrl.ulHandle = usHandle;
	ctrl.lpObjectAddress = lpObjectAddressToClose;

	/* send the kill command */

	bRet = DeviceIoControl(hProcExpDevice, IOCTL_CLOSE_HANDLE, (LPVOID)&ctrl, sizeof(PROCEXP_DATA_EXCHANGE), NULL,
		0,
		NULL,
		NULL);

	if (!bRet)
		return Error("ProcExpKillHandle.DeviceIoControl");


	return TRUE;
}


BOOL PrintProtectedHandleInformation(ULONGLONG ulPID, ULONGLONG ulProtectedHandle, PVOID lpObjectAddress) {

	PROCEXP_DATA_EXCHANGE data = { 0 };
	DWORD bytesReturned = 0;
	WCHAR szName[MAX_BUF] = { 0 };
	WCHAR szType[MAX_BUF] = { 0 };


	data.ulHandle = ulProtectedHandle;
	data.ulPID = ulPID;
	data.lpObjectAddress = lpObjectAddress;
	data.ulSize = 0;

	if (ProcExpGetObjectInformation(data, IOCTL_GET_HANDLE_NAME, szName)) {
		ProcExpGetObjectInformation(data, IOCTL_GET_HANDLE_TYPE, szType);
		printf("[%#llu] [%ws]: %ws\n", data.ulHandle, szType + 2, szName + 2);
	}

	return TRUE;
}


BOOL ProcExpGetObjectInformation(PROCEXP_DATA_EXCHANGE data, DWORD IOCTL, LPWSTR info) {

	DWORD dwBytesReturned = 0;
	BOOL bRet = FALSE;

	bRet = DeviceIoControl(hProcExpDevice, IOCTL, (LPVOID)&data, sizeof(PROCEXP_DATA_EXCHANGE), (LPVOID)info, MAX_BUF, &dwBytesReturned, NULL);
	if (!bRet)
		return Error("ProcExpGetObjectInformation.DeviceIoControl");


	if (dwBytesReturned == 8) // 8 bytes are returned when the handle is unnamed 
		return FALSE;
	
	
	return TRUE;
}
