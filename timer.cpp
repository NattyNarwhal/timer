// timer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

void ft2ul(LPFILETIME ft, PULARGE_INTEGER ul)
{
	ul->LowPart = ft->dwLowDateTime;
	ul->HighPart = ft->dwHighDateTime;
}

void ul2ft(PULARGE_INTEGER ul, LPFILETIME ft)
{
	ft->dwLowDateTime = ul->LowPart;
	ft->dwHighDateTime = ul->HighPart;
}

char *ftime2str(LPFILETIME ft)
{
	char str[1024], tmp[128];
	ZeroMemory(str, 1024);
	ZeroMemory(tmp, 128);
	unsigned _int64 ns, min, sec, remNs;
	ULARGE_INTEGER ulNs;
	ft2ul(ft, &ulNs);
	// each tick is 100ns
	ns = ulNs.QuadPart * 100;
	sec = ns / 1000000000; // 1,000,000,000;
	min = sec / 60;
	sec %= 60;
	remNs = ns % 1000000000;
	if (min) {
		sprintf(str, "%I64um", min);
	}
	if (remNs) {
		// unix time programs divide ns remainder by this; seems right
		sprintf(tmp, "%I64u.%I64us", sec, remNs / 10000000);
	} else {
		sprintf(tmp, "%I64us", sec);
	}
	strcat(str, tmp);
	return strdup(str);
}

TCHAR *tcscomb(int argc, TCHAR *argv[])
{
	TCHAR str[32768];
	ZeroMemory(str, 32768);
	for (int i = 0; i < argc; i++) {
		if (i)
			_tcscat(str, " ");
		_tcscat(str, argv[i]);
	}
	return _tcsdup(str);
}

int _tmain(int argc, TCHAR *argv[])
{
	BOOL ok;
	STARTUPINFO sinfo;
	PROCESS_INFORMATION pinfo;
	char *strReal, *strSys, *strUser, *combinedArgv;
	FILETIME beforeCreate, creation, ending, real, kernel, user;
	ULARGE_INTEGER ulCreate, ulEnd, ulReal;

#if NDEBUG
	if (argc < 2) {
		fputs("you must specify a command line\n", stderr);
		return 1;
	}
#endif

	combinedArgv = tcscomb(argc - 1, argv + 1);
	ZeroMemory(&sinfo, sizeof(sinfo));
	ZeroMemory(&pinfo, sizeof(pinfo));

	GetSystemTimeAsFileTime(&beforeCreate);
	ok = CreateProcess(NULL, combinedArgv, NULL, NULL, TRUE,
		0, NULL, NULL, &sinfo, &pinfo);
	if (!ok) {
		_ftprintf(stderr, _T("timer failed to create process of: %s\n"), argv[1]);
	}
	WaitForSingleObject(pinfo.hProcess, INFINITE);

	if (GetProcessTimes(pinfo.hProcess, &creation, &ending, &kernel, &user)) {
		// NT
		ft2ul(&creation, &ulCreate);
		ft2ul(&ending, &ulEnd);
		ulReal.QuadPart = ulEnd.QuadPart - ulCreate.QuadPart;
		ul2ft(&ulReal, &real);

		strReal = ftime2str(&real);
		strSys = ftime2str(&kernel);
		strUser = ftime2str(&user);
		printf("real %s sys %s user %s\n", strReal, strSys, strUser);
		free(strReal);
		free(strSys);
		free(strUser);
	} else {
		// 9x
		GetSystemTimeAsFileTime(&ending);
		ft2ul(&beforeCreate, &ulCreate);
		ft2ul(&ending, &ulEnd);
		ulReal.QuadPart = ulEnd.QuadPart - ulCreate.QuadPart;
		ul2ft(&ulReal, &real);

		strReal = ftime2str(&real);
		printf("real %s\n", strReal);
		free(strReal);
	}
	CloseHandle(pinfo.hProcess);
	CloseHandle(pinfo.hThread);

	return 0;
}

