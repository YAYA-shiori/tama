#include "stdafx.h"
#include "unicodeF.h"

// 汎用
#define MAX_LOADSTRING 100
#define	STRMAX	1024

// フォント設定
#define	FONTFACE		"Arial Unicode MS"	/* universal unicodeフォント */
#define	FONTFACELEN		16					/* FONTFACEのstrlen */

#define	FONT_JAPANESE	"ＭＳ ゴシック"		/* 日本語フォント */
#define	FONT_CHINESE	"MingLiU"			/* ハングルフォント */
#define	FONT_KOREAN		"Gulim"				/* 中国（繁体）フォント */
#define	FONT_ENGLISH	"Arial"				/* 上記以外（英語） */

// WM_COPYDATA dwData送信コマンド
#define	F_DEFAULT	0	/* 文字列 */
#define	F_FATAL		1	/* Fatal Error */
#define	F_ERROR		2	/* Error */
#define	F_WARNING	3	/* Warning */
#define	F_NOTE		4	/* Note */
#define	F_NUMBER	5	/* 総数 */
#define	E_SJIS		16	/* SJISモード */
#define	E_UTF8		17	/* UTF-8モード */
#define	E_DEFAULT	32	/* デフォルト文字コードモード */

/*// EM_SETTEXTEX補完
#define	EM_SETTEXTEX	(WM_USER + 97)

typedef struct _settextex {
    DWORD flags;
    UINT codepage;
} SETTEXTEX;
*/

// テキストサイズ上限
#define	TEXTMAX		131072	/* EM_EXLIMITTEXTするサイズ、128KB */

// フォントシェープ格納用
struct	SFShape
{
	int	pt;
	COLORREF	col;
	int	bold;
	int	italic;
};

struct	SFface
{
	string	face;
	int	charset;
};

// グローバル変数
HINSTANCE	hInst;							// インスタンス
HWND		hWnd, hDlgWnd;					// ウィンドウハンドル、ダイヤログウィンドウハンドル
HWND		hEdit;							// リッチエディットコントロールのハンドル
string		szTitle;						// キャプション
const char*	szWindowClass="TamaWndClass";	// ウィンドウクラス名
SFShape		fontshape[F_NUMBER];			// フォントシェープ
COLORREF	bkcol;							// 背景色
string		fontface;						// フォントフェース名
int			fontcharset;					// フォントの文字セット
string		dllpath;						// DLLパス
string		b_dllpath;						// 直前にunloadしたdllのパス
HMODULE		hDLL;							// DLLハンドル
int			reqshow;						// リクエストダイヤログの表示状態
string		dlgtext;						// リクエストダイヤログテキスト
char		charset;						// 文字セット
vector<SFface>	fontarray;					// フォント一覧
char		receive;						// 受信フラグ

HANDLE		hMutex;							// ミューテックスオブジェクト

// 関数のプロトタイプ
void	SetParameter(POINT &wp, SIZE &ws);
void	SaveParameter(void);
char	Split(char *str, char *s0, char *s1, const char *sepstr);
void	CutSpace(char *str);
int		HexStrToInt(char *str);
ATOM	MyRegisterClass(HINSTANCE hInstance);
BOOL	InitInstance( HINSTANCE, int );
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
int	CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int nFontType, LPARAM lParam);
int	GetFontCharSet(string name);

int	ExecLoad(void);
int	ExecRequest(const char *str);
int	ExecUnload(void);

void	EOS(int newaddsz);

BOOL	SetFontShape(int shapeid);
BOOL	SetFontShapeInit(int shapeid);
BOOL	SetMyFont(const char *facename, int shapeid, int scf);
BOOL	SetMyBkColor(COLORREF col);
BOOL	SetDlgFont(HWND hDlgEdit);
BOOL	SetDlgBkColor(HWND hDlgEdit, COLORREF col);

char	*setlocaleauto(int category);

bool	(*loadlib)(HGLOBAL h, long len);
bool	(*unloadlib)(void);
bool	(*logsend)(long hwnd);
HGLOBAL	(*requestlib)(HGLOBAL h, long *len);

string LoadStringFromResource(
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
		return string(pBuf, pBuf + len);
	else
		return "";
	*/
	string aret;
	size_t size = 1024,len=size;
	while(len == size && len) {
		aret.resize(size);
		auto len = LoadStringA(
			instance,
			stringID,
			aret.data(), size);
		size *= 2;
	}
	aret.resize(len);
	return aret;
}

