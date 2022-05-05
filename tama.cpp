#include "stdafx.h"
#include "my-gists/ukagaka/SFMO.hpp"
#include "my-gists/ukagaka/shiori_loader.hpp"
#define DONT_USE_SOCKET
#include "my-gists/ukagaka/sstp.hpp"
#include "my-gists/codepage.hpp"
#include <dwmapi.h>
#include "my-gists/windows/SetIcon.h"

// 汎用
#define MAX_LOADSTRING 100
#define STRMAX		   1024

// フォント設定
#define CHECK_TOOL_WCNAME L"TamaWndClass"
#define FONTFACE		  L"Arial Unicode MS" /* universal unicodeフォント */
#define FONTFACELEN		  16				  /* FONTFACEのlstrlenW */

#define FONT_JAPANESE L"ＭＳ ゴシック" /* 日本語フォント */
#define FONT_CHINESE  L"MingLiU"	   /* ハングルフォント */
#define FONT_KOREAN	  L"Gulim"		   /* 中国（繁体）フォント */
#define FONT_ENGLISH  L"Arial"		   /* 上記以外（英語） */

// WM_COPYDATA dwData送信コマンド
#define F_DEFAULT 0	 /* 文字列 */
#define F_FATAL	  1	 /* Fatal Error */
#define F_ERROR	  2	 /* Error */
#define F_WARNING 3	 /* Warning */
#define F_NOTE	  4	 /* Note */
#define F_NUMBER  5	 /* 総数 */
#define E_SJIS	  16 /* SJISモード */
#define E_UTF8	  17 /* UTF-8モード */
#define E_DEFAULT 32 /* デフォルト文字コードモード */

void CheckUpdate();

inline bool F_was_warning_or_above(int dwdata) {
	return dwdata <= 3 && dwdata >= 1;
}

/*// EM_SETTEXTEX補完
#define	EM_SETTEXTEX	(WM_USER + 97)

typedef struct _settextex {
    DWORD flags;
    UINT codepage;
} SETTEXTEX;
*/

// テキストサイズ上限
#define TEXTMAX 131072 /* EM_EXLIMITTEXTするサイズ、128KB */

// フォントシェープ格納用
struct SFShape {
	int		 pt;
	COLORREF col;
	bool	 bold;
	bool	 italic;
};

struct SFface {
	wstring face;
	BYTE	charset = DEFAULT_CHARSET;
};

// グローバル変数
HINSTANCE	   hInst;												  // インスタンス
HWND		   hWnd, hDlgWnd;										  // ウィンドウハンドル、ダイヤログウィンドウハンドル
HWND		   hEdit;												  // リッチエディットコントロールのハンドル
wstring		   szTitle;												  // キャプション
const wchar_t *szWindowClass = CHECK_TOOL_WCNAME;					  // ウィンドウクラス名
SFShape		   fontshape[F_NUMBER];									  // フォントシェープ
COLORREF	   bkcol;												  // 背景色
COLORREF	   bdcol;												  // 标题栏色
COLORREF	   dlgcol;												  // 对话框控件色
wstring		   fontface;											  // フォントフェース名
BYTE		   fontcharset;											  // フォントの文字セット
wstring		   dllpath;												  // DLLパス
wstring		   b_dllpath;											  // 直前にunloadしたdllのパス
int			   reqshow;												  // リクエストダイヤログの表示状態
wstring		   dlgtext;												  // リクエストダイヤログテキスト
vector<SFface> fontarray;											  // フォント一覧
bool		   receive;												  // 受信フラグ
bool		   AlertOnWarning;										  // Alert on warning
HBRUSH		   DlgBrush			  = CreateSolidBrush(0xffffff);		  // 对话框控件画刷
bool		   tamaOpen_called	  = 0;
bool		   has_fatal_or_error = 0;

UINT_PTR						ghost_status_watcher = NULL;
Cshiori							shiori;
SSTP_link_n::SSTP_Direct_link_t linker({{L"Charset", L"UTF-8"}, {L"Sender", L"tama"}});
SFMO_t							fmobj;

namespace args_info {
	wstring ghost_link_to;
	HWND	ghost_hwnd = NULL;
}		// namespace args_info
wstring ghost_path;
wstring ghost_uid;
wstring ghost_shiori;

HANDLE hMutex;		 // ミューテックスオブジェクト

// 関数のプロトタイプ
void			 SetParameter(POINT &wp, SIZE &ws);
bool			 SetParameter(const wstring s0, const wstring s1, POINT &wp, SIZE &ws);
void			 SaveParameter(void);
bool			 Split(wstring &str, wstring &s0, wstring &s1, const wstring sepstr);
void			 CutSpace(wstring &str);
int				 HexStrToInt(const wchar_t *str);
ATOM			 TamaMainWindowClassRegister(HINSTANCE hInstance);
BOOL			 InitInstance(HINSTANCE, int);
BOOL			 InitSendRequestDlg(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void			 reload_shiori_of_baseware();
void			 unload_shiori_of_baseware();
LRESULT CALLBACK SendRequestDlgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK GhostSelectDlgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
int CALLBACK	 EnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int nFontType, LPARAM lParam);
int				 GetFontCharSet(wstring name);

enum shiorimode_t { not_in_loading,
					load_by_baseware,
					load_by_tama };
