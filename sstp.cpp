#include "stdafx.h"
#define DONT_USE_SOCKET
#include "my-gists/ukagaka/sstp.hpp"
#include "my-gists/windows/SetIcon.h"
#include <windows.h>
#include "sstp.h"

//inline SSTP_link_n::SSTP_Direct_link_t linker(SSTP_link_n::SSTP_link_args_t{{L"Charset", L"UTF-8"}, {L"Sender", L"yatama"}});

void On_yatamaOpen(HWND hEdit) {
	linker.setReplayHwnd(hEdit);
	WCHAR selfpath[MAX_PATH];
	GetModuleFileNameW(NULL, selfpath, MAX_PATH);
	auto info = linker.NOTYFY({ { L"Event", L"yatamaOpen" },
								{ L"Reference0", std::to_wstring((size_t)hEdit) },
								{ L"Reference1", selfpath }
							  });
	if(info.has(L"Icon"))
		SetIcon(hEdit,info[L"Icon"].c_str());
	if(info.has(L"Tittle"))
		SetWindowTextW(hEdit,info[L"Tittle"].c_str());
}

void On_yatamaExit(HWND hEdit) {
	auto info = linker.NOTYFY({ { L"Event", L"yatamaExit" } });
}