// Winmain
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	hMutex      = ::CreateMutex(NULL, FALSE, "scns_task");
	if (hMutex == NULL)
		return FALSE;
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		::CloseHandle(hMutex);
		return FALSE;
	}

	MSG msg;
	HACCEL hAccelTable;

	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	szTitle		  = LoadStringFromResource(IDS_APP_TITLE, hInstance);
	MyRegisterClass( hInstance );

	if(!InitInstance(hInstance, nCmdShow)) 
		return FALSE;

	hAccelTable = LoadAccelerators(hInstance, szWindowClass);

	while(GetMessage(&msg, NULL, 0, 0)) {
		if(!TranslateAccelerator (msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}

	if(hMutex != NULL)
		::CloseHandle(hMutex);

	return msg.wParam;
}

void	SetParameter(POINT &wp, SIZE &ws)
{
	// 各種パラメータ設定

	// 初期値
	fontshape[F_DEFAULT].pt   = 9;
	fontshape[F_DEFAULT].col  = 0x0;
	fontshape[F_DEFAULT].bold = 0;
	fontshape[F_DEFAULT].italic = 0;
	fontshape[F_FATAL].pt     = 9;
	fontshape[F_FATAL].col    = 0x0000ff;
	fontshape[F_FATAL].bold   = 1;
	fontshape[F_FATAL].italic = 1;
	fontshape[F_ERROR].pt     = 9;
	fontshape[F_ERROR].col    = 0x0000ff;
	fontshape[F_ERROR].bold   = 0;
	fontshape[F_ERROR].italic = 0;
	fontshape[F_WARNING].pt   = 9;
	fontshape[F_WARNING].col  = 0x0080ff;
	fontshape[F_WARNING].bold = 0;
	fontshape[F_WARNING].italic = 0;
	fontshape[F_NOTE].pt      = 9;
	fontshape[F_NOTE].col     = 0x008000;
	fontshape[F_NOTE].bold    = 0;
	fontshape[F_NOTE].italic  = 0;
	bkcol                     = 0xffffff;
	fontface[0] = '\0';
	fontcharset = DEFAULT_CHARSET;
	receive = 1;
	wp.x  = -1024;
	wp.y  = -1024;
	ws.cx = 0;
	ws.cy = 0;
	// フォント列挙
	LOGFONT	logFont;
	ZeroMemory(&logFont, sizeof(LOGFONT));
	logFont.lfCharSet = DEFAULT_CHARSET;
    logFont.lfFaceName[0]    = NULL;
	logFont.lfPitchAndFamily = 0;
	HDC	hDC = GetDC(hWnd);
	EnumFontFamiliesEx(hDC, &logFont, (FONTENUMPROC)EnumFontFamExProc, NULL, 0);
	ReleaseDC(hWnd, hDC);
	// ファイルから取得
	string	filename;
	filename.resize(MAX_PATH);
	GetModuleFileName(NULL, filename.data(), MAX_PATH);
	char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	_splitpath(filename.c_str(), drive, dir, fname, ext);
	filename=string()+drive+dir+"tama.txt";
	FILE *fp = fopen(filename.c_str(), "rt");
	if (fp != NULL) {
		char	buf[STRMAX];
		char	s0[STRMAX], s1[STRMAX];
		for ( ; ; ) {
			if (fgets(buf, STRMAX, fp) == NULL)
				break;
			int	len = strlen(buf);
			if (len < 2)
				continue;
			if (buf[len - 1] == '\n')
				buf[len - 1] = '\0';
			if (buf[len - 2] == '\r')
				buf[len - 2] = '\0';
			if (!strlen(buf))
				continue;
			if (Split(buf, s0, s1, ",")) {
				// default
				if (!strcmp(s0, "default.pt"))
					fontshape[F_DEFAULT].pt = atoi(s1);
				else if (!strcmp(s0, "default.color")) {
					int	col = HexStrToInt(s1);
					fontshape[F_DEFAULT].col = ((col & 0xff) << 16) + (col & 0xff00) + ((col >> 16) & 0xff);
				}
				else if (!strcmp(s0, "default.bold"))
					fontshape[F_DEFAULT].bold = (atoi(s1)) ? 1 : 0;
				else if (!strcmp(s0, "default.italic"))
					fontshape[F_DEFAULT].italic = (atoi(s1)) ? 1 : 0;
				// fatal
				else if (!strcmp(s0, "fatal.pt"))
					fontshape[F_FATAL].pt = atoi(s1);
				else if (!strcmp(s0, "fatal.color")) {
					int	col = HexStrToInt(s1);
					fontshape[F_FATAL].col = ((col & 0xff) << 16) + (col & 0xff00) + ((col >> 16) & 0xff);
				}
				else if (!strcmp(s0, "fatal.bold"))
					fontshape[F_FATAL].bold = (atoi(s1)) ? 1 : 0;
				else if (!strcmp(s0, "fatal.italic"))
					fontshape[F_FATAL].italic = (atoi(s1)) ? 1 : 0;
				// error
				else if (!strcmp(s0, "error.pt"))
					fontshape[F_ERROR].pt = atoi(s1);
				else if (!strcmp(s0, "error.color")) {
					int	col = HexStrToInt(s1);
					fontshape[F_ERROR].col = ((col & 0xff) << 16) + (col & 0xff00) + ((col >> 16) & 0xff);
				}
				else if (!strcmp(s0, "error.bold"))
					fontshape[F_ERROR].bold = (atoi(s1)) ? 1 : 0;
				else if (!strcmp(s0, "error.italic"))
					fontshape[F_ERROR].italic = (atoi(s1)) ? 1 : 0;
				// warning
				else if (!strcmp(s0, "warning.pt"))
					fontshape[F_WARNING].pt = atoi(s1);
				else if (!strcmp(s0, "warning.color")) {
					int	col = HexStrToInt(s1);
					fontshape[F_WARNING].col = ((col & 0xff) << 16) + (col & 0xff00) + ((col >> 16) & 0xff);
				}
				else if (!strcmp(s0, "warning.bold"))
					fontshape[F_WARNING].bold = (atoi(s1)) ? 1 : 0;
				else if (!strcmp(s0, "warning.italic"))
					fontshape[F_WARNING].italic = (atoi(s1)) ? 1 : 0;
				// note
				else if (!strcmp(s0, "note.pt"))
					fontshape[F_NOTE].pt = atoi(s1);
				else if (!strcmp(s0, "note.color")) {
					int	col = HexStrToInt(s1);
					fontshape[F_NOTE].col = ((col & 0xff) << 16) + (col & 0xff00) + ((col >> 16) & 0xff);
				}
				else if (!strcmp(s0, "note.bold"))
					fontshape[F_NOTE].bold = (atoi(s1)) ? 1 : 0;
				else if (!strcmp(s0, "note.italic"))
					fontshape[F_NOTE].italic = (atoi(s1)) ? 1 : 0;
				// background
				else if (!strcmp(s0, "background.color")) {
					int	col = HexStrToInt(s1);
					bkcol = ((col & 0xff) << 16) + (col & 0xff00) + ((col >> 16) & 0xff);
				}
				// face
				else if (!strcmp(s0, "face"))
					fontface=s1;
				// window.x
				else if (!strcmp(s0, "window.x"))
					wp.x = atoi(s1);
				// window.y
				else if (!strcmp(s0, "window.y"))
					wp.y = atoi(s1);
				// window.width
				else if (!strcmp(s0, "window.width"))
					ws.cx = atoi(s1);
				// window.height
				else if (!strcmp(s0, "window.height"))
					ws.cy = atoi(s1);
			}
		}
		fclose(fp);
	}

	// フォント設定
	// ファイル指定→unicode→システムデフォルト→Arial、の順に検索して設定

	// ファイル指定
	if (fontface.size()) {
		fontcharset = GetFontCharSet(fontface);
		if (fontcharset != -1)
			return;
	}
	// unicode
	fontface=FONTFACE;
	fontcharset = GetFontCharSet(fontface);
	if (fontcharset != -1)
		return;
	// システムデフォルト
	switch(PRIMARYLANGID(GetSystemDefaultLangID())) {
	case LANG_KOREAN:
		fontface=FONT_KOREAN;
		break;
	case LANG_CHINESE:
		fontface=FONT_CHINESE;
		break;
	case LANG_JAPANESE:
		fontface=FONT_JAPANESE;
		break;
	default:	// 他はすべて英語にしてしまう
		fontface=FONT_ENGLISH;
		break;
	};
	fontcharset = GetFontCharSet(fontface);
	if (fontcharset != -1)
		return;
	// あきらめ。Arial/Default_charset
	fontface=FONT_ENGLISH;
	fontcharset = DEFAULT_CHARSET;
}

void	SaveParameter(void)
{
	// ファイルへ設定を書き出し
	string	filename;
	filename.resize(MAX_PATH);
	GetModuleFileName(NULL, filename.data(), MAX_PATH);
	char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	_splitpath(filename.c_str(), drive, dir, fname, ext);
	filename = string() + drive + dir + "tama.txt";
	FILE *fp = fopen(filename.c_str(), "wt");
	if (fp != NULL) {
		fprintf(fp, "default.pt,%d\n", fontshape[F_DEFAULT].pt);
		fprintf(fp, "default.color,%x\n",
			((fontshape[F_DEFAULT].col & 0xff) << 16) + (fontshape[F_DEFAULT].col & 0xff00) + ((fontshape[F_DEFAULT].col >> 16) & 0xff));
		fprintf(fp, "default.bold,%d\n", fontshape[F_DEFAULT].bold);
		fprintf(fp, "default.italic,%d\n", fontshape[F_DEFAULT].italic);

		fprintf(fp, "fatal.pt,%d\n", fontshape[F_FATAL].pt);
		fprintf(fp, "fatal.color,%x\n",
			((fontshape[F_FATAL].col & 0xff) << 16) + (fontshape[F_FATAL].col & 0xff00) + ((fontshape[F_FATAL].col >> 16) & 0xff));
		fprintf(fp, "fatal.bold,%d\n", fontshape[F_FATAL].bold);
		fprintf(fp, "fatal.italic,%d\n", fontshape[F_FATAL].italic);
		
		fprintf(fp, "error.pt,%d\n", fontshape[F_ERROR].pt);
		fprintf(fp, "error.color,%x\n",
			((fontshape[F_ERROR].col & 0xff) << 16) + (fontshape[F_ERROR].col & 0xff00) + ((fontshape[F_ERROR].col >> 16) & 0xff));
		fprintf(fp, "error.bold,%d\n", fontshape[F_ERROR].bold);
		fprintf(fp, "error.italic,%d\n", fontshape[F_ERROR].italic);
		
		fprintf(fp, "warning.pt,%d\n", fontshape[F_WARNING].pt);
		fprintf(fp, "warning.color,%x\n",
			((fontshape[F_WARNING].col & 0xff) << 16) + (fontshape[F_WARNING].col & 0xff00) + ((fontshape[F_WARNING].col >> 16) & 0xff));
		fprintf(fp, "warning.bold,%d\n", fontshape[F_WARNING].bold);
		fprintf(fp, "warning.italic,%d\n", fontshape[F_WARNING].italic);
		
		fprintf(fp, "note.pt,%d\n", fontshape[F_NOTE].pt);
		fprintf(fp, "note.color,%x\n",
			((fontshape[F_NOTE].col & 0xff) << 16) + (fontshape[F_NOTE].col & 0xff00) + ((fontshape[F_NOTE].col >> 16) & 0xff));
		fprintf(fp, "note.bold,%d\n", fontshape[F_NOTE].bold);
		fprintf(fp, "note.italic,%d\n", fontshape[F_NOTE].italic);
		
		fprintf(fp, "background.color,%x\n", ((bkcol & 0xff) << 16) + (bkcol & 0xff00) + ((bkcol >> 16) & 0xff));
		
		fprintf(fp, "face,%s\n", fontface.c_str());

		RECT	rect;
		GetWindowRect(hWnd, &rect);

		fprintf(fp, "window.x,%d\n", rect.left);
		fprintf(fp, "window.y,%d\n", rect.top);
		fprintf(fp, "window.width,%d\n", rect.right - rect.left + 1);
		fprintf(fp, "window.height,%d\n", rect.bottom - rect.top + 1);

		fclose(fp);
	}
}

int	GetFontCharSet(string name)
{
	// name名のフォントのcharsetを返す

	for(vector<SFface>::iterator it = fontarray.begin(); it != fontarray.end(); it++) {
		if (it->face == name)
			return it->charset;
	}
	return -1;
}

int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int nFontType, LPARAM lParam)
{
	// フォント列挙用のコールバック関数

	SFface	addface;
	addface.face    = lpelfe->elfLogFont.lfFaceName;
	addface.charset = lpelfe->elfLogFont.lfCharSet;
	fontarray.push_back(addface);

	return 1;
}

