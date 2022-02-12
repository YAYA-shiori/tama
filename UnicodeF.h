// http://kamoland.com/comp/unicode.html

// revised by umeici. 2004
// ・setlocaleを拡張して各国MBCSに対応

#ifndef INCLUDED_UNICODEF_H
#define INCLUDED_UNICODEF_H

#include <windows.h>

class CUnicodeF {
public:
    static wchar_t* sjis_to_utf16be(const char *pAnsiStr, int *nBytesOut, char charset);
    static char* utf16be_to_utf8(const wchar_t *pUcsStr, int *nBytesOut);
    static char* utf16be_to_sjis(const wchar_t *pUcsStr, int *nBytesOut, char charset);
    static wchar_t* utf8_to_utf16be(const char *pUtf8Str, int *nNumOut, BOOL bBigEndian);

private:
    static int utf16be_to_utf8_sub( char *pUtf8, const wchar_t *pUcs2, int nUcsNum, BOOL bCountOnly);
    static int utf8_to_utf16be_sub( wchar_t *pUcs2, const char *pUtf8, int nUtf8Num, BOOL bCountOnly, BOOL bBigEndian);
};

#endif
