#include "stdafx.h"
#include "LoadStringFromResource.h"
#include <windows.h>

const wchar_t *LoadCStringFromResource(
	__in UINT		   stringID,
	__in_opt HINSTANCE instance) {
	WCHAR *pBuf = NULL;

	int len = LoadStringW(
		instance,
		stringID,
		reinterpret_cast<LPWSTR>(&pBuf), 0);

	if(len)
		return pBuf;
	else
		return L"";
}
std::wstring LoadStringFromResource(
	__in UINT		   stringID,
	__in_opt HINSTANCE instance) {
	WCHAR *pBuf = NULL;

	int len = LoadStringW(
		instance,
		stringID,
		reinterpret_cast<LPWSTR>(&pBuf), 0);

	if(len)
		return std::wstring(pBuf, pBuf + len);
	else
		return L"";
}