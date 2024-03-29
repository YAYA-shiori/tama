﻿#include "header_files/tama.h"
#include "header_files/GhostStuff.hpp"
#include "header_files/Parameter.hpp"
#include "header_files/resource.h"
#include "header_files/UItools.hpp"
#include "my-gists/windows/SetIcon.h"
#include "my-gists/windows/LoadStringFromResource.hpp"

void On_tamaOpen(HWND hWnd, wstring ghost_path) {
	tamaOpen_called = 1;
	linker.setReplayHwnd(hWnd);
	WCHAR selfpath[MAX_PATH];
	GetModuleFileNameW(NULL, selfpath, MAX_PATH);
	auto info = linker.NOTYFY({{L"Event", L"tamaOpen"},
							   {L"Reference0", std::to_wstring((size_t)hWnd)},
							   {L"Reference1", selfpath}});
	{
		ICON_INFO_t icon_info = GetIcon(hWnd);
		if(info.has(L"Icon")) {
			auto hIcon = LoadIconWithBasePath(ghost_path, info[L"Icon"]);
			if(!hIcon)
				WriteText(LoadStringFromResource(IDS_ERROR_ICON_NOT_FOUND) + info[L"Icon"], F_ERROR);
			else {
				icon_info.hIcon		 = hIcon;
				icon_info.hIconSmall = hIcon;
			}
		}
		if(info.has(L"SmallIcon")) {
			auto hIcon = LoadIconWithBasePath(ghost_path, info[L"SmallIcon"]);
			if(!hIcon)
				WriteText(LoadStringFromResource(IDS_ERROR_ICON_NOT_FOUND) + info[L"SmallIcon"], F_ERROR);
			else
				icon_info.hIconSmall = hIcon;
		}
		SetIcon(hWnd, icon_info);
	}
	if(info.has(L"Title"))
		SetWindowTextW(hWnd, info[L"Title"].c_str());

	auto &info_map = info.get_params();
	POINT wpos{};
	SIZE  wsz{};
	for(auto &pair: info_map) {
		if(pair.name.starts_with(L"X-SSTP-PassThru-"))
			SetParameter(pair.name.substr(16), pair.var, wpos, wsz);
	}
	if(wsz.cx)
		MoveWindow(hWnd, wpos.x, wpos.y, wsz.cx, wsz.cy, TRUE);
	SetMyBkColor(bkcol);
	SetDlgFont(GetDlgItem(hDlgWnd, IDC_SEND_REQUEST_RICHEDIT));
	SetDlgBkColor(GetDlgItem(hDlgWnd, IDC_SEND_REQUEST_RICHEDIT), bkcol);
	ShowWindow(hWnd, SW_HIDE);
	ShowWindow(hWnd, SW_SHOW);
	SetFontShapeInit(F_DEFAULT);
}

void On_tamaExit(HWND hWnd, wstring ghost_path) {
	auto info = linker.NOTYFY({{L"Event", L"tamaExit"}});
}