shiorimode_t shiorimode = not_in_loading;
enum shioristaus_t { running,
					 critical };
shioristaus_t shioristaus = running;
enum tamamode_t { specified_ghost,
				  any };
tamamode_t tamamode				= any;
bool	   has_shiori_file_info = 0;
bool	   allow_file_drug		= 1;

void On_tamaOpen(HWND hWnd, wstring ghost_path);
void On_tamaExit(HWND hWnd, wstring ghost_path);

int	 ExecLoad(void);
void ExecRequest(const wchar_t *str);
void ExecUnload(void);
int	 ExecReload(void);

void EOS(int newaddsz);

BOOL SetFontShape(int shapeid);
BOOL SetFontShapeInit(int shapeid);
BOOL SetMyFont(const wchar_t *facename, int shapeid, int scf);
BOOL SetMyBkColor(COLORREF col);
BOOL SetDlgFont(HWND hDlgEdit);
BOOL SetDlgBkColor(HWND hDlgEdit, COLORREF col);

wchar_t *setlocaleauto(int category);

wstring LoadStringFromResource(
	__in UINT		   stringID,
	__in_opt HINSTANCE instance = NULL) {
	/*
	//bug but idk why
	TCHAR *pBuf = NULL;

	int len = LoadStringA(
		instance,
		stringID,
		reinterpret_cast<LPSTR>(&pBuf), 0);

	if(len)
		return wstring(pBuf, pBuf + len);
	else
		return "";
	*/
	wstring aret;
	size_t	size = 1024;
	auto	len	 = size;
	while(len == size && len) {
		aret.resize(size);
		len = LoadStringW(
			instance,
			stringID,
			aret.data(), size);
		size *= 2;
	}
	aret.resize(len);
	return aret;
}

