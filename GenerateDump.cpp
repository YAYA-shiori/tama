#include "stdafx.h"
#include "Windows.h"
#include "DbgHelp.h"

wstring LoadStringFromResource(
	__in UINT		   stringID,
	__in_opt HINSTANCE instance = NULL);

int GenerateDump(PEXCEPTION_POINTERS pExceptionPointers) {
	HANDLE hDumpFile = CreateFileW(L"Dump.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hDumpFile == INVALID_HANDLE_VALUE) {
		return EXCEPTION_EXECUTE_HANDLER;
	}

	typedef BOOL(WINAPI * MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
											 CONST PMINIDUMP_EXCEPTION_INFORMATION	 ExceptionParam,
											 CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
											 CONST PMINIDUMP_CALLBACK_INFORMATION	 CallbackParam);
	HMODULE hDbgHelp = LoadLibraryW(L"DbgHelp.dll");
	if(hDbgHelp == NULL) {
		return EXCEPTION_EXECUTE_HANDLER;
	}
	MINIDUMPWRITEDUMP pfnMiniDumpWriteDump = (MINIDUMPWRITEDUMP)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
	if(!pfnMiniDumpWriteDump) {
		return EXCEPTION_EXECUTE_HANDLER;
	}

	MINIDUMP_EXCEPTION_INFORMATION mdei;
	mdei.ThreadId		   = GetCurrentThreadId();
	mdei.ExceptionPointers = pExceptionPointers;
	mdei.ClientPointers	   = FALSE;

	BOOL bRet = pfnMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &mdei, NULL, NULL);
	CloseHandle(hDumpFile);
	MessageBoxW(NULL, LoadStringFromResource(IDS_ERROR_UNEXPECTED_EXCEPTION).c_str(), LoadStringFromResource(IDS_ERROR_TITLE).c_str(), MB_ICONERROR | MB_OK);
	return EXCEPTION_EXECUTE_HANDLER;
}
LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS lpExceptionInfo) {
	if(IsDebuggerPresent()) {
		return EXCEPTION_CONTINUE_SEARCH;
	}
	return GenerateDump(lpExceptionInfo);
}
void InstallExceptionFilter() {
	SetUnhandledExceptionFilter(ExceptionFilter);
}
