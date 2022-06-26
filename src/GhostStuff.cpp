#define _CRT_SECURE_NO_WARNINGS

#include "tama.h"
#include "GhostStuff.hpp"
#include "my-gists/windows/LoadStringFromResource.h"
#include "resource.h"
#include "ToolFunctions.hpp"
#include "UItools.hpp"

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
			if(has_shiori_file_info) {
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
		wstring tmp = dllpath + L" : tama error TE0001 : Cannot load dll.";
		SetWindowTextW(hEdit, tmp.c_str());
		shiorimode = not_in_loading;
		return 0;
	}
	// logsend
	if(!shiori.is_logsend_ok()) {
		wstring tmp = dllpath + L" : tama error TE0002 : Cannot set dll's logsend hwnd.";
		SetWindowTextW(hEdit, tmp.c_str());
		shiorimode = not_in_loading;
		return 0;
	}

	if(tamamode == specified_ghost && shioristaus == critical) {
		if(!has_fatal_or_error) {
			ExecUnload();
			reload_shiori_of_baseware();
		}
	}
	return 1;
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
