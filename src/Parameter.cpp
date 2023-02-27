#define _CRT_SECURE_NO_WARNINGS

#include "header_files/tama.h"
#include "header_files/ToolFunctions.hpp"
#include "header_files/Parameter.hpp"
#include "header_files/UItools.hpp"
#include "my-gists/file/canXfile.hpp"
#include "my-gists/ukagaka/ghost_path.hpp"
#include <dwmapi.h>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
	#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#ifndef DWMWA_BORDER_COLOR
	#define DWMWA_BORDER_COLOR 34
#endif

void InitSaveFilePath() {
	wstring &selfpath = args_info::selfpath;
	wstring	 filename;
	if(tamamode == specified_ghost)
		filename = ghost_path + L"profile\\tama.txt";
	else if(path_in_ghost_dir(selfpath)) {
		filename = get_ghost_path(selfpath) + L"profile\\tama.txt";
	}
	if(can_t_read_write_file(filename)) {
		wchar_t drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
		_wsplitpath(selfpath.c_str(), drive, dir, fname, ext);
		filename = wstring() + drive + dir + L"tama.txt";
	}
	savefile_path = filename;
}

void SetParameter(POINT &wp, SIZE &ws) {
	// 各種パラメータ設定

	// 初期値
	fontshape[F_DEFAULT].pt				   = 9;
	fontshape[F_DEFAULT].col			   = 0x0;
	fontshape[F_DEFAULT].bold			   = 0;
	fontshape[F_DEFAULT].italic			   = 0;
	fontshape[F_FATAL].pt				   = 9;
	fontshape[F_FATAL].col				   = 0x0000ff;
	fontshape[F_FATAL].bold				   = 1;
	fontshape[F_FATAL].italic			   = 1;
	fontshape[F_ERROR].pt				   = 9;
	fontshape[F_ERROR].col				   = 0x0000ff;
	fontshape[F_ERROR].bold				   = 0;
	fontshape[F_ERROR].italic			   = 0;
	fontshape[F_WARNING].pt				   = 9;
	fontshape[F_WARNING].col			   = 0x0080ff;
	fontshape[F_WARNING].bold			   = 0;
	fontshape[F_WARNING].italic			   = 0;
	fontshape[F_NOTE].pt				   = 9;
	fontshape[F_NOTE].col				   = 0x008000;
	fontshape[F_NOTE].bold				   = 0;
	fontshape[F_NOTE].italic			   = 0;
	bkcol								   = 0xffffff;
	bdcol								   = 0xffffff;
	dlgcol								   = 0xffffff;
	fontcharset							   = DEFAULT_CHARSET;
	receive								   = 1;
	AlertOnWarning						   = 1;
	disable_auto_transfer_shiori_ownership = 0;
	wp.x								   = -1024;
	wp.y								   = -1024;
	ws.cx								   = 0;
	ws.cy								   = 0;
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
	InitSaveFilePath();
	FILE *fp = _wfopen(savefile_path.c_str(), L"rt, ccs=UTF-8");
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
	if(wp.x < -ws.cx)
		wp.x = LONG_MIN;
	if(wp.y < -ws.cy)
		wp.y = LONG_MIN;
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

bool SetParameter(wstring_view s0, wstring_view s1, POINT &wp, SIZE &ws) {
	// default
	if(s0 == L"default.pt")
		fontshape[F_DEFAULT].pt = StrToInt(s1);
	else if(s0 == L"default.color") {
		int col					 = HexStrToInt(s1);
		fontshape[F_DEFAULT].col = ((col & 0xff) << 16) + (col & 0xff00) + ((col >> 16) & 0xff);
	}
	else if(s0 == L"default.bold")
		fontshape[F_DEFAULT].bold = (StrToInt(s1)) ? 1 : 0;
	else if(s0 == L"default.italic")
		fontshape[F_DEFAULT].italic = (StrToInt(s1)) ? 1 : 0;
	// fatal
	else if(s0 == L"fatal.pt")
		fontshape[F_FATAL].pt = StrToInt(s1);
	else if(s0 == L"fatal.color") {
		int col				   = HexStrToInt(s1);
		fontshape[F_FATAL].col = ((col & 0xff) << 16) + (col & 0xff00) + ((col >> 16) & 0xff);
	}
	else if(s0 == L"fatal.bold")
		fontshape[F_FATAL].bold = (StrToInt(s1)) ? 1 : 0;
	else if(s0 == L"fatal.italic")
		fontshape[F_FATAL].italic = (StrToInt(s1)) ? 1 : 0;
	// error
	else if(s0 == L"error.pt")
		fontshape[F_ERROR].pt = StrToInt(s1);
	else if(s0 == L"error.color") {
		int col				   = HexStrToInt(s1);
		fontshape[F_ERROR].col = ((col & 0xff) << 16) + (col & 0xff00) + ((col >> 16) & 0xff);
	}
	else if(s0 == L"error.bold")
		fontshape[F_ERROR].bold = (StrToInt(s1)) ? 1 : 0;
	else if(s0 == L"error.italic")
		fontshape[F_ERROR].italic = (StrToInt(s1)) ? 1 : 0;
	// warning
	else if(s0 == L"warning.pt")
		fontshape[F_WARNING].pt = StrToInt(s1);
	else if(s0 == L"warning.color") {
		int col					 = HexStrToInt(s1);
		fontshape[F_WARNING].col = ((col & 0xff) << 16) + (col & 0xff00) + ((col >> 16) & 0xff);
	}
	else if(s0 == L"warning.bold")
		fontshape[F_WARNING].bold = (StrToInt(s1)) ? 1 : 0;
	else if(s0 == L"warning.italic")
		fontshape[F_WARNING].italic = (StrToInt(s1)) ? 1 : 0;
	// note
	else if(s0 == L"note.pt")
		fontshape[F_NOTE].pt = StrToInt(s1);
	else if(s0 == L"note.color") {
		int col				  = HexStrToInt(s1);
		fontshape[F_NOTE].col = ((col & 0xff) << 16) + (col & 0xff00) + ((col >> 16) & 0xff);
	}
	else if(s0 == L"note.bold")
		fontshape[F_NOTE].bold = (StrToInt(s1)) ? 1 : 0;
	else if(s0 == L"note.italic")
		fontshape[F_NOTE].italic = (StrToInt(s1)) ? 1 : 0;
	// background
	else if(s0 == L"background.color") {
		int col = HexStrToInt(s1);
		bkcol	= ((col & 0xff) << 16) + (col & 0xff00) + ((col >> 16) & 0xff);
	}
	// To change border color
	else if(s0 == L"border.color") {
		int col = HexStrToInt(s1);
		bdcol	= ((col & 0xff) << 16) + (col & 0xff00) + ((col >> 16) & 0xff);
		DwmSetWindowAttribute(hWnd, DWMWA_BORDER_COLOR, &bdcol, sizeof(bdcol));
	}
	// To change darkmode
	else if(s0 == L"darkmode") {
		darkmode		  = HexStrToInt(s1);
		BOOL darkmode_tmp = darkmode;
		DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkmode_tmp, sizeof(darkmode_tmp));
	}
	// To change border color
	else if(s0 == L"dialogbox.color") {
		int col = HexStrToInt(s1);
		dlgcol	= ((col & 0xff) << 16) + (col & 0xff00) + ((col >> 16) & 0xff);
		DeleteObject(DlgBrush);
		DlgBrush = CreateSolidBrush(dlgcol);
	}
	// AlertOnWarning
	else if(s0 == L"alert.onwarning")
		AlertOnWarning = (StrToInt(s1)) ? 1 : 0;
	// disable_auto_transfer_shiori_ownership
	else if(s0 == L"shiori_ownership.auto_transfer.disable")
		disable_auto_transfer_shiori_ownership = (StrToInt(s1)) ? 1 : 0;
	// face
	else if(s0 == L"face")
		fontface = s1;
	// window.x
	else if(s0 == L"window.x")
		wp.x = StrToInt(s1);
	// window.y
	else if(s0 == L"window.y")
		wp.y = StrToInt(s1);
	// window.width
	else if(s0 == L"window.width") {
		ws.cx = StrToInt(s1);
		if(ws.cx < 0)
			ws.cx = 0;
	}
	// window.height
	else if(s0 == L"window.height") {
		ws.cy = StrToInt(s1);
		if(ws.cy < 0)
			ws.cy = 0;
	}
	else
		return 0;
	return 1;
}

void SaveParameter(void) {
	// ファイルへ設定を書き出し
	FILE *fp = _wfopen(savefile_path.c_str(), L"wt, ccs=UTF-8");
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

		fwprintf(fp, L"shiori_ownership.auto_transfer.disable,%d\n", disable_auto_transfer_shiori_ownership);

		fwprintf(fp, L"face,%s\n", fontface.c_str());

		fwprintf(fp, L"darkmode,%d\n", darkmode);

		RECT rect;
		GetWindowRect(hWnd, &rect);

		fwprintf(fp, L"window.x,%ld\n", rect.left);
		fwprintf(fp, L"window.y,%ld\n", rect.top);
		fwprintf(fp, L"window.width,%ld\n", rect.right - rect.left + 1);
		fwprintf(fp, L"window.height,%ld\n", rect.bottom - rect.top + 1);

		fclose(fp);
	}
}
