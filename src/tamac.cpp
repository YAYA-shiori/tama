// tamac.cpp
// YAYA ログを受け取り、標準出力に出力するコンソールツール
// Usage: tamac <yaya.dll> [-l fatal|error|warning|note] [--ci]
//
// GitHub Actions アノテーション出力（--ci モード）は以下を参考にしました:
//   check-tool.cpp - YAYA-shiori/yaya-CI-check
//   https://github.com/YAYA-shiori/yaya-CI-check

#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#include <windows.h>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <regex>
#include <fcntl.h>
#include <io.h>

#include "my-gists/ukagaka/shiori_loader.cpp"

// ログレベル定数（check-tool.cpp に合わせて統一）
#define E_I       0   /* info */
#define E_F       1   /* fatal */
#define E_E       2   /* error */
#define E_W       3   /* warning */
#define E_N       4   /* note */
#define E_J       5   /* other(j) */
#define E_END     6   /* ログの終了 */
#define E_SJIS   16   /* SJIS */
#define E_UTF8   17   /* UTF-8 */
#define E_DEFAULT 32  /* OS デフォルト */

#define E_LEVEL_MAX 5 /* 重大度レベルの数（prefix 配列サイズ）*/

static const wchar_t* s_level_prefix[E_LEVEL_MAX] = {
	L"",
	L"[FATAL] ",
	L"[ERROR] ",
	L"[WARN]  ",
	L"[NOTE]  ",
};

static HWND     g_hwnd      = NULL;
static Cshiori* g_shiori    = NULL;
static int      g_threshold = E_E;   // 検知対象の最低重大度（<=この値なら検知）
static bool     g_detected  = false;
static bool     g_ci_mode   = false; // GitHub Actions アノテーション出力モード

// ログメッセージを解析・出力する
// --ci 時は GitHub Actions アノテーション形式（check-tool.cpp を参考に実装）
// 通常時は既存の tamac 形式をそのまま維持
static void process_log(const wchar_t* stra, int mode, int id) {
	using namespace std;
	if (!stra) return;

	// 検知フラグ（しきい値以下の重大度は検知対象）
	if (mode >= E_F && mode <= g_threshold)
		g_detected = true;

	if (g_ci_mode) {
		// --- GitHub Actions アノテーション出力（check-tool.cpp を参考に実装）---
		wstring str = stra;
		size_t linenum = 0;
		wstring info, type, filename;
		{
			wsmatch result;
			// 例: path\to\file.dic(17) : error E0041 : メッセージ
			if (regex_search(str, result, wregex(L"\\((\\d+|-)\\) : "))) {
				filename = str.substr(0, result.position());
				{
					wstring lns = result[0]; // "(17) : "
					lns = lns.substr(1, lns.size() - 4); // "17"
					linenum = (size_t)wcstoll(lns.c_str(), nullptr, 10);
				}
				info = str.substr(result.position() + result.length());
				if (regex_search(info, result, wregex(L" *([WEN])(\\d+|-)( *: |：)"))) {
					type = info.substr(0, result.position());
					info = info.substr(result.position() + result.length());
				}
			}
			switch (mode) {
			case E_F: if (type.empty()) type = L"fatal";   goto common;
			case E_E: if (type.empty()) type = L"error";   goto common;
			case E_W: if (type.empty()) type = L"warning"; goto common;
			case E_N: if (type.empty()) type = L"notice";  goto common;
			common:
				if (info.empty()) info = str;
				if (regex_search(info, result, wregex(L"^// *")))
					info = info.substr(result.length());
			}
		}

		static bool in_dic_load = false, in_request_end = false;
		switch (mode) {
		case E_SJIS: case E_UTF8: case E_DEFAULT: case E_END: break;
		case E_I:
			if (str == L"// request\n")
				fputws(L"::group::request call\n", stdout);
			else if (!in_request_end && str.size() >= 30 && str.substr(0, 30) == L"// response (Execution time : ")
				in_request_end = true;
			else if (in_request_end && str == L"\n") {
				in_request_end = false;
				fputws(L"::endgroup::\n", stdout);
			}
			if (in_dic_load && id == 8) { // dic load end
				in_dic_load = false;
				fputws(L"::endgroup::\n", stdout);
			}
			fputws(str.data(), stdout);
			if (id == 3) { // dic load begin
				in_dic_load = true;
				fputws(L"::group::dic load list\n", stdout);
			}
			break;
		case E_F:
			fputws(str.data(), stderr);
			fputws((L"::error file=" + filename + L",line=" + to_wstring(linenum) + L",title=" + type + L"::" + info + L"\n").data(), stderr);
			break;
		case E_E:
			if (id != 57) { // E0057 は CI チェックで常に無視
				fputws(str.data(), stderr);
				if (id != 10) // id==10 は緊急モード
					fputws((L"::error file=" + filename + L",line=" + to_wstring(linenum) + L",title=" + type + L"::" + info + L"\n").data(), stderr);
				else
					fputws(L"::error title=Emergency mode::Goes into emergency mode\n", stderr);
			} else {
				fputws(str.data(), stdout);
				fputws(L"// from CI checker: E0057 is always ignored in CI check\n", stdout);
			}
			break;
		case E_W:
			fputws(str.data(), stderr);
			fputws((L"::warning file=" + filename + L",line=" + to_wstring(linenum) + L",title=" + type + L"::" + info + L"\n").data(), stderr);
			break;
		case E_N:
			if (id != 0) { // N0000 は CI チェックで常に無視
				fputws(str.data(), stderr);
				fputws((L"::notice file=" + filename + L",line=" + to_wstring(linenum) + L",title=" + type + L"::" + info + L"\n").data(), stderr);
			} else {
				fputws(str.data(), stdout);
				fputws(L"// from CI checker: N0000 is always ignored in CI check\n", stdout);
			}
			break;
		case E_J:
			fputws(str.data(), stdout);
			break;
		}
	} else {
		// --- 通常出力（既存 tamac 形式）---
		switch (mode) {
		case E_SJIS: case E_UTF8: case E_DEFAULT: case E_END: return;
		}
		if (mode >= E_F && mode > g_threshold) return;
		FILE* out = (mode >= E_F && mode <= E_W) ? stderr : stdout;
		if (mode >= 0 && mode < E_LEVEL_MAX)
			fputws(s_level_prefix[mode], out);
		fputws(stra, out);
	}
}