char	Split(char *str, char *s0, char *s1, const char *sepstr)
{
	// strをs0とs1に分解

	unsigned char *seppoint = _mbsstr((unsigned char *)str, (unsigned char *)sepstr);
	if (seppoint == NULL)
		return 0;
	strcpy(s0, str);
	*(s0 + ((int)seppoint - (int)str)) = '\0';
	strcpy(s1, (char *)((int)seppoint + strlen(sepstr)));
	CutSpace(s0);
	CutSpace(s1);

	return 1;
}

void	CutSpace(char *str)
{
	// str前後の空白とタブを削る

	if (str == NULL)
		return;
	int	i, j;
	i = strlen(str) - 1;
	if (i < 0)
		return;
	for(j = i; j >= 0; ) {
		unsigned char *_searchpoint = (unsigned char *)str + j;
		if (_mbsstr(_searchpoint, (unsigned char *)" " ) == _searchpoint ||
			_mbsstr(_searchpoint, (unsigned char *)"\t") == _searchpoint) {
			j--;
			continue;
		}
		break;
	}
	*(str + j + 1) = '\0';
	if (*str == '\0')
		return;
	i = j;
	for(j = 0; j <= i; ) {
		unsigned char *_searchpoint = (unsigned char *)str + j;
		if (_mbsstr(_searchpoint, (unsigned char *)" " ) == _searchpoint ||
			_mbsstr(_searchpoint, (unsigned char *)"\t") == _searchpoint) {
			j++;
			continue;
		}
		break;
	}
	memmove(str, str + j, i - j + 1);
	*(str + i - j + 1) = '\0';
}

