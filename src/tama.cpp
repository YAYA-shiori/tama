#include "my-gists/codepage.hpp"
#include "my-gists/windows/LoadStringFromResource.h"

#include <Richedit.h>

#include "resource.h"
#include "tama.h"
#include "GhostStuff.hpp"
#include "ToolFunctions.hpp"
#include "Events.hpp"
#include "UItools.hpp"
#include "Parameter.hpp"

using namespace std;

void			 GhostSelection(HINSTANCE hInstance);
ATOM			 TamaMainWindowClassRegister(HINSTANCE hInstance);
BOOL			 InitInstance(HINSTANCE, int);
BOOL			 InitSendRequestDlg(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK SendRequestDlgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK GhostSelectDlgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

bool has_corresponding_tama(wstring ghost_hwnd_str) {
	auto hMutex = ::CreateMutexW(NULL, FALSE, (L"scns_task" + ghost_hwnd_str).c_str());
	if(hMutex == NULL)
		return true;
	if(GetLastError() == ERROR_ALREADY_EXISTS) {
		::CloseHandle(hMutex);
		return true;
	}
	else {
		::CloseHandle(hMutex);
		return false;
	}
}
bool has_corresponding_tama(HWND ghost_hwnd) {
	return has_corresponding_tama(to_wstring((size_t)ghost_hwnd));
}

void ArgsHandling() {
	LPWSTR *argv;
	int		_targc;
	using namespace args_info;

	argv		= CommandLineToArgvW(GetCommandLineW(), &_targc);
	size_t argc = _targc;

	if(argc)
		selfpath = argv[0];
	
	if(argc != 1) {
		size_t i = 1;
		while(i < argc) {
			if(!lstrcmpW(argv[i], L"-g")) {		  //ghost
				i++;
				if(i < argc)
					ghost_link_to = argv[i];
			}
			else if(!lstrcmpW(argv[i], L"-gh")) {		//ghost hwnd
				i++;
				if(i < argc)
					ghost_hwnd = (HWND)wcstoll(argv[i], nullptr, 10);
			}
			i++;
		}
	}
}

void InstallExceptionFilter();
// Winmain
int APIENTRY WinMain(
	_In_ HINSTANCE	   hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR		   lpCmdLine,
	_In_ int		   nShowCmd) {
	InstallExceptionFilter();
	ArgsHandling();

	//new thread for CheckUpdate, using windows api
	auto checkupdate_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CheckUpdate, NULL, 0, NULL);
	
	GhostSelection(hInstance);
	if(!ghost_uid.empty())
		shiorimode = load_by_baseware;
	else
		shiorimode = not_in_loading;

	using namespace args_info;
	hMutex = ::CreateMutexW(NULL, FALSE, (L"scns_task" + to_wstring((size_t)ghost_hwnd)).c_str());
	if(hMutex == NULL)
		return FALSE;
	if(GetLastError() == ERROR_ALREADY_EXISTS) {
		::CloseHandle(hMutex);
		return FALSE;
	}

	if(ghost_hwnd)
		szWindowClass = CHECK_TOOL_WCNAME "_Targeted";

	MSG	   msg;
	HACCEL hAccelTable;

	szTitle = LoadStringFromResource(IDS_APP_TITLE, hInstance);
	TamaMainWindowClassRegister(hInstance);

	if(!InitInstance(hInstance, nShowCmd))
		return FALSE;
	if(!InitSendRequestDlg(hInstance))
		return FALSE;

	hAccelTable = LoadAcceleratorsW(hInstance, szWindowClass);

	ShowWindow(hWnd, SW_SHOW);

	if(tamamode == specified_ghost) {
		UpdateGhostModulestate();
		if(shioristaus == running)
			On_tamaOpen(hWnd, ghost_path);
		bool has_log = GetWindowTextLength(hEdit);
		if(!has_log) {
			if((shioristaus == unloaded) && fmobj.info_map[ghost_uid].map.contains(L"modulestate"))
				SetWindowTextW(hEdit, LoadCStringFromResource(IDS_TARGET_GHOST_SHIORI_UNLOADED));
			else
				SetWindowTextW(hEdit, LoadCStringFromResource(IDS_EVENT_DEF_REMINDER));
			allow_file_drug = 1;
		}

		ghost_status_watcher = SetTimer(hWnd, IDT_CHOST_STATUS_WATCHER, 7000, (TIMERPROC)NULL);		  // 7-second interval & no timer callback
	}
	while(GetMessage(&msg, NULL, 0, 0)) {
		if(!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	if(tamamode == specified_ghost && shioristaus == running)
		On_tamaExit(hWnd, ghost_path);
	
	if(hMutex != NULL)
		::CloseHandle(hMutex);

	if(checkupdate_thread)
		WaitForSingleObject(checkupdate_thread, INFINITE);

	return msg.wParam;
}

ATOM TamaMainWindowClassRegister(HINSTANCE hInstance) {
	// ウィンドウクラス登録

	WNDCLASSEX wcex{};
	wcex.cbSize		   = sizeof(wcex);
	wcex.style		   = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc   = (WNDPROC)WndProc;
	wcex.cbClsExtra	   = 0;
	wcex.cbWndExtra	   = 0;
	wcex.hInstance	   = hInstance;
	wcex.hIcon		   = LoadIcon(hInstance, (LPCTSTR)IDI_DRAGFILES);
	wcex.hCursor	   = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName  = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm	   = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
	// ウィンドウ作成と表示

	hInst = hInstance;
	RECT WindowRect;
	GetWindowRect(::GetDesktopWindow(), &WindowRect);
	hWnd = CreateWindowW(szWindowClass, szTitle.c_str(),
						 WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
						 WindowRect.right >> 1, WindowRect.bottom >> 1, WindowRect.right >> 1, WindowRect.bottom >> 1, NULL, NULL, NULL /*hInstance*/, NULL);
	if(!hWnd)
		return FALSE;
	shiori.set_logsend_hwnd(hWnd);

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	DragAcceptFiles(hWnd, TRUE);

	return TRUE;
}

BOOL InitSendRequestDlg(HINSTANCE hInstance) {
	reqshow = SW_HIDE;
	hDlgWnd = CreateDialog(hInstance, (LPCTSTR)IDD_SEND_REQUEST, hWnd, (DLGPROC)SendRequestDlgProc);
	if(hDlgWnd == NULL)
		return FALSE;
	ShowWindow(hDlgWnd, reqshow);
	RECT WindowRect;
	GetWindowRect(::GetDesktopWindow(), &WindowRect);
	RECT drect;
	GetWindowRect(hDlgWnd, &drect);
	MoveWindow(hDlgWnd, (WindowRect.right >> 1) - 32, (WindowRect.bottom >> 1) + 64,
			   drect.right - drect.left, drect.bottom - drect.top, TRUE);

	return TRUE;
}

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
			if(!ghost_hwnd) {
				ShowWindow(hWnd, SW_HIDE);
				MessageBoxW(NULL, (LoadCStringFromResource(IDS_ERROR_GHOST_NOT_FOUND_P1) + ghost_link_to + LoadCStringFromResource(IDS_ERROR_GHOST_NOT_FOUND_P2)).c_str(), LoadCStringFromResource(IDS_ERROR_TITLE), MB_ICONERROR | MB_OK);
				exit(EXIT_FAILURE);
			}
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

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	// メッセージプロシージャ

	static HINSTANCE hRtLib;

	switch(message) {
	case WM_CTLCOLORDLG:
		return (INT_PTR)DlgBrush;
	case WM_CREATE: {
		POINT wpos;
		SIZE  wsz;
		// パラメータ設定
		SetParameter(wpos, wsz);
		if(wsz.cx && wsz.cy)
			if(wpos.x != LONG_MIN && wpos.y != LONG_MIN)
				MoveWindow(hWnd, wpos.x, wpos.y, wsz.cx, wsz.cy, TRUE);
			else
				SendMessage(hWnd, WM_SIZE, wsz.cx, wsz.cy);
		
		// リッチエディットコントロール作成
		hRtLib		  = LoadLibrary(L"RICHED32.DLL");
		hEdit		  = CreateWindowEx(WS_EX_CLIENTEDGE, L"RICHEDIT", L"",
									   WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE |
										   WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY,
									   0, 0, 0, 0, hWnd, (HMENU)1, hInst, NULL);
		DWORD dwEvent = SendMessage(hEdit, EM_GETEVENTMASK, 0, 0) | ENM_KEYEVENTS | ENM_MOUSEEVENTS;
		SendMessage(hEdit, EM_SETEVENTMASK, 0, (LPARAM)dwEvent);
		SendMessage(hEdit, EM_EXLIMITTEXT, 0, (LPARAM)TEXTMAX);
		// フォントシェープ設定
		SetMyBkColor(bkcol);
		SetFontShapeInit(F_DEFAULT);
		break;
	}
	case WM_COPYDATA: {
		COPYDATASTRUCT *cds = (COPYDATASTRUCT *)lParam;
		if(cds->dwData >= 0 && cds->dwData < F_NUMBER && receive) {
			// メッセージ表示更新　NT系はunicodeのまま、9x系はMBCSへ変換して更新
			if(cds->cbData > 0) {
				// 更新
				wstring logbuf;
				logbuf.resize(cds->cbData);
				wcscpy_s(logbuf.data(), (size_t)cds->cbData, (wchar_t*)cds->lpData);

				static OSVERSIONINFO osi	   = {sizeof(osi)};
				static bool			 osiiniter = GetVersionEx(&osi);

				if(osi.dwPlatformId == VER_PLATFORM_WIN32_NT) {
					EOS(logbuf.size());
					SetFontShape(cds->dwData);
					SendMessageW(hEdit, EM_REPLACESEL, (WPARAM)0, (LPARAM)logbuf.c_str());
				}
				else {
					string mstr = CODEPAGE_n::UnicodeToMultiByte(logbuf, CODEPAGE_n::CP_ACP);
					if(!mstr.empty()) {
						EOS(mstr.size());
						SetFontShape(cds->dwData);
						SendMessageA(hEdit, EM_REPLACESEL, (WPARAM)0, (LPARAM)mstr.c_str());
					}
				}
				if(!has_fatal_or_error && (cds->dwData == F_FATAL || cds->dwData == F_ERROR))
					has_fatal_or_error = 1;
				// AlertOnWarning
				if(AlertOnWarning && F_was_warning_or_above(cds->dwData) && shiorimode == load_by_baseware) {
					static FLASHWINFO wfinfo{sizeof(wfinfo), NULL, FLASHW_ALL | FLASHW_TIMERNOFG, 13, 0};
					wfinfo.hwnd = hWnd;
					FlashWindowEx(&wfinfo);
					LPCWSTR boxtitle;
					auto	icontype = MB_ICONERROR;
					switch(cds->dwData) {
					case F_FATAL:
						boxtitle = L"ghost fatal error";
						break;
					case F_ERROR:
						boxtitle = L"ghost error";
						break;
					case F_WARNING:
						boxtitle = L"ghost warning";
						icontype  = MB_ICONWARNING;
						break;
					default:
						boxtitle = NULL;
					}
					MessageBoxW(NULL, logbuf.c_str(), boxtitle, icontype | MB_OK);
				}
			}
		}
		else if(cds->dwData == F_NUMBER) {
			// 最後のメッセージ
		}
		// 文字コード設定
		else if(cds->dwData == E_SJIS)
			shiori.SetCodePage(CODEPAGE_n::CP_SJIS);
		else if(cds->dwData == E_UTF8)
			shiori.SetCodePage(CODEPAGE_n::CP_UTF8);
		else if(cds->dwData == E_DEFAULT)
			shiori.SetCodePage(CODEPAGE_n::CP_ACP);
		else if(cds->dwData == dwDataOfDirectSSTP)
			linker.DirectSSTPprocess(cds);
		break;
	}
	case WM_DROPFILES: {
		wstring tmpdllpath;
		HDROP	hDrop = (HDROP)wParam;
		if(allow_file_drug) {
			tmpdllpath.resize(MAX_PATH);
			if(::DragQueryFileW(hDrop, 0xFFFFFFFF, tmpdllpath.data(), MAX_PATH)) {
				// ドロップされたファイル名を取得
				::DragQueryFileW(hDrop, 0, tmpdllpath.data(), MAX_PATH);
				// dllパス更新
				b_dllpath = dllpath;
				dllpath	  = tmpdllpath;
				// 实行
				ExecLoad();
			}
		}
		else {
			MessageBoxW(NULL, LoadCStringFromResource(IDS_ERROR_DRAGGING_FILE_IS_NOT_ALLOWED), LoadCStringFromResource(IDS_ERROR_TITLE), MB_ICONERROR | MB_OK);
		}
		::DragFinish(hDrop);
		break;
	}
	case WM_NOTIFY:
		switch(((NMHDR *)lParam)->code) {
		case EN_MSGFILTER: {
			MSGFILTER *pmf = (MSGFILTER *)lParam;
			if(pmf->msg == WM_RBUTTONDOWN) {
				HMENU hMenu, hSubMenu;
				POINT pt{};
				// ポップアップメニュー表示
				hMenu	 = LoadMenu(hInst, L"IDR_POPUP");
				hSubMenu = GetSubMenu(hMenu, 0);
				pt.x	 = (long)LOWORD(pmf->lParam);
				pt.y	 = (long)HIWORD(pmf->lParam);
				ClientToScreen(hEdit, &pt);
				CheckMenuItem(hSubMenu, ID_TAMA_REQUEST, (reqshow == SW_SHOW) ? MF_CHECKED : MF_UNCHECKED);
				CheckMenuItem(hSubMenu, ID_TAMA_RECEIVE, (receive) ? MF_CHECKED : MF_UNCHECKED);
				CheckMenuItem(hSubMenu, ID_TAMA_ALERTONWARNING, (AlertOnWarning) ? MF_CHECKED : MF_UNCHECKED);
				CheckMenuItem(hSubMenu, ID_TAMA_DISABLE_AUTO_TRANSFER_SHIORI_OWNERSHIP, (disable_auto_transfer_shiori_ownership) ? MF_CHECKED : MF_UNCHECKED);
				EnableMenuItem(hSubMenu, ID_TAMA_REQUEST, (shiorimode != not_in_loading && shioristaus == running) ? MF_ENABLED : MF_GRAYED);
				EnableMenuItem(hSubMenu, ID_TAMA_UNLOAD, (shiorimode != not_in_loading && shioristaus != unloaded) ? MF_ENABLED : MF_GRAYED);
				EnableMenuItem(hSubMenu, ID_TAMA_RELOAD, (has_shiori_file_info || shiorimode == load_by_baseware) ? MF_ENABLED : MF_GRAYED);
				EnableMenuItem(hSubMenu, ID_TAMA_DISABLE_AUTO_TRANSFER_SHIORI_OWNERSHIP, (!has_shiori_file_info) ? MF_DISABLED : MF_ENABLED);
				TrackPopupMenu(hSubMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
				DestroyMenu(hMenu);
			}
			break;
		}
		default:
			break;
		};
		break;
	case WM_SIZE:
		MoveWindow(hEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
		break;
	case WM_TIMER:
		switch(wParam) {
		case IDT_CHOST_STATUS_WATCHER:
			UpdateGhostModulestate();
			if(!tamaOpen_called && shioristaus == running)
				On_tamaOpen(hWnd, ghost_path);
			return 0;
		}
		break;
	case WM_COMMAND: {
		WORD wmId	 = LOWORD(wParam);
		WORD wmEvent = HIWORD(wParam);
		switch(wmId) {
		case ID_TAMA_JUMPTOP:
			//　先頭へジャンプ
			SendMessage(hEdit, WM_VSCROLL, SB_TOP, NULL);
			SendMessage(hEdit, WM_HSCROLL, SB_LEFT, NULL);
			break;
		case ID_TAMA_JUMPBOTTOM:
			//　終端へジャンプ
			SendMessage(hEdit, WM_VSCROLL, SB_BOTTOM, NULL);
			SendMessage(hEdit, WM_HSCROLL, SB_LEFT, NULL);
			break;
		case ID_TAMA_REQUEST:
			if(shiorimode == load_by_tama) {
				reqshow = (reqshow == SW_SHOW) ? SW_HIDE : SW_SHOW;
				ShowWindow(hDlgWnd, reqshow);
			}
			else {
				if(linker.was_linked_to_ghost()) {
					linker.SEND({{L"ID", ghost_uid},
								 {L"Script", L"\\![open,shiorirequest]"}});
				}
			}
			break;
		case ID_TAMA_ALERTONWARNING:
			AlertOnWarning ^= 1;
			break;
		case ID_TAMA_DISABLE_AUTO_TRANSFER_SHIORI_OWNERSHIP:
			disable_auto_transfer_shiori_ownership ^= 1;
			break;
		case ID_TAMA_RECEIVE:
			receive ^= 1;
			break;
		case ID_TAMA_RELOAD:
			// reload
			if(shiorimode == load_by_tama) {
				if(dllpath.size()) {
					ExecReload();
				}
				else if(b_dllpath.size()) {
					dllpath = b_dllpath;
					ExecLoad();
				}
			}
			else {
				if(linker.was_linked_to_ghost()) {
					if(shioristaus == unloaded)
						reload_shiori_of_baseware();
					if(shioristaus != critical) {
						auto info = linker.NOTYFY({{L"Event", L"tama.ShioriReloadRequest"}});
						switch(info.get_code()) {
						case -1:		//ssp exit
							[[fallthrough]];
						case 404:		//Not Found
							LostGhostLink();
							break;
						case 204:		//No Content
							[[fallthrough]];
						case 400:		//Bad Request
							reload_shiori_of_baseware();
						}
					}
					else
						ExecReload();
				}
			}
			break;
		case ID_TAMA_UNLOAD:
			// unload
			if(shiorimode == load_by_tama) {
				if(dllpath.size()) {
					ExecUnload();
					b_dllpath = dllpath;
					dllpath.clear();
				}
			}
			else {
				if(linker.was_linked_to_ghost()) {
					if(shioristaus == unloaded)
						unload_shiori_of_baseware();
					if(shioristaus != critical) {
						auto info = linker.NOTYFY({{L"Event", L"tama.ShioriUnloadRequest"}});
						switch(info.get_code()) {
						case -1:		//ssp exit
							[[fallthrough]];
						case 404:		//Not Found
							LostGhostLink();
							break;
						case 204:		//No Content
							[[fallthrough]];
						case 400:		//Bad Request
							unload_shiori_of_baseware();
						}
					}
					else
						ExecUnload();
				}
			}
			break;
		case ID_TAMA_COPY:
			// クリップボードへコピー
			SendMessage(hEdit, WM_COPY, 0, 0);
			break;
		case ID_TAMA_EXIT:
			// 終了
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	}
	case WM_CLOSE:
		if(!dllpath.empty())
			ExecUnload();
		SaveParameter();
		DestroyWindow(hEdit);
		DestroyWindow(hDlgWnd);
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		FreeLibrary(hRtLib);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK SendRequestDlgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch(msg) {
	case WM_CTLCOLORDLG:
		return (INT_PTR)DlgBrush;
	case WM_INITDIALOG:
		SetDlgFont(GetDlgItem(hWnd, IDC_SEND_REQUEST_RICHEDIT));
		SetDlgBkColor(GetDlgItem(hWnd, IDC_SEND_REQUEST_RICHEDIT), bkcol);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wp)) {
		case IDC_SEND:
			if(dllpath.size()) {
				auto DIofE = GetDlgItem(hWnd, IDC_SEND_REQUEST_RICHEDIT);
				auto size  = GetWindowTextLength(DIofE);
				dlgtext.resize(size);
				GetWindowTextW(DIofE, dlgtext.data(), size);
				ExecRequest(dlgtext.c_str());
			}
			return TRUE;
		case IDCANCEL:
			reqshow = SW_HIDE;
			ShowWindow(hDlgWnd, reqshow);
			return TRUE;
		};
	};

	return FALSE;
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