// Set_loghandler 経由で呼ばれるコールバック
static void loghandler_callback(const wchar_t* str, int mode, int id) {
	process_log(str, mode, id);
}

// logsend (WM_COPYDATA) 経由のログ受信用ウィンドウプロシージャ
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	if (msg == WM_COPYDATA) {
		const COPYDATASTRUCT* cds = (COPYDATASTRUCT*)lp;
		if (cds->dwData < (ULONG_PTR)E_LEVEL_MAX && cds->cbData > 0) {
			// WM_COPYDATA のデータは null 終端を持たないため wstring 経由で渡す
			int wlen = (int)(cds->cbData / sizeof(wchar_t));
			std::wstring buf((const wchar_t*)cds->lpData, wlen);
			process_log(buf.c_str(), (int)cds->dwData, 0);
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
	fwprintf(stderr, L"[tamac] %S\n", to_string(err).data());
}
static void warning_cb(Cshiori::Warning warn) {
	fwprintf(stderr, L"[tamac] %S\n", to_string(warn).data());
}

static int parse_level(const wchar_t* s) {
	if      (!_wcsicmp(s, L"fatal"))   return E_F;
	else if (!_wcsicmp(s, L"error"))   return E_E;
	else if (!_wcsicmp(s, L"warning")) return E_W;
	else if (!_wcsicmp(s, L"note"))    return E_N;
	return -1;
}

int wmain(int argc, wchar_t* argv[]) {
	if (argc < 2) {
		fwprintf(stderr, L"Usage: tamac <yaya.dll> [-l fatal|error|warning|note] [--ci]\n");
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
		} else if (!_wcsicmp(argv[i], L"--ci")) {
			g_ci_mode = true;
		}
	}

	// GitHub Actions 環境では自動的に CI モードへ
	if (!g_ci_mode && getenv("GITHUB_ACTIONS"))
		g_ci_mode = true;

	// wide 文字列 I/O を UTF-8 テキストモードに統一（check-tool.cpp と同様）
	void(_setmode(_fileno(stderr), _O_U8TEXT));
	void(_setmode(_fileno(stdout), _O_U8TEXT));
	void(_setmode(_fileno(stdin),  _O_U8TEXT));

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

	// Set_loghandler は DLL ロード前でも設定可能（SetTo 内で適用される）
	shiori.Set_loghandler(loghandler_callback);
	shiori.set_logsend_hwnd(g_hwnd);
	shiori.SetTo(argv[1]);

	// load() は同期完了・WM_COPYDATA も同一スレッドの SendMessage で処理済みなので即終了
	g_shiori = NULL;
	// shiori のデストラクタが Dounload() を呼ぶ
	if (!shiori.All_OK()) return EXIT_FAILURE;

	// CI モードかつ対応 SHIORI なら CI_check_failed() による判定も行う（check-tool.cpp 参照）
	if (g_ci_mode && shiori.can_make_CI_check()) {
		auto failed = shiori.CI_check_failed();
		if (failed)
			fputws(L"::error title=open your tama!::some error in your dic\n", stdout);
		return failed ? EXIT_FAILURE : EXIT_SUCCESS;
	}

	return g_detected ? 2 : EXIT_SUCCESS;
}
