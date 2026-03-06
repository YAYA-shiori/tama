// tamac.cpp
// YAYA ログを受け取り、標準出力に出力するコンソールツール
// Usage: tamac <yaya.dll>

#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#include <windows.h>
#include <stdio.h>
#include <string>

#include "my-gists/ukagaka/shiori_loader.cpp"

// WM_COPYDATA dwData values (tama.h より)
#define F_DEFAULT 0
#define F_FATAL   1
#define F_ERROR   2
#define F_WARNING 3
#define F_NOTE    4
#define F_NUMBER  5
#define E_SJIS    16
#define E_UTF8    17
#define E_DEFAULT 32

static const char* level_prefix[F_NUMBER] = {
	"",
	"[FATAL] ",
	"[ERROR] ",
	"[WARN]  ",
	"[NOTE]  ",
};

static HWND     g_hwnd      = NULL;
static Cshiori* g_shiori    = NULL;
static int      g_threshold = F_ERROR; // 検知対象の最低重要度（<=この値なら検知）
static bool     g_detected  = false;

static void output_log(const wchar_t* str, int wlen, int level) {
	if (level >= F_FATAL && level <= g_threshold)
		g_detected = true;
	if (level >= 0 && level < F_NUMBER)
		fputs(level_prefix[level], stdout);
	int mblen = WideCharToMultiByte(CP_UTF8, 0, str, wlen, NULL, 0, NULL, NULL);
	if (mblen > 0) {
		std::string buf(mblen, '\0');
		WideCharToMultiByte(CP_UTF8, 0, str, wlen, buf.data(), mblen, NULL, NULL);
		fwrite(buf.data(), 1, mblen, stdout);
	}
	fflush(stdout);
}

// Set_loghandler 経由で呼ばれるコールバック
static void loghandler_callback(const wchar_t* str, int mode, int /*id*/) {
	if (str) output_log(str, -1, mode);
}

// logsend (WM_COPYDATA) 経由のログ受信用ウィンドウプロシージャ
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	if (msg == WM_COPYDATA) {
		const COPYDATASTRUCT* cds = (COPYDATASTRUCT*)lp;
		if (cds->dwData < (ULONG_PTR)F_NUMBER && cds->cbData > 0) {
			output_log(
				(const wchar_t*)cds->lpData,
				(int)(cds->cbData / sizeof(wchar_t)),
				(int)cds->dwData);
		} else if (g_shiori) {
			if      (cds->dwData == E_SJIS)    g_shiori->SetCodePage(CODEPAGE_n::CP_SJIS);
			else if (cds->dwData == E_UTF8)    g_shiori->SetCodePage(CODEPAGE_n::CP_UTF8);
			else if (cds->dwData == E_DEFAULT) g_shiori->SetCodePage(CODEPAGE_n::CP_ACP);
		}
		return TRUE;
	}
	return DefWindowProc(hwnd, msg, wp, lp);
}

static void error_cb(Cshiori::Error err) {
	fprintf(stderr, "[tamac] %s\n", to_string(err).data());
}
static void warning_cb(Cshiori::Warning warn) {
	fprintf(stderr, "[tamac] %s\n", to_string(warn).data());
}

static int parse_level(const wchar_t* s) {
	if      (!_wcsicmp(s, L"fatal"))   return F_FATAL;
	else if (!_wcsicmp(s, L"error"))   return F_ERROR;
	else if (!_wcsicmp(s, L"warning")) return F_WARNING;
	else if (!_wcsicmp(s, L"note"))    return F_NOTE;
	return -1;
}

int wmain(int argc, wchar_t* argv[]) {
	if (argc < 2) {
		fwprintf(stderr, L"Usage: tamac <yaya.dll> [-l fatal|error|warning|note]\n");
		return 1;
	}

	for (int i = 2; i < argc; i++) {
		if (!_wcsicmp(argv[i], L"-l") && i + 1 < argc) {
			int lv = parse_level(argv[++i]);
			if (lv < 0) {
				fwprintf(stderr, L"Unknown level: %s\n", argv[i]);
				return 1;
			}
			g_threshold = lv;
		}
	}

	SetConsoleOutputCP(CP_UTF8);

	// logsend (WM_COPYDATA) 受信用の隠しウィンドウを作成
	HINSTANCE hInst = GetModuleHandle(NULL);
	WNDCLASSEX wc{};
	wc.cbSize        = sizeof(wc);
	wc.lpfnWndProc   = WndProc;
	wc.hInstance     = hInst;
	wc.lpszClassName = L"tamacMsgWnd";
	RegisterClassEx(&wc);

	g_hwnd = CreateWindowEx(0, L"tamacMsgWnd", L"",
	                        0, 0, 0, 0, 0,
	                        HWND_MESSAGE, NULL, hInst, NULL);

	Cshiori shiori{Cshiori::error_logger_type{error_cb, warning_cb}};
	g_shiori = &shiori;

	// Set_loghandler はDLLロード前でも設定可能（SetTo内で適用される）
	shiori.Set_loghandler(loghandler_callback);
	shiori.set_logsend_hwnd(g_hwnd);
	shiori.SetTo(argv[1]);

	// load() は同期完了・WM_COPYDATA も同一スレッドの SendMessage で処理済みなので即終了
	g_shiori = NULL;
	// shiori のデストラクタが Dounload() を呼ぶ
	if (!shiori.All_OK()) return 1;
	return g_detected ? 2 : 0;
}