BOOL has_corresponding_tama(wstring ghost_hwnd_str) {
	auto hMutex = ::CreateMutexW(NULL, FALSE, (L"scns_task" + ghost_hwnd_str).c_str());
	if(hMutex == NULL)
		return TRUE;
	if(GetLastError() == ERROR_ALREADY_EXISTS) {
		::CloseHandle(hMutex);
		return TRUE;
	}
	else {
		::CloseHandle(hMutex);
		return FALSE;
	}
}
BOOL has_corresponding_tama(HWND ghost_hwnd) {
	return has_corresponding_tama(to_wstring((size_t)ghost_hwnd));
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

void ArgsHandling() {
	LPWSTR *argv;
	int		_targc;
	using namespace args_info;

	argv		= CommandLineToArgvW(GetCommandLineW(), &_targc);
	size_t argc = _targc;

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
void GhostSelection(HINSTANCE hInstance) {
	using namespace args_info;
	if(ghost_hwnd)
		goto link_to_ghost;
	if(fmobj.Update_info()) {
		auto ghostnum = fmobj.info_map.size();
		if(ghostnum == 0) {
			ShowWindow(hWnd, SW_HIDE);
			MessageBoxW(NULL, LoadStringFromResource(IDS_ERROR_NONE_GHOST).c_str(), LoadStringFromResource(IDS_ERROR_TITTLE).c_str(), MB_ICONERROR | MB_OK);
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
				MessageBoxW(NULL, (LoadStringFromResource(IDS_ERROR_GHOST_NOT_FOUND_P1) + ghost_link_to + LoadStringFromResource(IDS_ERROR_GHOST_NOT_FOUND_P2)).c_str(), LoadStringFromResource(IDS_ERROR_TITTLE).c_str(), MB_ICONERROR | MB_OK);
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
			auto index = SendDlgItemMessageW(gsui, IDC_GHOST_SELECT_LIST, LB_ADDSTRING, 0, (LPARAM)LoadStringFromResource(IDS_GHOST_SELECT_START_WITH_OUT_GHOST).c_str());
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
		MessageBoxW(NULL, LoadStringFromResource(IDS_ERROR_GHOST_ALREADY_HAS_TAMA).c_str(), LoadStringFromResource(IDS_ERROR_TITTLE).c_str(), MB_ICONERROR | MB_OK);
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

void UpdateGhostModulestate() {
	if(ghost_uid.empty())
		return;
	fmobj.Update_info();
	auto shioristate	 = fmobj.info_map[ghost_uid].get_modulestate(L"shiori");
	auto shioristausback = shioristaus;
	allow_file_drug = 0;
	if(shioristate == L"running") {
		shioristaus = running;
		if(shioristausback == critical) {
			allow_file_drug = 0;
			if(!dllpath.empty())
				ExecUnload();
		}
	}
	else if(shioristate == L"critical") {
		shioristaus = critical;
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
}

// Winmain
int APIENTRY WinMain(
	_In_ HINSTANCE	   hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR		   lpCmdLine,
	_In_ int		   nShowCmd) {
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

	if(shiorimode == load_by_baseware)
		UpdateGhostModulestate();

	if(tamamode == specified_ghost && shioristaus == running)
		On_tamaOpen(hWnd, ghost_path);
	if(tamamode == specified_ghost) {
		bool has_log = GetWindowTextLength(hEdit);
		if(!has_log) {
			SetWindowTextW(hEdit, LoadStringFromResource(IDS_EVENT_DEF_REMINDER).c_str());
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

void On_tamaOpen(HWND hWnd, wstring ghost_path) {
	tamaOpen_called = 1;
	linker.setReplayHwnd(hWnd);
	WCHAR selfpath[MAX_PATH];
	GetModuleFileNameW(NULL, selfpath, MAX_PATH);
	auto info = linker.NOTYFY({{L"Event", L"tamaOpen"},
							   {L"Reference0", std::to_wstring((size_t)hWnd)},
							   {L"Reference1", selfpath}});
	if(info.has(L"Icon"))
		if(!SetIcon(hWnd, info[L"Icon"].c_str())) {
			wstring icofullpath = ghost_path + info[L"Icon"];
			SetIcon(hWnd, icofullpath.c_str());
		}
	if(info.has(L"Tittle"))
		SetWindowTextW(hWnd, info[L"Tittle"].c_str());

	auto &info_map = info.to_str_map();
	POINT wpos{};
	SIZE  wsz{};
	for(auto &pair: info_map) {
		if(pair.first.starts_with(L"X-SSTP-PassThru-"))
			SetParameter(pair.first.substr(16), pair.second, wpos, wsz);
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

[[noreturn]] void LostGhostLink() {
	MessageBoxW(NULL, LoadStringFromResource(IDS_ERROR_LOST_GHOST_LINK).c_str(), LoadStringFromResource(IDS_ERROR_TITTLE).c_str(), MB_ICONERROR | MB_OK);
	exit(EXIT_FAILURE);
}

void SetParameter(POINT &wp, SIZE &ws) {
	// 各種パラメータ設定

	// 初期値
	fontshape[F_DEFAULT].pt		= 9;
	fontshape[F_DEFAULT].col	= 0x0;
	fontshape[F_DEFAULT].bold	= 0;
	fontshape[F_DEFAULT].italic = 0;
	fontshape[F_FATAL].pt		= 9;
	fontshape[F_FATAL].col		= 0x0000ff;
	fontshape[F_FATAL].bold		= 1;
	fontshape[F_FATAL].italic	= 1;
	fontshape[F_ERROR].pt		= 9;
	fontshape[F_ERROR].col		= 0x0000ff;
	fontshape[F_ERROR].bold		= 0;
	fontshape[F_ERROR].italic	= 0;
	fontshape[F_WARNING].pt		= 9;
	fontshape[F_WARNING].col	= 0x0080ff;
	fontshape[F_WARNING].bold	= 0;
	fontshape[F_WARNING].italic = 0;
	fontshape[F_NOTE].pt		= 9;
	fontshape[F_NOTE].col		= 0x008000;
	fontshape[F_NOTE].bold		= 0;
	fontshape[F_NOTE].italic	= 0;
	bkcol						= 0xffffff;
	bdcol						= 0xffffff;
	dlgcol						= 0xffffff;
	fontcharset					= DEFAULT_CHARSET;
	receive						= 1;
	AlertOnWarning				= 1;
	wp.x						= -1024;
	wp.y						= -1024;
	ws.cx						= 0;
	ws.cy						= 0;
	// フォント列挙
	LOGFONT logFont;
	ZeroMemory(&logFont, sizeof(logFont));
	logFont.lfCharSet		 = DEFAULT_CHARSET;
	logFont.lfFaceName[0]	 = NULL;
	logFont.lfPitchAndFamily = 0;
	HDC hDC					 = GetDC(hWnd);
	EnumFontFamiliesEx(hDC, &logFont, (FONTENUMPROC)EnumFontFamExProc, NULL, 0);
	ReleaseDC(hWnd, hDC);
	// ファイルから取得
	wstring filename;
	if(tamamode==specified_ghost)
		filename = ghost_path + L"profile\\tama.txt";
	else {
		filename.resize(MAX_PATH);
		GetModuleFileName(NULL, filename.data(), MAX_PATH);
		wchar_t drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
		_wsplitpath(filename.c_str(), drive, dir, fname, ext);
		filename = wstring() + drive + dir + L"tama.txt";
	}
	FILE *fp = _wfopen(filename.c_str(), L"rt, ccs=UTF-8");
	if(fp == NULL && tamamode==specified_ghost){
		filename.resize(MAX_PATH);
		GetModuleFileName(NULL, filename.data(), MAX_PATH);
		wchar_t drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
		_wsplitpath(filename.c_str(), drive, dir, fname, ext);
		filename = wstring() + drive + dir + L"tama.txt";
		fp = _wfopen(filename.c_str(), L"rt, ccs=UTF-8");
	}
	if(fp != NULL) {
		wstring buf;
		wstring s0, s1;
		for(;;) {
			buf.resize(STRMAX);
			if(fgetws(buf.data(), STRMAX, fp) == NULL)
				break;
			size_t len = lstrlenW(buf.data());
			buf.resize(len);
			if(len < 2)
				continue;
			if(buf[len - 1] == '\n')
				buf.resize(len - 1);
			if(buf[len - 2] == '\r')
				buf.resize(len - 2);
			if(buf.empty())
				continue;
			if(Split(buf, s0, s1, L",")) {
				SetParameter(s0, s1, wp, ws);
			}
		}
		fclose(fp);
	}

	// フォント設定
	// ファイル指定→システムデフォルト→unicode→Arial、の順に検索して設定

	// ファイル指定
	if(fontface.size()) {
		fontcharset = GetFontCharSet(fontface);
		if(fontcharset != -1)
			return;
	}
	// システムデフォルト
	switch(PRIMARYLANGID(GetSystemDefaultLangID())) {
	case LANG_KOREAN:
		fontface = FONT_KOREAN;
		break;
	case LANG_CHINESE:
		fontface = FONT_CHINESE;
		break;
	case LANG_JAPANESE:
		fontface = FONT_JAPANESE;
		break;
	default:	   // 他はすべて英語にしてしまう
		fontface = FONT_ENGLISH;
		break;
	};
	fontcharset = GetFontCharSet(fontface);
	if(fontcharset != -1)
		return;
	// unicode
	fontface	= FONTFACE;
	fontcharset = GetFontCharSet(fontface);
	if(fontcharset != -1)
		return;
	// あきらめ。Arial/Default_charset
	fontface	= FONT_ENGLISH;
	fontcharset = DEFAULT_CHARSET;
}

bool SetParameter(const wstring s0, const wstring s1, POINT &wp, SIZE &ws) {
	// default
	if(s0 == L"default.pt")
		fontshape[F_DEFAULT].pt = _wtoi(s1.c_str());
	else if(s0 == L"default.color") {
		int col					 = HexStrToInt(s1.c_str());
		fontshape[F_DEFAULT].col = ((col & 0xff) << 16) + (col & 0xff00) + ((col >> 16) & 0xff);
	}
	else if(s0 == L"default.bold")
		fontshape[F_DEFAULT].bold = (_wtoi(s1.c_str())) ? 1 : 0;
	else if(s0 == L"default.italic")
		fontshape[F_DEFAULT].italic = (_wtoi(s1.c_str())) ? 1 : 0;
	// fatal
	else if(s0 == L"fatal.pt")
		fontshape[F_FATAL].pt = _wtoi(s1.c_str());
	else if(s0 == L"fatal.color") {
		int col				   = HexStrToInt(s1.c_str());
		fontshape[F_FATAL].col = ((col & 0xff) << 16) + (col & 0xff00) + ((col >> 16) & 0xff);
	}
	else if(s0 == L"fatal.bold")
		fontshape[F_FATAL].bold = (_wtoi(s1.c_str())) ? 1 : 0;
	else if(s0 == L"fatal.italic")
		fontshape[F_FATAL].italic = (_wtoi(s1.c_str())) ? 1 : 0;
	// error
	else if(s0 == L"error.pt")
		fontshape[F_ERROR].pt = _wtoi(s1.c_str());
	else if(s0 == L"error.color") {
		int col				   = HexStrToInt(s1.c_str());
		fontshape[F_ERROR].col = ((col & 0xff) << 16) + (col & 0xff00) + ((col >> 16) & 0xff);
	}
	else if(s0 == L"error.bold")
		fontshape[F_ERROR].bold = (_wtoi(s1.c_str())) ? 1 : 0;
	else if(s0 == L"error.italic")
		fontshape[F_ERROR].italic = (_wtoi(s1.c_str())) ? 1 : 0;
	// warning
	else if(s0 == L"warning.pt")
		fontshape[F_WARNING].pt = _wtoi(s1.c_str());
	else if(s0 == L"warning.color") {
		int col					 = HexStrToInt(s1.c_str());
		fontshape[F_WARNING].col = ((col & 0xff) << 16) + (col & 0xff00) + ((col >> 16) & 0xff);
	}
	else if(s0 == L"warning.bold")
		fontshape[F_WARNING].bold = (_wtoi(s1.c_str())) ? 1 : 0;
	else if(s0 == L"warning.italic")
		fontshape[F_WARNING].italic = (_wtoi(s1.c_str())) ? 1 : 0;
	// note
	else if(s0 == L"note.pt")
		fontshape[F_NOTE].pt = _wtoi(s1.c_str());
	else if(s0 == L"note.color") {
		int col				  = HexStrToInt(s1.c_str());
		fontshape[F_NOTE].col = ((col & 0xff) << 16) + (col & 0xff00) + ((col >> 16) & 0xff);
	}
	else if(s0 == L"note.bold")
		fontshape[F_NOTE].bold = (_wtoi(s1.c_str())) ? 1 : 0;
	else if(s0 == L"note.italic")
		fontshape[F_NOTE].italic = (_wtoi(s1.c_str())) ? 1 : 0;
	// background
	else if(s0 == L"background.color") {
		int col = HexStrToInt(s1.c_str());
		bkcol	= ((col & 0xff) << 16) + (col & 0xff00) + ((col >> 16) & 0xff);
	}
	// To change border color
	else if(s0 == L"border.color") {
		int col = HexStrToInt(s1.c_str());
		bdcol	= ((col & 0xff) << 16) + (col & 0xff00) + ((col >> 16) & 0xff);
		DwmSetWindowAttribute(hWnd, 20, &bdcol, sizeof(bdcol));
	}
	// To change border color
	else if(s0 == L"dialogbox.color") {
		int col = HexStrToInt(s1.c_str());
		dlgcol	= ((col & 0xff) << 16) + (col & 0xff00) + ((col >> 16) & 0xff);
		DeleteObject(DlgBrush);
		DlgBrush = CreateSolidBrush(dlgcol);
	}
	// AlertOnWarning
	else if(s0 == L"alert.onwarning")
		AlertOnWarning = (_wtoi(s1.c_str())) ? 1 : 0;
	// face
	else if(s0 == L"face")
		fontface = s1;
	// window.x
	else if(s0 == L"window.x")
		wp.x = _wtoi(s1.c_str());
	// window.y
	else if(s0 == L"window.y")
		wp.y = _wtoi(s1.c_str());
	// window.width
	else if(s0 == L"window.width")
		ws.cx = _wtoi(s1.c_str());
	// window.height
	else if(s0 == L"window.height")
		ws.cy = _wtoi(s1.c_str());
	else
		return 0;
	return 1;
}

void SaveParameter(void) {
	// ファイルへ設定を書き出し
	wstring filename;
	if(tamamode == specified_ghost)
		filename = ghost_path + L"profile\\tama.txt";
	else {
		filename.resize(MAX_PATH);
		GetModuleFileName(NULL, filename.data(), MAX_PATH);
		wchar_t drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
		_wsplitpath(filename.c_str(), drive, dir, fname, ext);
		filename = wstring() + drive + dir + L"tama.txt";
	}
	FILE *fp = _wfopen(filename.c_str(), L"wt, ccs=UTF-8");
	if(fp != NULL) {
		fwprintf(fp, L"default.pt,%d\n", fontshape[F_DEFAULT].pt);
		fwprintf(fp, L"default.color,%x\n",
				 ((fontshape[F_DEFAULT].col & 0xff) << 16) + (fontshape[F_DEFAULT].col & 0xff00) + ((fontshape[F_DEFAULT].col >> 16) & 0xff));
		fwprintf(fp, L"default.bold,%d\n", fontshape[F_DEFAULT].bold);
		fwprintf(fp, L"default.italic,%d\n", fontshape[F_DEFAULT].italic);

		fwprintf(fp, L"fatal.pt,%d\n", fontshape[F_FATAL].pt);
		fwprintf(fp, L"fatal.color,%x\n",
				 ((fontshape[F_FATAL].col & 0xff) << 16) + (fontshape[F_FATAL].col & 0xff00) + ((fontshape[F_FATAL].col >> 16) & 0xff));
		fwprintf(fp, L"fatal.bold,%d\n", fontshape[F_FATAL].bold);
		fwprintf(fp, L"fatal.italic,%d\n", fontshape[F_FATAL].italic);

		fwprintf(fp, L"error.pt,%d\n", fontshape[F_ERROR].pt);
		fwprintf(fp, L"error.color,%x\n",
				 ((fontshape[F_ERROR].col & 0xff) << 16) + (fontshape[F_ERROR].col & 0xff00) + ((fontshape[F_ERROR].col >> 16) & 0xff));
		fwprintf(fp, L"error.bold,%d\n", fontshape[F_ERROR].bold);
		fwprintf(fp, L"error.italic,%d\n", fontshape[F_ERROR].italic);

		fwprintf(fp, L"warning.pt,%d\n", fontshape[F_WARNING].pt);
		fwprintf(fp, L"warning.color,%x\n",
				 ((fontshape[F_WARNING].col & 0xff) << 16) + (fontshape[F_WARNING].col & 0xff00) + ((fontshape[F_WARNING].col >> 16) & 0xff));
		fwprintf(fp, L"warning.bold,%d\n", fontshape[F_WARNING].bold);
		fwprintf(fp, L"warning.italic,%d\n", fontshape[F_WARNING].italic);

		fwprintf(fp, L"note.pt,%d\n", fontshape[F_NOTE].pt);
		fwprintf(fp, L"note.color,%x\n",
				 ((fontshape[F_NOTE].col & 0xff) << 16) + (fontshape[F_NOTE].col & 0xff00) + ((fontshape[F_NOTE].col >> 16) & 0xff));
		fwprintf(fp, L"note.bold,%d\n", fontshape[F_NOTE].bold);
		fwprintf(fp, L"note.italic,%d\n", fontshape[F_NOTE].italic);

		fwprintf(fp, L"background.color,%x\n", ((bkcol & 0xff) << 16) + (bkcol & 0xff00) + ((bkcol >> 16) & 0xff));

		fwprintf(fp, L"border.color,%x\n", ((bdcol & 0xff) << 16) + (bdcol & 0xff00) + ((bdcol >> 16) & 0xff));

		fwprintf(fp, L"dialogbox.color,%x\n", ((dlgcol & 0xff) << 16) + (dlgcol & 0xff00) + ((dlgcol >> 16) & 0xff));
		
		fwprintf(fp, L"alert.onwarning,%d\n", AlertOnWarning);

		fwprintf(fp, L"face,%s\n", fontface.c_str());

		RECT rect;
		GetWindowRect(hWnd, &rect);

		fwprintf(fp, L"window.x,%ld\n", rect.left);
		fwprintf(fp, L"window.y,%ld\n", rect.top);
		fwprintf(fp, L"window.width,%ld\n", rect.right - rect.left + 1);
		fwprintf(fp, L"window.height,%ld\n", rect.bottom - rect.top + 1);

		fclose(fp);
	}
}

int GetFontCharSet(wstring name) {
	// name名のフォントのcharsetを返す

	for(vector<SFface>::iterator it = fontarray.begin(); it != fontarray.end(); it++) {
		if(it->face == name)
			return it->charset;
	}
	return -1;
}

int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int nFontType, LPARAM lParam) {
	// フォント列挙用のコールバック関数

	SFface addface;
	addface.face	= lpelfe->elfLogFont.lfFaceName;
	addface.charset = lpelfe->elfLogFont.lfCharSet;
	fontarray.push_back(addface);

	return 1;
}

bool Split(wstring &str, wstring &s0, wstring &s1, const wstring sepstr) {
	// strをs0とs1に分解
	auto begin = str.find(sepstr);
	s0		   = str.substr(0, begin);
	s1		   = str.substr(begin + sepstr.size());
	CutSpace(s0);
	CutSpace(s1);

	return begin != wstring::npos;
}

void CutSpace(wstring &str) {
	// str前後の空白とタブを削る

	str = str.substr(str.find_first_not_of(L" \t"), str.find_last_not_of(L" \t") + 1);
}

int HexStrToInt(const wchar_t *str) {
	// HEXのstrをintに変換

	int len	   = lstrlenW(str);
	int result = 0;
	for(int i = 0; i < len; i++) {
		int digit = (int)*(str + i);

		if(digit <= L'9')
			digit -= L'0';
		else if(digit <= L'Z')
			digit = digit - L'A' + 10;
		else
			digit = digit - L'a' + 10;
		result = (result << 4) + digit;
	}
	return result;
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
		if(wsz.cx)
			MoveWindow(hWnd, wpos.x, wpos.y, wsz.cx, wsz.cy, TRUE);
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
				SetFontShape(cds->dwData);
				// 更新
				wstring logbuf;
				logbuf.resize(cds->cbData);
				wcscpy(logbuf.data(), (wchar_t *)cds->lpData);

				static OSVERSIONINFO osi	   = {sizeof(osi)};
				static bool			 osiiniter = GetVersionEx(&osi);

				if(osi.dwPlatformId == VER_PLATFORM_WIN32_NT) {
					EOS(logbuf.size());
					SendMessageW(hEdit, EM_REPLACESEL, (WPARAM)0, (LPARAM)logbuf.c_str());
				}
				else {
					string mstr = CODEPAGE_n::UnicodeToMultiByte(logbuf, CODEPAGE_n::CP_ACP);
					if(!mstr.empty()) {
						EOS(mstr.size());
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
					LPCWSTR boxtittle;
					auto	icontype = MB_ICONERROR;
					switch(cds->dwData) {
					case F_FATAL:
						boxtittle = L"ghost fatal error";
						break;
					case F_ERROR:
						boxtittle = L"ghost error";
						break;
					case F_WARNING:
						boxtittle = L"ghost warning";
						icontype  = MB_ICONWARNING;
						break;
					default:
						boxtittle = NULL;
					}
					MessageBoxW(NULL, logbuf.c_str(), boxtittle, icontype | MB_OK);
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
			MessageBoxW(NULL, LoadStringFromResource(IDS_ERROR_DRAGGING_FILE_IS_NOT_ALLOWED).c_str(), LoadStringFromResource(IDS_ERROR_TITTLE).c_str(), MB_ICONERROR | MB_OK);
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
				CheckMenuItem(hSubMenu, ID_TAMA_REQUEST, (reqshow == SW_SHOW) ? 8 : 0);
				CheckMenuItem(hSubMenu, ID_TAMA_RECEIVE, (receive) ? 8 : 0);
				CheckMenuItem(hSubMenu, ID_TAMA_ALERTONWARNING, (AlertOnWarning) ? 8 : 0);
				EnableMenuItem(hSubMenu, ID_TAMA_REQUEST, (shiorimode != not_in_loading) ? MF_ENABLED : MF_GRAYED);
				EnableMenuItem(hSubMenu, ID_TAMA_UNLOAD, (shiorimode != not_in_loading) ? MF_ENABLED : MF_GRAYED);
				EnableMenuItem(hSubMenu, ID_TAMA_RELOAD, (has_shiori_file_info || shiorimode == load_by_baseware) ? MF_ENABLED : MF_GRAYED);
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
					if(shioristaus == running) {
						auto info = linker.NOTYFY({{L"Event", L"tama.ShioriReloadRequest"}});
						switch(info.get_code()) {
						case -1:	   //ssp exit
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
					if(shioristaus == running) {
						auto info = linker.NOTYFY({{L"Event", L"tama.ShioriUnloadRequest"}});
						switch(info.get_code()) {
						case -1:	   //ssp exit
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

void reload_shiori_of_baseware() {
	linker.SEND({{L"ID", ghost_uid},
				 {L"Script", L"\\![reload,shiori]"}});
	shiorimode = load_by_baseware;
}

void unload_shiori_of_baseware() {
	linker.SEND({{L"ID", ghost_uid},
				 {L"Script", L"\\![unload,shiori]"}});
	shiorimode = not_in_loading;
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
				MessageBoxW(NULL, LoadStringFromResource(IDS_ERROR_NULL_GHOST_HWND).c_str(), LoadStringFromResource(IDS_ERROR_TITTLE).c_str(), MB_ICONERROR | MB_OK);
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

int ExecLoad(void) {
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

int ExecReload() {
	ExecUnload();
	return ExecLoad();
}

BOOL SetFontShape(int shapeid) {
	// IDでフォント属性を設定

	return SetMyFont(fontface.c_str(), shapeid, SCF_SELECTION);
}

BOOL SetFontShapeInit(int shapeid) {
	// IDでフォント属性を初期化

	return SetMyFont(fontface.c_str(), shapeid, SCF_ALL);
}

BOOL SetMyFont(const wchar_t *facename, int shapeid, int scf) {
	// フォントフェース、ポイント、色、ボールドを設定
	static CHARFORMAT cfm{sizeof(cfm), CFM_BOLD | CFM_CHARSET | CFM_COLOR |
										   CFM_FACE | CFM_ITALIC | CFM_SIZE | CFM_STRIKEOUT |
										   CFM_UNDERLINE};
	cfm.bCharSet						   = fontcharset;
	static const wchar_t *facename_setting = NULL;
	if(facename_setting != facename) {
		facename_setting = facename;
		wcscpy_s(cfm.szFaceName, sizeof(cfm.szFaceName) / sizeof(TCHAR), facename);
	}
	cfm.yHeight		= 20 * fontshape[shapeid].pt;		// pt to twips
	cfm.crTextColor = fontshape[shapeid].col;
	cfm.dwEffects	= ((fontshape[shapeid].bold) ? CFE_BOLD : 0) | ((fontshape[shapeid].italic) ? CFE_ITALIC : 0);
	if(SendMessage(hEdit, EM_SETCHARFORMAT, (WPARAM)scf, (LPARAM)&cfm) == 0)
		return FALSE;

	SetFocus(hEdit);
	return TRUE;
}

BOOL SetDlgFont(HWND hDlgEdit) {
	// requestダイヤログリッチエディットのフォントフェース、ポイント、色、ボールドを設定

	CHARFORMAT cfm;
	memset(&cfm, 0, sizeof(cfm));
	cfm.cbSize = sizeof(cfm);
	cfm.dwMask = CFM_BOLD | CFM_CHARSET | CFM_COLOR |
				 CFM_FACE | CFM_ITALIC | CFM_SIZE | CFM_STRIKEOUT |
				 CFM_UNDERLINE;
	cfm.yHeight	 = 20 * fontshape[F_DEFAULT].pt;	   // pt to twips
	cfm.bCharSet = fontcharset;
	wcscpy_s(cfm.szFaceName, sizeof(cfm.szFaceName) / sizeof(TCHAR), fontface.c_str());
	cfm.dwEffects = (fontshape[F_DEFAULT].bold) ? CFE_BOLD : 0;
	cfm.dwEffects |= (fontshape[F_DEFAULT].italic) ? CFE_ITALIC : 0;
	cfm.crTextColor = fontshape[F_DEFAULT].col;
	if(SendMessage(hDlgEdit, EM_SETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&cfm) == 0)
		return FALSE;

	SetFocus(hDlgEdit);
	return TRUE;
}

BOOL SetMyBkColor(COLORREF col) {
	// 背景色をcolに設定

	if(SendMessage(hEdit, EM_SETBKGNDCOLOR, 0, (LPARAM)col) == 0)
		return FALSE;
	return TRUE;
}

BOOL SetDlgBkColor(HWND hDlgEdit, COLORREF col) {
	// 背景色をcolに設定

	if(SendMessage(hDlgEdit, EM_SETBKGNDCOLOR, 0, (LPARAM)col) == 0)
		return FALSE;
	return TRUE;
}

wchar_t *setlocaleauto(int category) {
	// OSデフォルトの言語IDでロケール設定する

	switch(PRIMARYLANGID(GetSystemDefaultLangID())) {
	case LANG_AFRIKAANS:
		return _wsetlocale(category, L"Afrikaans");
	case LANG_KONKANI:
		return _wsetlocale(category, L"Konkani");
	case LANG_ALBANIAN:
		return _wsetlocale(category, L"Albanian");
	case LANG_KOREAN:
		return _wsetlocale(category, L"Korean");
	case LANG_ARABIC:
		return _wsetlocale(category, L"Arabic");
	case LANG_LATVIAN:
		return _wsetlocale(category, L"Latvian");
	case LANG_ARMENIAN:
		return _wsetlocale(category, L"Armenian");
	case LANG_LITHUANIAN:
		return _wsetlocale(category, L"Lithuanian");
	case LANG_ASSAMESE:
		return _wsetlocale(category, L"Assamese");
	case LANG_MACEDONIAN:
		return _wsetlocale(category, L"Macedonian");
	case LANG_AZERI:
		return _wsetlocale(category, L"Azeri");
	case LANG_MALAY:
		return _wsetlocale(category, L"Malay");
	case LANG_BASQUE:
		return _wsetlocale(category, L"Basque");
	case LANG_MALAYALAM:
		return _wsetlocale(category, L"Malayalam");
	case LANG_BELARUSIAN:
		return _wsetlocale(category, L"Belarusian");
	case LANG_MANIPURI:
		return _wsetlocale(category, L"Manipuri");
	case LANG_BENGALI:
		return _wsetlocale(category, L"Bengali");
	case LANG_MARATHI:
		return _wsetlocale(category, L"Marathi");
	case LANG_BULGARIAN:
		return _wsetlocale(category, L"Bulgarian");
	case LANG_NEPALI:
		return _wsetlocale(category, L"Nepali");
	case LANG_CATALAN:
		return _wsetlocale(category, L"Catalan");
	case LANG_NEUTRAL:
		return _wsetlocale(category, L"Neutral");
	case LANG_CHINESE:
		return _wsetlocale(category, L"Chinese");
	case LANG_NORWEGIAN:
		return _wsetlocale(category, L"Norwegian");
	case LANG_CROATIAN:
		return _wsetlocale(category, L"Croatian");
	case LANG_ORIYA:
		return _wsetlocale(category, L"Oriya");
	case LANG_CZECH:
		return _wsetlocale(category, L"Czech");
	case LANG_POLISH:
		return _wsetlocale(category, L"Polish");
	case LANG_DANISH:
		return _wsetlocale(category, L"Danish");
	case LANG_PORTUGUESE:
		return _wsetlocale(category, L"Portuguese");
	case LANG_DUTCH:
		return _wsetlocale(category, L"Dutch");
	case LANG_PUNJABI:
		return _wsetlocale(category, L"Punjabi");
	case LANG_ENGLISH:
		return _wsetlocale(category, L"English");
	case LANG_ROMANIAN:
		return _wsetlocale(category, L"Romanian");
	case LANG_ESTONIAN:
		return _wsetlocale(category, L"Estonian");
	case LANG_RUSSIAN:
		return _wsetlocale(category, L"Russian");
	case LANG_FAEROESE:
		return _wsetlocale(category, L"Faeroese");
	case LANG_SANSKRIT:
		return _wsetlocale(category, L"Sanskrit");
	case LANG_FARSI:
		return _wsetlocale(category, L"Farsi");
	//case LANG_SERBIAN:					// どこかと値が衝突している模様
	//	return _wsetlocale(category, L"Serbian");
	case LANG_FINNISH:
		return _wsetlocale(category, L"Finnish");
	case LANG_SINDHI:
		return _wsetlocale(category, L"Sindhi");
	case LANG_FRENCH:
		return _wsetlocale(category, L"French");
	case LANG_SLOVAK:
		return _wsetlocale(category, L"Slovak");
	case LANG_GEORGIAN:
		return _wsetlocale(category, L"Georgian");
	case LANG_SLOVENIAN:
		return _wsetlocale(category, L"Slovenian");
	case LANG_GERMAN:
		return _wsetlocale(category, L"German");
	case LANG_SPANISH:
		return _wsetlocale(category, L"Spanish");
	case LANG_GREEK:
		return _wsetlocale(category, L"Greek");
	case LANG_SWAHILI:
		return _wsetlocale(category, L"Swahili");
	case LANG_GUJARATI:
		return _wsetlocale(category, L"Gujarati");
	case LANG_SWEDISH:
		return _wsetlocale(category, L"Swedish");
	case LANG_HEBREW:
		return _wsetlocale(category, L"Hebrew");
	case LANG_TAMIL:
		return _wsetlocale(category, L"Tamil");
	case LANG_HINDI:
		return _wsetlocale(category, L"Hindi");
	case LANG_TATAR:
		return _wsetlocale(category, L"Tatar");
	case LANG_HUNGARIAN:
		return _wsetlocale(category, L"Hungarian");
	case LANG_TELUGU:
		return _wsetlocale(category, L"Telugu");
	case LANG_ICELANDIC:
		return _wsetlocale(category, L"Icelandic");
	case LANG_THAI:
		return _wsetlocale(category, L"Thai");
	case LANG_INDONESIAN:
		return _wsetlocale(category, L"Indonesian");
	case LANG_TURKISH:
		return _wsetlocale(category, L"Turkish");
	case LANG_ITALIAN:
		return _wsetlocale(category, L"Italian");
	//case LANG_UKRANIAN:					// 存在しない？
	//	return _wsetlocale(category, L"Ukranian");
	case LANG_JAPANESE:
		return _wsetlocale(category, L"Japanese");
	case LANG_URDU:
		return _wsetlocale(category, L"Urdu");
	case LANG_KANNADA:
		return _wsetlocale(category, L"Kannada");
	case LANG_UZBEK:
		return _wsetlocale(category, L"Uzbek");
	case LANG_KASHMIRI:
		return _wsetlocale(category, L"Kashmiri");
	case LANG_VIETNAMESE:
		return _wsetlocale(category, L"Vietnamese");
	case LANG_KAZAK:
		return _wsetlocale(category, L"Kazak");
	default:
		return _wsetlocale(category, L"English");
	};
}

void EOS(int newaddsz) {
	// newaddsz文字追加前に溢れた行を削除する
	int j = TEXTMAX;
	int i = ::GetWindowTextLength(hEdit);
	if(i + newaddsz > j) {
		int k = newaddsz - (j - i);
		if(k < 0)
			k = 0;
		int m = ::SendMessage(hEdit, EM_LINEFROMCHAR, (WPARAM)k, 0);
		int l = ::SendMessage(hEdit, EM_LINEINDEX, (WPARAM)m, 0);
		if(l < k)
			l = ::SendMessage(hEdit, EM_LINEINDEX, (WPARAM)m + 1, 0);
		::SendMessage(hEdit, EM_SETSEL, 0, l);
		::SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM) "");
	}

	SendMessage(hEdit, EM_SETSEL, -1, -1);
}
