#include "stdafx.h"

//更新检查用
#include <wininet.h>
#pragma comment(lib, "WinInet.lib")

wstring LoadStringFromResource(
	__in UINT		   stringID,
	__in_opt HINSTANCE instance = NULL);

//check update at https://github.com/nikolat/tama/releases with windows api
//if has update,open https://github.com/nikolat/tama/releases/latest
#define VERSION_STRING u8"v1.0.3.3"
void CheckUpdate() {
	#ifndef _DEBUG
	u8string tagname;
	{
		HINTERNET hInternet = InternetOpen(L"Tama", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if(hInternet == NULL) {
			return;
		}
		HINTERNET hConnect = InternetConnect(hInternet, L"api.github.com", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
		if(hConnect == NULL) {
			InternetCloseHandle(hInternet);
			return;
		}
		HINTERNET hRequest = HttpOpenRequest(hConnect, L"GET", L"/repos/nikolat/tama/releases/latest", NULL, NULL, NULL, INTERNET_FLAG_SECURE, 0);
		if(hRequest == NULL) {
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hInternet);
			return;
		}
		//download json
		HttpSendRequest(hRequest, NULL, 0, NULL, 0);
		DWORD	 dwSize = 0;
		u8string buff;
		InternetQueryDataAvailable(hRequest, &dwSize, 0, 0);
		buff.resize(dwSize);
		while(InternetReadFile(hRequest, buff.data(), buff.size(), &dwSize) && dwSize > 0) {
			buff.resize(buff.size() + dwSize);
		}
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		//parse json
		auto p = buff.find(u8"\"tag_name\":");
		if(p == u8string::npos)
			return;
		buff = buff.substr(p + 11);
		p	 = buff.find(u8"\"");
		if(p == u8string::npos)
			return;
		buff = buff.substr(p + 1);
		p	 = buff.find(u8"\"");
		if(p == u8string::npos)
			return;
		tagname = buff.substr(0, p);
	}
	if (tagname == VERSION_STRING) {
		return;
	}
	if(MessageBox(NULL, LoadStringFromResource(IDS_NEW_VERSION_AVAILABLE).c_str(), L"Tama", MB_YESNO) == IDYES)
		ShellExecute(NULL, L"open", L"https://github.com/nikolat/tama/releases/latest", NULL, NULL, SW_SHOW);
	#endif
}
