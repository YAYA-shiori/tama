#pragma once
#include <wtypes.h>
#include <string>
#include <vector>
using namespace std;

// テキストサイズ上限
#define TEXTMAX 131072 /* EM_EXLIMITTEXTするサイズ、128KB */


// 汎用
#define MAX_LOADSTRING 100
#define STRMAX		   1024

// フォント設定
#define CHECK_TOOL_WCNAME L"TamaWndClass"
#define FONTFACE		  L"Arial Unicode MS" /* universal unicodeフォント */
#define FONTFACELEN		  16				  /* FONTFACEのlstrlenW */

#define FONT_JAPANESE L"ＭＳ ゴシック" /* 日本語フォント */
#define FONT_CHINESE  L"MingLiU"	   /* 中国（繁体）フォント */
#define FONT_KOREAN	  L"Gulim"		   /* ハングルフォント */
#define FONT_ENGLISH  L"Arial"		   /* 上記以外（英語） */

// WM_COPYDATA dwData送信コマンド
#define F_DEFAULT 0 /* 文字列 */
#define F_FATAL	  1 /* Fatal Error */
#define F_ERROR	  2 /* Error */
#define F_WARNING 3 /* Warning */
#define F_NOTE	  4 /* Note */
#define F_NUMBER  5 /* 総数 */
inline bool F_was_warning_or_above(int dwdata) {
	return dwdata <= 3 && dwdata >= 1;
}
#define E_SJIS	  16 /* SJISモード */
#define E_UTF8	  17 /* UTF-8モード */
#define E_DEFAULT 32 /* デフォルト文字コードモード */

void CheckUpdate();


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
inline HINSTANCE	  hInst;												 // インスタンス
inline HWND			  hWnd, hDlgWnd;										 // ウィンドウハンドル、ダイヤログウィンドウハンドル
inline HWND			  hEdit;												 // リッチエディットコントロールのハンドル
inline wstring		  szTitle;												 // キャプション
inline const wchar_t *szWindowClass = CHECK_TOOL_WCNAME;					 // ウィンドウクラス名
inline SFShape		  fontshape[F_NUMBER];									 // フォントシェープ
inline COLORREF		  bkcol;												 // 背景色
inline COLORREF		  bdcol;												 // 标题栏色
inline COLORREF		  dlgcol;												 // 对话框控件色
inline wstring		  fontface;												 // フォントフェース名
inline BYTE			  fontcharset;											 // フォントの文字セット
inline wstring		  dllpath;												 // DLLパス
inline wstring		  b_dllpath;											 // 直前にunloadしたdllのパス
inline int			  reqshow;												 // リクエストダイヤログの表示状態
inline wstring		  dlgtext;												 // リクエストダイヤログテキスト
inline vector<SFface> fontarray;											 // フォント一覧
inline bool			  receive;												 // 受信フラグ
inline bool			  AlertOnWarning;										 // Alert on warning
inline HBRUSH		  DlgBrush			 = CreateSolidBrush(0xffffff);		 // 对话框控件画刷
inline bool			  tamaOpen_called	 = 0;
inline bool			  has_fatal_or_error = 0;
inline bool			  darkmode			 = 0;

inline UINT_PTR ghost_status_watcher = NULL;

namespace args_info {
	inline wstring selfpath;
	inline wstring ghost_link_to;
	inline HWND	   ghost_hwnd = NULL;
}		// namespace args_info
inline wstring ghost_path;
inline wstring ghost_uid;
inline wstring ghost_shiori;
inline wstring savefile_path;

inline HANDLE hMutex;		// ミューテックスオブジェクト


enum shiorimode_t { not_in_loading,
					load_by_baseware,
					load_by_tama };
inline shiorimode_t shiorimode = not_in_loading;
enum shioristaus_t { running,
					 critical,
					 unloaded };
inline shioristaus_t shioristaus = running;
enum tamamode_t { specified_ghost,
				  any };
inline tamamode_t tamamode			   = any;
inline bool		  has_shiori_file_info = 0;
inline bool		  allow_file_drug	   = 1;
