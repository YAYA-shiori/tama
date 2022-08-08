#include <windows.h>
#include "header_files/tama.h"
#include "header_files/resource.h"
#include "header_files/GhostStuff.hpp"
#include "my-gists/windows/LoadStringFromResource.h"
#include "my-gists/ukagaka/SSPpath.hpp"
#include "my-gists/ukagaka/ghost_path.hpp"
#include "my-gists/STL/replace_all.hpp"

LRESULT CALLBACK GhostSelectDlgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

void GhostSelection(HINSTANCE hInstance) {
	using namespace args_info;
	if(ghost_hwnd)
		goto link_to_ghost;
	if(fmobj.Update_info()) {
		auto ghostnum = fmobj.info_map.size();
		if(ghostnum == 0) {
			ShowWindow(hWnd, SW_HIDE);
			MessageBoxW(NULL, LoadCStringFromResource(IDS_ERROR_NONE_GHOST), LoadCStringFromResource(IDS_ERROR_TITLE), MB_ICONERROR | MB_OK);
			exit(EXIT_FAILURE);
		}
		else if(!ghost_link_to.empty()) {
			for(auto &i: fmobj.info_map) {
				HWND tmp_hwnd = (HWND)wcstoll(i.second[L"hwnd"].c_str(), nullptr, 10);
				if(i.second[L"name"] == ghost_link_to || i.second[L"fullname"] == ghost_link_to)
					ghost_hwnd = tmp_hwnd;
			}
			if(!ghost_hwnd)
				goto ask_if_ghost_running;
		}
		else if(ghostnum == 1) {
			//"Only one ghost was running.\n";
			ghost_hwnd = (HWND)wcstoll(fmobj.info_map.begin()->second.map[L"hwnd"].c_str(), nullptr, 10);
			if(has_corresponding_tama(ghost_hwnd))
				ghost_hwnd = NULL;
		}
		else {
			auto gsui = CreateDialog(hInstance, (LPCTSTR)IDD_GHOST_SELECT, hWnd, (DLGPROC)GhostSelectDlgProc);
			ShowWindow(gsui, SW_SHOW);
			ShowWindow(hWnd, SW_HIDE);
			for(auto &i: fmobj.info_map) {
				if(has_corresponding_tama(i.second[L"hwnd"]))
					continue;
				HWND tmp_hwnd = (HWND)wcstoll(i.second[L"hwnd"].c_str(), nullptr, 10);
				if(!tmp_hwnd)
					continue;
				wstring name;
				if(i.second[L"fullname"].size())
					name = i.second[L"fullname"];
				if(i.second[L"name"].size()) {
					if(!name.empty() && i.second[L"name"] != name)
						name += L'(' + i.second[L"name"] + L')';
					else
						name = i.second[L"name"];
				}
				auto index = SendDlgItemMessageW(gsui, IDC_GHOST_SELECT_LIST, LB_ADDSTRING, 0, (LPARAM)name.c_str());
				SendDlgItemMessageW(gsui, IDC_GHOST_SELECT_LIST, LB_SETITEMDATA, index, (LPARAM)tmp_hwnd);
			}
			auto index = SendDlgItemMessageW(gsui, IDC_GHOST_SELECT_LIST, LB_ADDSTRING, 0, (LPARAM)LoadCStringFromResource(IDS_GHOST_SELECT_START_WITH_OUT_GHOST));
			SendDlgItemMessageW(gsui, IDC_GHOST_SELECT_LIST, LB_SETITEMDATA, index, (LPARAM)(HWND)-1);
			MSG msg;
			while(GetMessage(&msg, NULL, 0, 0)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				if(ghost_hwnd)
					goto link_to_ghost;
			}
		}
	}
	else if(!ghost_link_to.empty()) {		//ssp not running 
	ask_if_ghost_running:
		if(IsSSPinstalled()) {
			if(MessageBoxW(NULL, LoadCStringFromResource(IDS_ASK_IF_GHOST_RUNNING), LoadCStringFromResource(IDS_ERROR_TITLE), MB_ICONERROR | MB_YESNO)
				== IDYES) {
			ask_if_ghost_running_cofirmed:
				std::wstring SSPpath = GetSSPpath();
				std::wstring SSPargs = ghost_link_to;
				SSPargs = L"/G \"" + replace_all(SSPargs, L"\"", L"\"\"") + L"\"";
				if((INT_PTR)ShellExecuteW(NULL, L"open", SSPpath.c_str(), SSPargs.c_str(), NULL, SW_SHOW)
					<= 32) {
					ShowWindow(hWnd, SW_HIDE);
					MessageBoxW(NULL, LoadCStringFromResource(IDS_ERROR_RUN_SSP_FAILED), LoadCStringFromResource(IDS_ERROR_TITLE), MB_ICONERROR | MB_OK);
					exit(EXIT_FAILURE);
				}
				//sleep until ghost is running
				size_t sleep_sec = 0;
				while(1) {
					Sleep(1000);
					++sleep_sec;
					if(sleep_sec > 90) {
						ShowWindow(hWnd, SW_HIDE);
						MessageBoxW(NULL, LoadCStringFromResource(IDS_ERROR_SSP_RUNNING_TIMEOUT), LoadCStringFromResource(IDS_ERROR_TITLE), MB_ICONERROR | MB_OK);
						exit(EXIT_FAILURE);
					}
					if(fmobj.Update_info()) {
						for(auto &i: fmobj.info_map) {
							HWND tmp_hwnd = (HWND)wcstoll(i.second[L"hwnd"].c_str(), nullptr, 10);
							if(i.second[L"name"] == ghost_link_to || i.second[L"fullname"] == ghost_link_to)
								ghost_hwnd = tmp_hwnd;
						}
						if(ghost_hwnd)
							goto link_to_ghost;
					}
				}
			}
			else	   //IDNO
				exit(EXIT_FAILURE);
		}
		ShowWindow(hWnd, SW_HIDE);
		MessageBoxW(NULL, (LoadCStringFromResource(IDS_ERROR_GHOST_NOT_FOUND_P1) + ghost_link_to + LoadCStringFromResource(IDS_ERROR_GHOST_NOT_FOUND_P2)).c_str(), LoadCStringFromResource(IDS_ERROR_TITLE), MB_ICONERROR | MB_OK);
		exit(EXIT_FAILURE);
	}
	else if(path_in_ghost_dir(selfpath)) {
		if(MessageBoxW(NULL, LoadCStringFromResource(IDS_SELF_IN_GHOST_DIR_ASK_IF_RUN_SSP), LoadCStringFromResource(IDS_INFO_TITLE), MB_ICONINFORMATION | MB_YESNO)
			== IDYES) {
			ghost_link_to = get_ghost_dir_name(selfpath);
			goto ask_if_ghost_running_cofirmed;
		}
	}
link_to_ghost:
	if(ghost_hwnd == (HWND)-1)
		ghost_hwnd = NULL;
	if(!ghost_hwnd) {
		tamamode = any;
		return;
	}
	else
		tamamode = specified_ghost;
	if(has_corresponding_tama(ghost_hwnd)) {
		ShowWindow(hWnd, SW_HIDE);
		MessageBoxW(NULL, LoadCStringFromResource(IDS_ERROR_GHOST_ALREADY_HAS_TAMA), LoadCStringFromResource(IDS_ERROR_TITLE), MB_ICONERROR | MB_OK);
		exit(EXIT_FAILURE);
	}
	linker.link_to_ghost(ghost_hwnd);
	//get ghost path
	fmobj.Update_info();
	for(auto &i: fmobj.info_map) {
		HWND tmp_hwnd = (HWND)wcstoll(i.second[L"hwnd"].c_str(), nullptr, 10);
		if(tmp_hwnd != ghost_hwnd)
			continue;
		ghost_path	 = i.second[L"ghostpath"] + L"ghost\\master\\";
		ghost_uid	 = i.second.ID;
		ghost_shiori = get_shiori_path(ghost_path);
		//
		has_shiori_file_info = !ghost_shiori.empty();
	}
}

LRESULT CALLBACK GhostSelectDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lp) {
	using namespace args_info;
	switch(message) {
	case WM_CTLCOLORDLG:
		return (INT_PTR)DlgBrush;
	case WM_INITDIALOG:
		SetFocus(GetDlgItem(hDlg, IDC_GHOST_SELECT_LIST));
		return FALSE;
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDOK: {
			auto index = SendDlgItemMessage(hDlg, IDC_GHOST_SELECT_LIST, LB_GETCURSEL, 0, 0);
			if(index == LB_ERR)
				return TRUE;
			ghost_hwnd = (HWND)SendDlgItemMessageW(hDlg, IDC_GHOST_SELECT_LIST, LB_GETITEMDATA, index, 0);
			EndDialog(hDlg, TRUE);
			if(!ghost_hwnd) {
				MessageBoxW(NULL, LoadCStringFromResource(IDS_ERROR_NULL_GHOST_HWND), LoadCStringFromResource(IDS_ERROR_TITLE), MB_ICONERROR | MB_OK);
				exit(EXIT_FAILURE);
			}
			return TRUE;
		}
		case IDCANCEL:
			EndDialog(hDlg, TRUE);
			exit(0);
		default:
			return FALSE;
		}
	default:
		return FALSE;
	}
}
