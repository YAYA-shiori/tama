#define _CRT_SECURE_NO_WARNINGS

#include "header_files/tama.h"
#include "header_files/GhostStuff.hpp"
#include "my-gists/windows/LoadStringFromResource.hpp"
#include "header_files/resource.h"
#include "header_files/ToolFunctions.hpp"
#include "header_files/UItools.hpp"

[[noreturn]] void LostGhostLink() {
	MessageBoxW(NULL, LoadCStringFromResource(IDS_ERROR_LOST_GHOST_LINK), LoadCStringFromResource(IDS_ERROR_TITLE), MB_ICONERROR | MB_OK);
	exit(EXIT_FAILURE);
}

void reload_shiori_of_baseware() {
	linker.SEND({{L"ID", ghost_uid},
				 {L"Script", L"\\![reload,shiori]"}});
	shiorimode = load_by_baseware;
}

void unload_shiori_of_baseware() {
	linker.SEND({{L"ID", ghost_uid},
				 {L"Script", L"\\![unload,shiori]"}});
	shioristaus = unloaded;
	shiorimode	= not_in_loading;
}

wstring get_shiori_path(wstring ghost_path) {
	auto descript_name = ghost_path + L"descript.txt";
	auto descript_f	   = _wfopen(descript_name.c_str(), L"rb");
	//
	CODEPAGE_n::CODEPAGE cp = CODEPAGE_n::CP_UTF8;
	char				 buf[2048];
	wstring				 line, s0, s1;
	if(descript_f) {
		while(fgets(buf, 2048, descript_f)) {
			line	 = CODEPAGE_n::MultiByteToUnicode(buf, cp);
			auto len = line.size();
			if(len && *line.rbegin() == L'\n')
				line.resize(--len);
			if(len && *line.rbegin() == L'\r')
				line.resize(--len);
			Split(line, s0, s1, L",");
			if(s0 == L"charset")
				cp = CODEPAGE_n::StringtoCodePage(s1.c_str());
			else if(s0 == L"shiori") {
				fclose(descript_f);
				return ghost_path + s1;
			}
		}
		fclose(descript_f);
	}
	return {};
}

void UpdateGhostModulestate() {
	if(ghost_uid.empty())
		return;
	fmobj.Update_info();
	auto shioristate	 = fmobj.info_map[ghost_uid].get_modulestate(L"shiori");
	auto shioristausback = shioristaus;
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


void CshioriErrorHandler(Cshiori::Error err) {
	using enum Cshiori::Error;
	wstring tmp = dllpath;
	switch(err) {
	case interface_load_not_found:
		tmp += L" : tama error TE0001 : Necessary interface \"load\" not found.";
		break;
	case interface_unload_not_found:
		tmp += L" : tama error TE0001 : Necessary interface \"unload\" not found.";
		break;
	case interface_request_not_found:
		tmp += L" : tama error TE0001 : Necessary interface \"request\" not found.";
		break;
	case interface_load_failed:
		tmp += L" : tama error TE0001 : Failed to load, unloading file...";
		break;
	case interface_unload_failed:
		tmp += L" : tama error TE0001 : unload returns failure";
		break;
	case dll_file_load_failed:
		tmp += L" : tama error TE0001 : dll file loading failure";
		break;
	case skip_unload_call_because_load_failed:
		tmp += L" : tama error TE0001 : Skip the unload call as load failed";
		break;
	case skip_unload_call_because_interface_unload_not_found:
		tmp += L" : tama error TE0001 : Skip the unload call as it was not found";
		break;
	default:
		tmp += L" : tama error TE0001 : Something fucked up.";
		break;
	}
	SetWindowTextW(hEdit, tmp.c_str());
}
void CshioriWarningHandler(Cshiori::Warning warn) {
	using enum Cshiori::Warning;
	switch(warn) {
	case interface_CI_check_not_found:
		return;//but who cares? tama does not use this interface.
	case interface_logsend_not_found: {
		wstring tmp = dllpath + L" : tama error TE0001 : Interface \"logsend\" not found.";
		SetWindowTextW(hEdit, tmp.c_str());
		return;
	}
	case logsend_failed: {
		wstring tmp = dllpath + L" : tama error TE0001 : Cannot set dll's logsend hwnd.";
		SetWindowTextW(hEdit, tmp.c_str());
		return;
	}
	default:
		return;//I don't care.
	}
}

void ExecRequest(const wchar_t *str) {
	SendMessage(hEdit, EM_SETSEL, -1, -1);
	// 実行
	auto retstr = shiori(str);
}

void ExecUnload() {
	SendMessage(hEdit, EM_SETSEL, -1, -1);

	shiori.Dounload();
	shiorimode = not_in_loading;
}

bool ExecReload() {
	ExecUnload();
	return ExecLoad();
}