int	HexStrToInt(char *str)
{
	// HEXのstrをintに変換

	int	len = strlen(str);
	int	result = 0;
	for(int i = 0; i < len; i++)
	{
		int	digit = (int)*(str + i);
	
		if (digit <= '9')
			digit -= '0';
		else if (digit <= 'Z')
			digit = digit - 'A' + 10;
		else
			digit = digit - 'a' + 10;
		result = (result << 4) + digit;
	}
	return result;
}

ATOM MyRegisterClass( HINSTANCE hInstance )
{
	// ウィンドウクラス登録

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style		= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon		= LoadIcon(hInstance, (LPCTSTR)IDI_DRAGFILES);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

BOOL InitInstance( HINSTANCE hInstance, int nCmdShow )
{
	// ウィンドウ作成と表示

	hInst = hInstance;
	RECT	rect;
	GetWindowRect(::GetDesktopWindow(), &rect);
	hWnd = CreateWindow(szWindowClass, szTitle.c_str(),
		WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
		rect.right >> 1, rect.bottom >> 1, rect.right >> 1, rect.bottom >> 1, NULL, NULL, NULL/*hInstance*/, NULL);
	if( !hWnd ) 
		return FALSE;
	hDlgWnd = CreateDialog(hInstance, "IDD_REQUEST", hWnd, (DLGPROC)DlgProc);
	if (hDlgWnd == NULL)
		return FALSE;

	ShowWindow(hWnd, SW_SHOW);
	reqshow = SW_HIDE;
	ShowWindow(hDlgWnd, reqshow);
	RECT	drect;
	GetWindowRect(hDlgWnd, &drect);
	MoveWindow(hDlgWnd, (rect.right >> 1) - 32, (rect.bottom >> 1) + 64,
		drect.right - drect.left, drect.bottom - drect.top, TRUE);
	UpdateWindow( hWnd );
	DragAcceptFiles(hWnd,TRUE);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// メッセージプロシージャ

	static HINSTANCE	hRtLib;
	static string tmpdllpath;

	int wmId, wmEvent;
	MSGFILTER *pmf;
	DWORD dwEvent;
	COPYDATASTRUCT	*cds;
	HDROP	hDrop;
	wchar_t	*logbuf;
	char	*mstr;
	int	dmy;
	HMENU hMenu, hSubMenu;
	POINT	pt;
	OSVERSIONINFO	osi = { sizeof(OSVERSIONINFO) };
	POINT	wpos;
	SIZE	wsz;

	switch(message) {
		case WM_CREATE:
			// パラメータ設定
			SetParameter(wpos, wsz);
			if (wsz.cx)
				MoveWindow(hWnd, wpos.x, wpos.y, wsz.cx, wsz.cy, TRUE);
			// リッチエディットコントロール作成
			hRtLib = LoadLibrary("RICHED32.DLL");
			hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "RICHEDIT", "",
				WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE |
				WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY,
				0, 0, 0, 0, hWnd, (HMENU)1, hInst, NULL);
			dwEvent = SendMessage(hEdit, EM_GETEVENTMASK, 0, 0) | ENM_KEYEVENTS | ENM_MOUSEEVENTS;
			SendMessage(hEdit, EM_SETEVENTMASK, 0, (LPARAM)dwEvent);
			SendMessage(hEdit, EM_EXLIMITTEXT,  0, (LPARAM)TEXTMAX);
			// フォントシェープ設定
			SetMyBkColor(bkcol);
			SetFontShapeInit(F_DEFAULT);
            break;
		case WM_COPYDATA:
			cds = (COPYDATASTRUCT *)lParam;
			if (cds->dwData >= 0 && cds->dwData < F_NUMBER && receive) {
				// メッセージ表示更新　NT系はunicodeのまま、9x系はMBCSへ変換して更新
				SetFontShape(cds->dwData);
				if (cds->cbData > 0) {
					// 更新
					wstring logbuf;
					logbuf.resize(cds->cbData);
					wcscpy(logbuf.data(), (wchar_t *)cds->lpData);
					GetVersionEx(&osi);
					if (osi.dwPlatformId == VER_PLATFORM_WIN32_NT) {
						EOS(logbuf.size());
						SendMessageW(hEdit, EM_REPLACESEL, (WPARAM)0, (LPARAM)logbuf.c_str());
					}
					else {
						mstr = CUnicodeF::utf16be_to_sjis(logbuf.c_str(), &dmy, CHARSET_DEFAULT);
						if (mstr != NULL) {
							EOS(strlen(mstr));
							SendMessage(hEdit, EM_REPLACESEL, (WPARAM)0, (LPARAM)mstr);
							free(mstr);
						}
					}
				}
			}
			else if (cds->dwData == F_NUMBER) {
				// 最後のメッセージ
			}
			// 文字コード設定
			else if (cds->dwData == E_SJIS)
				charset = CHARSET_SJIS;
			else if (cds->dwData == E_UTF8)
				charset = CHARSET_UTF8;
			else if (cds->dwData == E_DEFAULT)
				charset = CHARSET_DEFAULT;
			break;
		case WM_DROPFILES:
			hDrop = (HDROP)wParam;
			tmpdllpath.resize(MAX_PATH);
			if(::DragQueryFile(hDrop, 0xFFFFFFFF, tmpdllpath.data(), MAX_PATH)) {
				// ドロップされたファイル名を取得
				::DragQueryFile(hDrop, 0, tmpdllpath.data(), MAX_PATH);
				// load中ならunload
				if (dllpath.size())
					ExecUnload();
				// dllパス更新
				b_dllpath = dllpath;
				dllpath   = tmpdllpath;
				// load　失敗したらdllパスは空にする
				if (!ExecLoad())
					dllpath = "";
			}
			::DragFinish(hDrop);
			break;
		case WM_NOTIFY:
			switch (((NMHDR*)lParam)->code ) {
			case EN_MSGFILTER:
				pmf = (MSGFILTER *)lParam;
				if (pmf->msg == WM_RBUTTONDOWN) {
					// ポップアップメニュー表示
					hMenu = LoadMenu(hInst, "IDR_POPUP");
					hSubMenu = GetSubMenu(hMenu, 0);
					pt.x = (long)LOWORD(pmf->lParam);
					pt.y = (long)HIWORD(pmf->lParam);
					ClientToScreen(hEdit, &pt);
					CheckMenuItem(hSubMenu, ID_TAMA_REQUEST, (reqshow == SW_SHOW) ? 8 : 0);
					CheckMenuItem(hSubMenu, ID_TAMA_RECEIVE, (receive) ? 8 : 0);
					EnableMenuItem(hSubMenu, ID_TAMA_UNLOAD, (dllpath.size()) ? MF_ENABLED : MF_GRAYED);
					EnableMenuItem(hSubMenu, ID_TAMA_RELOAD, (dllpath.size() || b_dllpath.size()) ? MF_ENABLED : MF_GRAYED);
					TrackPopupMenu(hSubMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
					DestroyMenu(hMenu);
				}
				break;
			default:
				break;
            };
            break;
		case WM_SIZE:
			MoveWindow(hEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
			break;
		case WM_COMMAND:
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			switch(wmId) {
				case IDM_EXIT:
					// 終了
					DestroyWindow(hWnd);
					break;
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
					reqshow = (reqshow == SW_SHOW) ? SW_HIDE : SW_SHOW;
					ShowWindow(hDlgWnd, reqshow);
					break;
				case ID_TAMA_RECEIVE:
					receive ^= 1;
					break;
				case ID_TAMA_RELOAD:
					// reload
					if (dllpath.size()) {
						ExecUnload();
						ExecLoad();
					}
					else if (b_dllpath.size()) {
						dllpath = b_dllpath;
						ExecLoad();
					}
					break;
				case ID_TAMA_UNLOAD:
					// unload
					if (dllpath.size()) {
						ExecUnload();
						b_dllpath = dllpath;
						dllpath = "";
					}
					break;
				case ID_TAMA_COPY:
					// クリップボードへコピー
					SendMessage(hEdit, WM_COPY, 0, 0);
					break;
					break;
				case ID_TAMA_EXIT:
					// 終了
					SendMessage(hWnd, WM_CLOSE, 0, 0);
					break;
				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_CLOSE:
			if (dllpath.c_str())
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

LRESULT CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static HWND hParent;
	hParent = GetParent(hWnd);

	switch(msg) {
	case WM_INITDIALOG:
		SetDlgFont(GetDlgItem(hWnd, IDC_RICHEDIT));
		SetDlgBkColor(GetDlgItem(hWnd, IDC_RICHEDIT), bkcol);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wp)) {
		case IDC_SEND:
			if (dllpath.size()) {
				auto DIofE = GetDlgItem(hWnd, IDC_RICHEDIT);
				auto size  = GetWindowTextLength(DIofE);
				dlgtext.resize(size);
				GetWindowText(DIofE, dlgtext.data(), size);
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

int	ExecLoad(void)
{
	// ロード
	// 0/1=失敗/成功

	// テキスト全クリア
	SetWindowText(hEdit, "");
	SetFontShapeInit(F_DEFAULT);

	// dllフルパス作成
	char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	_splitpath(dllpath.c_str(), drive, dir, fname, ext);
	if (strlen(ext) != 4) {
		string tmp = dllpath + " : tama error TE0001 : This file isn't AYA dll.";
		SetWindowText(hEdit, tmp.c_str());
		return 0;
	}
	if (ext[0] != '.' ||
		tolower(ext[1]) != 'd' ||
		tolower(ext[2]) != 'l' ||
	   tolower(ext[3]) != 'l') {
		string tmp = dllpath + " : tama error TE0001 : This file isn't AYA dll.";
		SetWindowText(hEdit, tmp.c_str());
		return 0;
	}
	string path = string() + drive + dir;

	// logsend
	hDLL = LoadLibrary(dllpath.c_str());
	logsend = (bool (*)(long hwnd))GetProcAddress(hDLL, "logsend");
	if (logsend == NULL) {
		string tmp = dllpath + " : tama error TE0002 : Cannot load dll. (logsend)";
		SetWindowText(hEdit, tmp.c_str());
		FreeLibrary(hDLL);
		return 0;
	}
	(*logsend)((long)hWnd);
	// load
	loadlib = (bool (*)(HGLOBAL h, long len))GetProcAddress(hDLL, "load");
	if (loadlib == NULL) {
		string tmp = dllpath + " : tama error TE0003 : Load failure.";
		SetWindowText(hEdit, tmp.c_str());
		FreeLibrary(hDLL);
		return 0;
	}
	long pathlen = (long)path.size();
	HGLOBAL	pathstr;
	pathstr = ::GlobalAlloc(GMEM_FIXED, pathlen);
	memcpy(pathstr, path.c_str(), pathlen);
	(*loadlib)(pathstr, pathlen);

	return 1;
}

int	ExecRequest(const char *str)
{
	// リクエスト
	// 0/1=失敗/成功

//	if (!strlen(str))
//		return 0;

	OSVERSIONINFO	osi = { sizeof(OSVERSIONINFO) };

	SendMessage(hEdit, EM_SETSEL, -1, -1);

	requestlib = (HGLOBAL (*)(HGLOBAL h, long *len))GetProcAddress(hDLL, "request");
	if (requestlib == NULL) {
		string tmp = dllpath + " : tama error TE0005 : Request failure.";
		EOS(tmp.size());
		SendMessage(hEdit, EM_REPLACESEL, (WPARAM)0, (LPARAM)tmp.c_str());
		return 0;
	}
	// 文字コード変換
	int	dmy;
	wchar_t	*wstr = CUnicodeF::sjis_to_utf16be(str, &dmy, CHARSET_DEFAULT);
	if (wstr == NULL) {
		string tmp = dllpath + " : tama error TE0006 : Request failure.";
		EOS(tmp.size());
		SendMessage(hEdit, EM_REPLACESEL, (WPARAM)0, (LPARAM)tmp.c_str());
		return 0;
	}
	char	*sstr;
	if (charset == CHARSET_UTF8) {
		// UTF-8
		sstr = CUnicodeF::utf16be_to_utf8(wstr, &dmy);
		free(wstr);
		if (sstr == NULL) {
			string tmp = dllpath + " : tama error TE0007 : Request failure.";
			EOS(tmp.size());
			SendMessage(hEdit, EM_REPLACESEL, (WPARAM)0, (LPARAM)tmp.c_str());
			return 0;
		}
	}
	else {
		// SJISあるいはシステムデフォルト
		sstr = CUnicodeF::utf16be_to_sjis(wstr, &dmy, charset);
		free(wstr);
		if (sstr == NULL) {
			string tmp = dllpath + " : tama error TE0008 : Request failure.";
			EOS(tmp.size());
			SendMessage(hEdit, EM_REPLACESEL, (WPARAM)0, (LPARAM)tmp.c_str());
			return 0;
		}
	}
	// GMEM_FIXEDにコピー
	long len = (long)strlen(sstr);
	HGLOBAL	headerstr = ::GlobalAlloc(GMEM_FIXED, len);
	memcpy(headerstr, sstr, len);
	free(sstr);
	// 実行
	HGLOBAL	returnvalue = (*requestlib)(headerstr, &len);
	// 応答取得
	char	*returnstr = (char *)malloc((len + 1)*sizeof(char));
	if (returnstr != NULL) {
		// 取得
		strncpy(returnstr, (char *)returnvalue, len);
		*(returnstr + len) = '\0';
		GlobalFree(returnvalue);
		// 文字コード変換
		if (charset == CHARSET_UTF8)
			wstr = CUnicodeF::sjis_to_utf16be(returnstr, &dmy, CHARSET_DEFAULT);
		else
			wstr = CUnicodeF::sjis_to_utf16be(returnstr, &dmy, charset);
		free(returnstr);
		if (wstr == NULL) {
			string tmp = dllpath + " : tama error TE0009 : Request failure.";
			EOS(tmp.size());
			SendMessage(hEdit, EM_REPLACESEL, (WPARAM)0, (LPARAM)tmp.c_str());
			return 0;
		}
		free(wstr);
		EOS(2);
		SendMessage(hEdit, EM_REPLACESEL, (WPARAM)0, (LPARAM)"\n");
		return 1;
	}
	else {
		// 取得失敗
		GlobalFree(returnvalue);
		string tmp = dllpath + " : tama error TE0010 : Request failure.";
		EOS(tmp.size());
		SendMessage(hEdit, EM_REPLACESEL, (WPARAM)0, (LPARAM)tmp.c_str());
		return 0;
	}
}

int	ExecUnload()
{
	// アンロード
	// 0/1=失敗/成功

	SendMessage(hEdit, EM_SETSEL, -1, -1);

	unloadlib = (bool (*)(void))GetProcAddress(hDLL, "unload");
	if (unloadlib == NULL) {
		string tmp = dllpath + " : tama error TE0004 : Unload failure.";
		SetWindowText(hEdit, tmp.c_str());
		FreeLibrary(hDLL);
		return 0;
	}
	(*unloadlib)();
	FreeLibrary(hDLL);

	return 1;
}

BOOL	SetFontShape(int shapeid)
{
	// IDでフォント属性を設定

	return SetMyFont(fontface.c_str(), shapeid, SCF_SELECTION);
}

BOOL	SetFontShapeInit(int shapeid)
{
	// IDでフォント属性を初期化

	return SetMyFont(fontface.c_str(), shapeid, SCF_ALL);
}

BOOL SetMyFont(const char *facename, int shapeid, int scf)
{
	// フォントフェース、ポイント、色、ボールドを設定

	CHARFORMAT cfm;
	memset(&cfm, 0, sizeof(CHARFORMAT));
	cfm.cbSize = sizeof(CHARFORMAT);
	cfm.dwMask = CFM_BOLD | CFM_CHARSET | CFM_COLOR |
		CFM_FACE | CFM_ITALIC | CFM_SIZE | CFM_STRIKEOUT |
		CFM_UNDERLINE;
	cfm.yHeight = 20*fontshape[shapeid].pt;	// pt to twips
	cfm.bCharSet = fontcharset;
	strcpy_s(cfm.szFaceName, sizeof(cfm.szFaceName), facename);
	cfm.dwEffects = (fontshape[shapeid].bold) ? CFE_BOLD : 0;
	cfm.dwEffects |= (fontshape[shapeid].italic) ? CFE_ITALIC : 0;
	cfm.crTextColor = fontshape[shapeid].col;
	if (SendMessage(hEdit, EM_SETCHARFORMAT, (WPARAM)scf, (LPARAM)&cfm) == 0)
		return FALSE;

	SetFocus(hEdit);
    return TRUE;
}

BOOL SetDlgFont(HWND hDlgEdit)
{
	// requestダイヤログリッチエディットのフォントフェース、ポイント、色、ボールドを設定

	CHARFORMAT cfm;
	memset(&cfm, 0, sizeof(CHARFORMAT));
	cfm.cbSize = sizeof(CHARFORMAT);
	cfm.dwMask = CFM_BOLD | CFM_CHARSET | CFM_COLOR |
		CFM_FACE | CFM_ITALIC | CFM_SIZE | CFM_STRIKEOUT |
		CFM_UNDERLINE;
	cfm.yHeight = 20*fontshape[F_DEFAULT].pt;	// pt to twips
	cfm.bCharSet = fontcharset;
	strcpy_s(cfm.szFaceName, sizeof(cfm.szFaceName), fontface.c_str());
	cfm.dwEffects = (fontshape[F_DEFAULT].bold) ? CFE_BOLD : 0;
	cfm.dwEffects |= (fontshape[F_DEFAULT].italic) ? CFE_ITALIC : 0;
	cfm.crTextColor = fontshape[F_DEFAULT].col;
	if (SendMessage(hDlgEdit, EM_SETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&cfm) == 0)
		return FALSE;

	SetFocus(hDlgEdit);
    return TRUE;
}

BOOL	SetMyBkColor(COLORREF col)
{
	// 背景色をcolに設定

	if (SendMessage(hEdit, EM_SETBKGNDCOLOR, 0, (LPARAM)col) == 0)
		return FALSE;
    return TRUE;
}

BOOL	SetDlgBkColor(HWND hDlgEdit, COLORREF col)
{
	// 背景色をcolに設定

	if (SendMessage(hDlgEdit, EM_SETBKGNDCOLOR, 0, (LPARAM)col) == 0)
		return FALSE;
    return TRUE;
}

char *setlocaleauto(int category)
{
	// OSデフォルトの言語IDでロケール設定する

	switch(PRIMARYLANGID(GetSystemDefaultLangID())) {
		case LANG_AFRIKAANS:
			return setlocale(category, "Afrikaans");
		case LANG_KONKANI:
			return setlocale(category, "Konkani");
		case LANG_ALBANIAN:
			return setlocale(category, "Albanian");
		case LANG_KOREAN:
			return setlocale(category, "Korean");
		case LANG_ARABIC:
			return setlocale(category, "Arabic");
		case LANG_LATVIAN:
			return setlocale(category, "Latvian");
		case LANG_ARMENIAN:
			return setlocale(category, "Armenian");
		case LANG_LITHUANIAN:
			return setlocale(category, "Lithuanian");
		case LANG_ASSAMESE:
			return setlocale(category, "Assamese");
		case LANG_MACEDONIAN:
			return setlocale(category, "Macedonian");
		case LANG_AZERI:
			return setlocale(category, "Azeri");
		case LANG_MALAY:
			return setlocale(category, "Malay");
		case LANG_BASQUE:
			return setlocale(category, "Basque");
		case LANG_MALAYALAM:
			return setlocale(category, "Malayalam");
		case LANG_BELARUSIAN:
			return setlocale(category, "Belarusian");
		case LANG_MANIPURI:
			return setlocale(category, "Manipuri");
		case LANG_BENGALI:
			return setlocale(category, "Bengali");
		case LANG_MARATHI:
			return setlocale(category, "Marathi");
		case LANG_BULGARIAN:
			return setlocale(category, "Bulgarian");
		case LANG_NEPALI:
			return setlocale(category, "Nepali");
		case LANG_CATALAN:
			return setlocale(category, "Catalan");
		case LANG_NEUTRAL:
			return setlocale(category, "Neutral");
		case LANG_CHINESE:
			return setlocale(category, "Chinese");
		case LANG_NORWEGIAN:
			return setlocale(category, "Norwegian");
		case LANG_CROATIAN:
			return setlocale(category, "Croatian");
		case LANG_ORIYA:
			return setlocale(category, "Oriya");
		case LANG_CZECH:
			return setlocale(category, "Czech");
		case LANG_POLISH:
			return setlocale(category, "Polish");
		case LANG_DANISH:
			return setlocale(category, "Danish");
		case LANG_PORTUGUESE:
			return setlocale(category, "Portuguese");
		case LANG_DUTCH:
			return setlocale(category, "Dutch");
		case LANG_PUNJABI:
			return setlocale(category, "Punjabi");
		case LANG_ENGLISH:
			return setlocale(category, "English");
		case LANG_ROMANIAN:
			return setlocale(category, "Romanian");
		case LANG_ESTONIAN:
			return setlocale(category, "Estonian");
		case LANG_RUSSIAN:
			return setlocale(category, "Russian");
		case LANG_FAEROESE:
			return setlocale(category, "Faeroese");
		case LANG_SANSKRIT:
			return setlocale(category, "Sanskrit");
		case LANG_FARSI:
			return setlocale(category, "Farsi");
//		case LANG_SERBIAN:					// どこかと値が衝突している模様
//			return setlocale(category, "Serbian");
		case LANG_FINNISH:
			return setlocale(category, "Finnish");
		case LANG_SINDHI:
			return setlocale(category, "Sindhi");
		case LANG_FRENCH:
			return setlocale(category, "French");
		case LANG_SLOVAK:
			return setlocale(category, "Slovak");
		case LANG_GEORGIAN:
			return setlocale(category, "Georgian");
		case LANG_SLOVENIAN:
			return setlocale(category, "Slovenian");
		case LANG_GERMAN:
			return setlocale(category, "German");
		case LANG_SPANISH:
			return setlocale(category, "Spanish");
		case LANG_GREEK:
			return setlocale(category, "Greek");
		case LANG_SWAHILI:
			return setlocale(category, "Swahili");
		case LANG_GUJARATI:
			return setlocale(category, "Gujarati");
		case LANG_SWEDISH:
			return setlocale(category, "Swedish");
		case LANG_HEBREW:
			return setlocale(category, "Hebrew");
		case LANG_TAMIL:
			return setlocale(category, "Tamil");
		case LANG_HINDI:
			return setlocale(category, "Hindi");
		case LANG_TATAR:
			return setlocale(category, "Tatar");
		case LANG_HUNGARIAN:
			return setlocale(category, "Hungarian");
		case LANG_TELUGU:
			return setlocale(category, "Telugu");
		case LANG_ICELANDIC:
			return setlocale(category, "Icelandic");
		case LANG_THAI:
			return setlocale(category, "Thai");
		case LANG_INDONESIAN:
			return setlocale(category, "Indonesian");
		case LANG_TURKISH:
			return setlocale(category, "Turkish");
		case LANG_ITALIAN:
			return setlocale(category, "Italian");
//		case LANG_UKRANIAN:					// 存在しない？
//			return setlocale(category, "Ukranian");
		case LANG_JAPANESE:
			return setlocale(category, "Japanese");
		case LANG_URDU:
			return setlocale(category, "Urdu");
		case LANG_KANNADA:
			return setlocale(category, "Kannada");
		case LANG_UZBEK:
			return setlocale(category, "Uzbek");
		case LANG_KASHMIRI:
			return setlocale(category, "Kashmiri");
		case LANG_VIETNAMESE:
			return setlocale(category, "Vietnamese");
		case LANG_KAZAK:
			return setlocale(category, "Kazak");
		default:
			return setlocale(category, "English");
	};
}

void	EOS(int newaddsz)
{
	// newaddsz文字追加前に溢れた行を削除する
	int	j = TEXTMAX;
	int	i = ::GetWindowTextLength(hEdit);
	if (i + newaddsz > j) {
		int	k = newaddsz - (j - i);
		if (k < 0)
			k = 0;
		int	m = ::SendMessage(hEdit, EM_LINEFROMCHAR, (WPARAM)k, 0);
		int	l = ::SendMessage(hEdit, EM_LINEINDEX,    (WPARAM)m, 0);
		if (l < k )
			l = ::SendMessage(hEdit, EM_LINEINDEX, (WPARAM)m+1, 0);
		::SendMessage(hEdit, EM_SETSEL, 0, l);
		::SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)"");
	}

	SendMessage(hEdit, EM_SETSEL, -1, -1);
}