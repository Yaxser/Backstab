#include "Driverloading.h"

/*
Adopted from: https://github.com/GitMirar/DriverLoader/blob/master/DriverLoader/DriverLoader.cpp
*/



BOOL SetRegistryValues(LPWSTR szPath, LPWSTR szServiceName) {

	HKEY hKey = NULL;
	WCHAR regPath[MAX_PATH] = { 0 };
	WCHAR driverPath[MAX_PATH] = { 0 };
	LSTATUS status = -1;
	DWORD dwData = 0, dwDisposition = 0;

	/* create the registry path string */
	_snwprintf_s(regPath, MAX_PATH, _TRUNCATE, L"System\\CurrentControlSet\\Services\\%ws", szServiceName);
	_snwprintf_s(driverPath, MAX_PATH, _TRUNCATE, L"%ws%ws", L"\\??\\", szPath);


	status = RegCreateKeyExW(HKEY_LOCAL_MACHINE, regPath, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
	if (status) {
		printf("SetRegistryValues.RegCreateKeyExA: %d\n", status);
		return FALSE;
	}


	status = RegSetValueEx(hKey, L"Type", 0, REG_DWORD, (BYTE*)&dwData, sizeof(DWORD));
	if (status) {
		printf("SetRegistryValues.RegSetValueEx.Type: %d\n", status);
		return FALSE;
	}

	status = RegSetValueEx(hKey, L"ErrorControl", 0, REG_DWORD, (BYTE*)&dwData, sizeof(DWORD));
	if (status) {
		printf("SetRegistryValues.RegSetValueEx.ErrorControl: %d\n", status);
		return FALSE;
	}

	status = RegSetValueEx(hKey, L"Start", 0, REG_DWORD, (BYTE*)&dwData, sizeof(DWORD));
	if (status) {
		printf("SetRegistryValues.RegSetValueEx.Start: %d\n", status);
		return FALSE;
	}

	status = RegSetValueEx(hKey, L"ImagePath", 0, REG_SZ, (const BYTE*)driverPath, (DWORD)(sizeof(wchar_t) * (wcslen(driverPath)+1)));
	if (status) {
		printf("SetRegistryValues.RegSetValueEx.ImagePath: %d\n", status);
		return FALSE;
	}

	return TRUE;
}




BOOL EnablePrivilege(LPCWSTR lpPrivilegeName)
{
	TOKEN_PRIVILEGES tpPrivilege;
	HANDLE hToken;

	tpPrivilege.PrivilegeCount = 1;
	tpPrivilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!LookupPrivilegeValueW(NULL, lpPrivilegeName,
		&tpPrivilege.Privileges[0].Luid))
		return Error("EnablePrivilege.LookupPrivilegeValueW");

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
		return Error("EnablePrivilege.OpenProcessToken");

	if (!AdjustTokenPrivileges(hToken, FALSE, &tpPrivilege, sizeof(tpPrivilege) ,NULL, NULL)) {
		CloseHandle(hToken);
		return Error("EnablePrivilege.AdjustTokenPrivileges");
	}

	CloseHandle(hToken);
	return TRUE;
}


BOOL DeleteRegistryKey(LPWSTR szServiceName) {
	WCHAR szRegistryPath[MAX_PATH] = { 0 };
	LSTATUS status;

	_snwprintf_s(szRegistryPath, MAX_PATH, _TRUNCATE, L"System\\CurrentControlSet\\Services\\%ws", szServiceName);
	status = RegDeleteKeyExW(HKEY_LOCAL_MACHINE, szRegistryPath, KEY_WOW64_64KEY, 0);

	if (status) {
		printf("[OpSec] could not remove service registry key: %d\n", status);
		return FALSE;
	}
	return TRUE;
}

BOOL LoadDriver(LPWSTR szPath, LPWSTR szServiceName) {
	
	UNICODE_STRING usDriverServiceName = {0};
	WCHAR szNtRegistryPath[MAX_PATH] = { 0 };
	WCHAR szRegistryPath[MAX_PATH] = { 0 };
	NTSTATUS ret;

	if (!EnablePrivilege(L"SeLoadDriverPrivilege")) {
		return FALSE;
	}


	if (!SetRegistryValues(szPath, szServiceName))
	{
		return Error("NtUnloadDriver.SetRegistryKeyValues");
	}

	
	_snwprintf_s(szNtRegistryPath, MAX_PATH, _TRUNCATE, L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\%ws", szServiceName);
	_RtlInitUnicodeString(&usDriverServiceName, szNtRegistryPath);

	ret = _NtLoadDriver(&usDriverServiceName);
	if (ret != STATUS_SUCCESS && ret != STATUS_IMAGE_ALREADY_LOADED && ret != STATUS_OBJECT_NAME_COLLISION) {
		printf("NtLoadDriver: %x\n", ret);
		return FALSE;
	}

//	DeleteRegistryKey(szServiceName); //don't care that much if it fails
	
	return TRUE;
}



BOOL UnloadDriver(LPWSTR szPath, LPWSTR szServiceName) {

	UNICODE_STRING usDriverServiceName = { 0 };
	WCHAR szRegistryPath[MAX_PATH] = { 0 };
	NTSTATUS ret;

	//if (!SetRegistryValues(szPath, szServiceName))
	//{
	//	return FALSE;
	//}

	_snwprintf_s(szRegistryPath, MAX_PATH, _TRUNCATE, L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\%ws", szServiceName);
	_RtlInitUnicodeString(&usDriverServiceName, szRegistryPath);

	ret = _NtUnLoadDriver(&usDriverServiceName);
	if (ret != STATUS_SUCCESS) {
		printf("Error : NtUnLoadDriver: %x\n", ret);
		DeleteRegistryKey(szServiceName);
		return FALSE;
	}

	DeleteRegistryKey(szServiceName);
//	printf("[+] Driver unloaded successfully\n");
	return TRUE;
}
