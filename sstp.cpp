#include "stdafx.h"
#define DONT_USE_SOCKET
#include "my-gists/ukagaka/sstp.hpp"
#include "my-gists/windows/SetIcon.h"
#include <windows.h>
#include "sstp.h"

//inline SSTP_link_n::SSTP_Direct_link_t linker(SSTP_link_n::SSTP_link_args_t{{L"Charset", L"UTF-8"}, {L"Sender", L"tama"}});

void On_tamaOpen(HWND hEdit, wstring ghost_path) {
	linker.setReplayHwnd(hEdit);
	WCHAR selfpath[MAX_PATH];
	GetModuleFileNameW(NULL, selfpath, MAX_PATH);
	auto info = linker.NOTYFY({ { L"Event", L"tamaOpen" },
								{ L"Reference0", std::to_wstring((size_t)hEdit) },
								{ L"Reference1", selfpath }
							  });
	if(info.has(L"Icon"))
		if(!SetIcon(hEdit, info[L"Icon"].c_str())) {
			wstring icofullpath = ghost_path + info[L"Icon"];
			SetIcon(hEdit, icofullpath.c_str());
		}
	if(info.has(L"Tittle"))
		SetWindowTextW(hEdit,info[L"Tittle"].c_str());
	/*
	auto &info_map = info.to_str_map();
	POINT wp{};
	SIZE  ws{};
	for(auto pair: info_map) {
		if(pair.first.starts_with(L"X-SSTP-PassThru-"))
			SetParameter();
	}
	*/
}

void On_tamaExit(HWND hEdit, wstring ghost_path) {
	auto info = linker.NOTYFY({ { L"Event", L"tamaExit" } });
}
