﻿#define _CRT_SECURE_NO_WARNINGS

#include "header_files/tama.h"
#include "header_files/GhostStuff.hpp"
#include "my-gists/windows/LoadStringFromResource.hpp"
#include "my-gists/STL/string_add_string_view.hpp"
#include "header_files/resource.h"
#include "header_files/ToolFunctions.hpp"
#include "header_files/UItools.hpp"

[[noreturn]] void LostGhostLink() noexcept {
	MessageBoxW(NULL, LoadCStringFromResource(IDS_ERROR_LOST_GHOST_LINK), LoadCStringFromResource(IDS_ERROR_TITLE), MB_ICONERROR | MB_OK);
	exit(EXIT_FAILURE);
}

void reload_shiori_of_baseware() {
	linker.SEND({{L"ID", ghost_uid},
				 {L"Script", L"\\![reload,shiori]"}});
	shioristaus = unloaded;//wait for UpdateGhostModulestate
	shiorimode = load_by_baseware;
}

void unload_shiori_of_baseware() {
	linker.SEND({{L"ID", ghost_uid},
				 {L"Script", L"\\![unload,shiori]"}});
	if(shioristaus != critical)
		shioristaus = unloaded;
	shiorimode	= not_in_loading;
}

void UpdateGhostModulestate() {
	if(ghost_uid.empty())
		return;
	fmobj.Update_info();
	auto shioristate	 = fmobj.info_map[ghost_uid].get_modulestate(L"shiori");
	const auto shioristausback = shioristaus;
	allow_file_drug		 = 0;
	if(shioristate == L"running") {
		shioristaus = running;
		shiorimode	= load_by_baseware;
		if(shioristausback == critical) {
			allow_file_drug = 0;
			if(!dllpath.empty())
				ExecUnload();
		}
	}
	else if(shioristate == L"critical") {
		shioristaus = critical;
		shiorimode	= load_by_baseware;
		if(shioristausback == running) {
			if(has_shiori_file_info && !disable_auto_transfer_shiori_ownership) {
				unload_shiori_of_baseware();
				dllpath = ghost_shiori;
				if(!ExecReload())
					allow_file_drug = 1;
			}
			else {
				allow_file_drug = 1;
			}
		}
	}
	else if(shioristate.empty()) {
		if(shioristaus != critical)
			shioristaus = unloaded;
		shiorimode	= not_in_loading;
	}
}

bool ExecLoad(void) {
	has_fatal_or_error = 0;
	shiorimode		   = load_by_tama;
	// テキスト全クリア
	SetWindowText(hEdit, L"");
	SetFontShapeInit(F_DEFAULT);

	shiori.SetTo(dllpath.c_str());
	// load　失敗したらdllパスは空にする
	if(!shiori.All_OK()) {
		dllpath.clear();
		shiorimode = not_in_loading;
		return 0;
	}
	// logsend
	if(!shiori.is_logsend_ok()) {
		shiorimode = not_in_loading;
		return 0;
	}

	if(tamamode == specified_ghost && shioristaus == critical && !disable_auto_transfer_shiori_ownership) {
		if(!has_fatal_or_error) {
			ExecUnload();
			reload_shiori_of_baseware();
		}
	}
	return 1;
}

void WriteShioriError(int id,UINT stringResourceID) {
	//id最少4个字符，不够的话前面补0
	auto idstr = to_wstring(id);
	if(idstr.size()<4)
		idstr.insert(0, 4 - idstr.size(), L'0');

	wstring tmp = dllpath + L" : tama error TE" + idstr + L" : " + LoadStringViewFromResource(stringResourceID) + L"\r\n";
	WriteText(tmp, F_ERROR);
}
void CshioriErrorHandler(Cshiori::Error err) {
	using enum Cshiori::Error;
	wstring tmp = dllpath;
	switch(err) {
	case interface_load_not_found:
		WriteShioriError(1, IDS_interface_load_not_found);
		break;
	case interface_unload_not_found:
		WriteShioriError(2, IDS_interface_unload_not_found);
		break;
	case interface_request_not_found:
		WriteShioriError(3, IDS_interface_request_not_found);
		break;
	case interface_load_failed:
		WriteShioriError(4, IDS_interface_load_failed);
		break;
	case interface_unload_failed:
		WriteShioriError(5, IDS_interface_unload_failed);
		break;
	case dll_file_load_failed:
		WriteShioriError(0, IDS_dll_file_load_failed);
		break;
	case skip_unload_call_because_load_failed:
		WriteShioriError(6, IDS_skip_unload_call_because_load_failed);
		break;
	case skip_unload_call_because_unload_not_found:
		WriteShioriError(7, IDS_skip_unload_call_because_unload_not_found);
		break;
	default:
		WriteShioriError(9999, IDS_unknown_Cshiori_error);
		break;
	}
}
void CshioriWarningHandler(Cshiori::Warning warn) {
	using enum Cshiori::Warning;
	switch(warn) {
	case interface_CI_check_not_found:
		return;//but who cares? tama does not use this interface.
	case interface_logsend_not_found:
		WriteShioriError(8, IDS_interface_logsend_not_found);
		break;
	case logsend_failed:
		WriteShioriError(9, IDS_logsend_failed);
		break;
	default:
		return;//I don't care.
	}
}

void ExecRequest(const wchar_t *str) {
	SendMessage(hEdit, EM_SETSEL, -1, -1);
	// 実行
	auto retstr = shiori(str);
}

void ExecUnload() noexcept {
	SendMessage(hEdit, EM_SETSEL, -1, -1);

	shiori.Dounload();
	shiorimode = not_in_loading;
}

bool ExecReload() {
	ExecUnload();
	return ExecLoad();
}
