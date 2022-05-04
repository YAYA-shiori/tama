#include "stdafx.h"

//更新检查用
#include <wininet.h>
#pragma comment(lib, "WinInet.lib")

wstring LoadStringFromResource(
	__in UINT		   stringID,
	__in_opt HINSTANCE instance = NULL);

//check update at https://github.com/nikolat/tama/releases with windows api
//if has update,open https://github.com/nikolat/tama/releases/latest
#define VERSION_STRING u8"v1.0.3.1"
void CheckUpdate() {
	#ifndef _DEBUG
	HINTERNET hInternet = InternetOpen(L"Tama", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hInternet == NULL) {
		return;
	}
	HINTERNET hConnect = InternetConnect(hInternet, L"api.github.com", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
	if (hConnect == NULL) {
		InternetCloseHandle(hInternet);
		return;
	}
	HINTERNET hRequest = HttpOpenRequest(hConnect, L"GET", L"/repos/nikolat/tama/releases/latest", NULL, NULL, NULL, INTERNET_FLAG_SECURE, 0);
	if (hRequest == NULL) {
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		return;
	}
	//download json
	HttpSendRequest(hRequest, NULL, 0, NULL, 0);
	DWORD dwSize = 0;
	char *pBuffer = NULL;
	u8string tagname;
	do {
		InternetQueryDataAvailable(hRequest, &dwSize, 0, 0);
		if (dwSize == 0)
			break;
		auto tmp = (char *)realloc(pBuffer, dwSize + 1);
		if (tmp == NULL)
			free(pBuffer);
		pBuffer = tmp;
		if (pBuffer == NULL)
			break;
		DWORD dwRead = 0;
		InternetReadFile(hRequest, pBuffer, dwSize, &dwRead);
		pBuffer[dwRead] = '\0';
	} while (0);
	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);
	if (dwSize == 0 || pBuffer == NULL) {
		return;
	}
	pBuffer[dwSize] = '\0';
	auto p = strstr(pBuffer, "\"tag_name\":");
	if (p == NULL) {
		free(pBuffer);
		return;
	}
	p += 11;
	p = strstr(pBuffer, "\"");
	if (p == NULL) {
		free(pBuffer);
		return;
	}
	p++;
	auto p2 = strstr(p, "\"");
	if (p2 == NULL) {
		free(pBuffer);
		return;
	}
	tagname.assign(p, p2);
	free(pBuffer);
	if (tagname == VERSION_STRING) {
		return;
	}
	if(MessageBox(NULL, LoadStringFromResource(IDS_NEW_VERSION_AVAILABLE).c_str(), L"Tama", MB_YESNO) == IDYES)
		ShellExecute(NULL, L"open", L"https://github.com/nikolat/tama/releases/latest", NULL, NULL, SW_SHOW);
	#endif
}
