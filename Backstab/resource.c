#include "resource.h"


/* adopted from: https://stackoverflow.com/questions/11388134/extract-file-from-resource-in-windows-module */
BOOL WriteResourceToDisk(LPWSTR path) {
	HGLOBAL     hgResHandle = NULL;
	HRSRC       hrRes = NULL;
	LPVOID		lpLock = NULL;
	DWORD       dwResourceSize = 0, dwBytesWritten = 0;
	HANDLE		hFile = NULL;
	BOOL		bRet;

	hrRes = FindResource(NULL, MAKEINTRESOURCE(RES_PROCEXP_BINARY), RT_RCDATA);
	if (!hrRes)
		return Error("FindResource");

	hgResHandle = LoadResource(NULL, hrRes);
	if (!hgResHandle)
		return Error("LoadResource");

	lpLock = (LPVOID)LockResource(hgResHandle);
	if (!lpLock)
		return Error("LockResource");

	dwResourceSize = SizeofResource(NULL, hrRes);
	if (dwResourceSize == 0)
		return Error("SizeOfResource");

	hFile = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return Error("WriteResourceToDisk.CreateFile");

	bRet = WriteFile(hFile, lpLock, dwResourceSize, &dwBytesWritten, NULL);
	if (!bRet)
		return Error("WriteResourceToDisk.WriteFile");

	CloseHandle(hFile);
	FreeResource(hgResHandle);

	return TRUE;
}