#include <Richedit.h>
#include "header_files/tama.h"
#include "header_files/UItools.hpp"

bool SetFontShape(int shapeid) {
	// IDでフォント属性を設定
	return SetMyFont(fontface.c_str(), shapeid, SCF_SELECTION);
}

bool SetFontShapeInit(int shapeid) {
	// IDでフォント属性を初期化
	return SetMyFont(fontface.c_str(), shapeid, SCF_ALL);
}

int GetFontCharSet(wstring name) {
	// name名のフォントのcharsetを返す
	for(vector<SFface>::iterator it = fontarray.begin(); it != fontarray.end(); it++) {
		if(it->face == name)
			return it->charset;
	}
	return -1;
}

bool SetMyFont(const wchar_t *facename, int shapeid, int scf) {
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
		return false;

	SetFocus(hEdit);
	return true;
}

bool SetDlgFont(HWND hDlgEdit) {
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
		return false;

	SetFocus(hDlgEdit);
	return true;
}

bool SetMyBkColor(COLORREF col) {
	// 背景色をcolに設定
	if(SendMessage(hEdit, EM_SETBKGNDCOLOR, 0, (LPARAM)col) == 0)
		return false;
	return true;
}

bool SetDlgBkColor(HWND hDlgEdit, COLORREF col) {
	// 背景色をcolに設定
	if(SendMessage(hDlgEdit, EM_SETBKGNDCOLOR, 0, (LPARAM)col) == 0)
		return false;
	return true;
}

int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int nFontType, LPARAM lParam) {
	// フォント列挙用のコールバック関数
	SFface addface;
	addface.face	= lpelfe->elfLogFont.lfFaceName;
	addface.charset = lpelfe->elfLogFont.lfCharSet;
	fontarray.push_back(addface);

	return 1;
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
