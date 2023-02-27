#include <string>
#include <windows.h>
#include "header_files/tama.h"
#include "my-gists/STL/CutSpace.hpp"
using namespace std;

int HexStrToInt(wstring_view str) {
	// HEXのstrをintに変換
	size_t len	   = str.size();
	int result = 0;
	for(size_t i = 0; i < len; i++) {
		int digit = (int)str[i];

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

int StrToInt(wstring_view str) {
	// 10進数のstrをintに変換
	int result = 0;
	if(str[0] == L'-') {
		for(size_t i = 1; i < str.size(); i++) {
			result = result * 10 - (str[i] - L'0');
		}
	}
	else{
		for(size_t i = 0; i < str.size(); i++) {
			result = result * 10 + (str[i] - L'0');
		}
	}
	return result;
}

bool Split(wstring &str, wstring &s0, wstring &s1, wstring_view sepstr) {
	// strをs0とs1に分解
	auto begin = str.find(sepstr);
	s0		   = str.substr(0, begin);
	s1		   = str.substr(begin + sepstr.size());
	CutSpace(s0);
	CutSpace(s1);

	return begin != wstring::npos;
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
