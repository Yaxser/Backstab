#include "common.h"
#include "Processes.h"
#include "Driverloading.h"

BOOL Error(LPSTR szMethod) {
	printf("%s: %d\n", szMethod, GetLastError());
	return FALSE;
}

PVOID GetLibraryProcAddress(LPSTR szLibraryName, LPSTR szProcName)
{
	return GetProcAddress(GetModuleHandleA(szLibraryName), szProcName);
}


BOOL InitializeNecessaryNtAddresses()
{
	_NtDuplicateObject =
		GetLibraryProcAddress("ntdll.dll", "NtDuplicateObject");

	_NtQueryObject =
		GetLibraryProcAddress("ntdll.dll", "NtQueryObject");

	_NtQuerySystemInformation =
		(fNtQuerySystemInformation)GetLibraryProcAddress("ntdll", "NtQuerySystemInformation");

	_RtlInitUnicodeString = 
		(fRtlInitUnicodeString)GetLibraryProcAddress("ntdll.dll", "RtlInitUnicodeString");
	
	_NtLoadDriver = 
		(fNtLoadDriver)GetLibraryProcAddress("ntdll", "NtLoadDriver");

	_NtUnLoadDriver = 
		(fNtUnLoadDriver)GetLibraryProcAddress("ntdll", "NtUnloadDriver");


	if (!_NtQueryObject || !_NtDuplicateObject || !_NtQuerySystemInformation || !_NtLoadDriver || !_NtUnLoadDriver)
	{
		return Error("InitializeNecessaryNtAddresses");
	}
	return TRUE;
}

LPWSTR charToWChar(LPCSTR szSource)
{
	size_t strlen = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, szSource, -1, NULL, 0);
	if (strlen == 0) { return NULL; }

	LPWSTR convertedString = (LPWSTR)calloc(strlen + 1, sizeof(WCHAR));
	if (!convertedString) { return NULL; }


	MultiByteToWideChar(CP_UTF8, 0, szSource, -1, convertedString, (int)strlen);
	return convertedString;
}
