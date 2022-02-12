// http://kamoland.com/comp/unicode.html

// revised by umeici. 2004
// ・setlocaleを拡張して各国MBCSに対応

#include "StdAfx.h"
#include "unicodeF.h"

char *setlocaleauto(int category);

/* -----------------------------------------------------------------------
 *  関数名  ：  CUnicodeF::utf16be_to_sjis
 *  機能概要：  UTF-16BE -> SJIS へ文字列のコード変換
 *  引数    ：  (I)  wchar_t *pUcsStr - 変換元UTF16BE文字列へのポインタ
 *              (O)  int *nBytesOut - 変換後文字列のバイト数
 *  戻り値  ：  char* 変換後SJIS文字列へのポインタ
 *
 *  処理概要：
 * -----------------------------------------------------------------------
 */
char *CUnicodeF::utf16be_to_sjis(const wchar_t *pUcsStr, int *nBytesOut, char charset)
{
    char *pAnsiStr = NULL;
    int nLen;

    if (!pUcsStr) return NULL;

	if (charset == CHARSET_SJIS)
		setlocale(LC_ALL, "Japanese");
	else
		setlocaleauto(LC_ALL);

    nLen = wcslen( pUcsStr);

    if ( pUcsStr[0] == (wchar_t)0xfeff || pUcsStr[0] == (wchar_t)0xfffe) {
        pUcsStr++; // 先頭にBOM(byte Order Mark)があれば，スキップする
        nLen--;
    }

    pAnsiStr = (char *)calloc((nLen+1), sizeof(wchar_t));
    if (!pAnsiStr) return NULL;

    // 1文字ずつ変換する。
    // まとめて変換すると、変換不能文字への対応が困難なので
    int nRet, i, nMbpos = 0;
    char *pcMbchar = new char[MB_CUR_MAX];

    for ( i=0; i < nLen; i++) {
        nRet = wctomb( pcMbchar, pUcsStr[i]);
        switch ( nRet) {
        case 1:
            pAnsiStr[nMbpos++] = pcMbchar[0];
            break;

        case 2:
            pAnsiStr[nMbpos++] = pcMbchar[0];
            pAnsiStr[nMbpos++] = pcMbchar[1];
            break;

        default: // 変換不能
            pAnsiStr[nMbpos++] = ' ';
            break;
        }
    }
    pAnsiStr[nMbpos] = '\0';

    delete [] pcMbchar;

    *nBytesOut = nMbpos;

    return pAnsiStr;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CUnicodeF::sjis_to_utf16be
 *  機能概要：  SJIS -> UTF-16 へ文字列のコード変換
 *  引数    ：  (I)    char *pAnsiStr - 変換元SJIS文字列へのポインタ
 *              (O)    int *nBytesOut - 変換後文字列のバイト数
 *  戻り値  ：  wchar_t* 変換後UTF-16BE文字列へのポインタ
 *
 *  処理概要：
 * -----------------------------------------------------------------------
 */
wchar_t *CUnicodeF::sjis_to_utf16be(const char *pAnsiStr, int *nBytesOut, char charset)
{
    int len;
    wchar_t *pUcsStr = NULL;

    if (!pAnsiStr) return NULL;

	if (charset == CHARSET_SJIS)
		setlocale(LC_ALL, "Japanese");
	else
		setlocaleauto(LC_ALL);

    len = strlen(pAnsiStr);
    *nBytesOut = sizeof(wchar_t)*(len);

    pUcsStr = (wchar_t *)calloc(*nBytesOut + 2, sizeof(char));
    if (!pUcsStr) return NULL;

	if (len)
		mbstowcs(pUcsStr, pAnsiStr, len+1);
	else
		*pUcsStr = L'\0';

    return pUcsStr;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CUnicodeF::utf16be_to_utf8
 *  機能概要：  UTF-16 -> UTF-8 へ文字列のコード変換
 *  引数    ：  (I) wchar_t *pUcsStr - 変換元UTF-16BE文字列へのポインタ
 *              (O) int *nBytesOut - 変換後文字列のバイト数
 *  戻り値  ：  char* 変換後UTF-8文字列へのポインタ
 *
 *  処理概要：
 * -----------------------------------------------------------------------
 */
char *CUnicodeF::utf16be_to_utf8(const wchar_t *pUcsStr, int *nBytesOut)
{
    int nUcsNum;
    char *pUtf8Str;

    nUcsNum = wcslen(pUcsStr);

//    *nBytesOut = utf16be_to_utf8_sub( NULL, pUcsStr, nUcsNum, TRUE);

//    pUtf8Str = (char *)calloc(*nBytesOut + 3, 1);
//    utf16be_to_utf8_sub( pUtf8Str, pUcsStr, nUcsNum, FALSE);

	// ワイド文字1文字2バイトが最大で3バイトになるので、基本的に文字数*3バイト確保すれば間に合う
    pUtf8Str = (char *)calloc(nUcsNum*3/* = sizeof(wchar_t) + 1*/ + 4, sizeof(char));
    *nBytesOut = utf16be_to_utf8_sub( pUtf8Str, pUcsStr, nUcsNum, FALSE);

    return pUtf8Str;
}

// Unicode(UTF-16) -> UTF-8 下請け
int CUnicodeF::utf16be_to_utf8_sub( char *pUtf8, const wchar_t *pUcs2, int nUcsNum, BOOL bCountOnly)
{
    int nUcs2, nUtf8 = 0;

    for ( nUcs2=0; nUcs2 < nUcsNum; nUcs2++) {
        if ( (unsigned short)pUcs2[nUcs2] <= 0x007f) {
            if ( bCountOnly == FALSE) {
                pUtf8[nUtf8] = (pUcs2[nUcs2] & 0x007f);
            }
            nUtf8 += 1;
        } else if ( (unsigned short)pUcs2[nUcs2] <= 0x07ff) {
            if ( bCountOnly == FALSE) {
                pUtf8[nUtf8] = ((pUcs2[nUcs2] & 0x07C0) >> 6 ) | 0xc0; // 2002.08.17 修正
                pUtf8[nUtf8+1] = (pUcs2[nUcs2] & 0x003f) | 0x80;
            }
            nUtf8 += 2;
        } else {
            if ( bCountOnly == FALSE) {
                pUtf8[nUtf8] = ((pUcs2[nUcs2] & 0xf000) >> 12) | 0xe0; // 2002.08.04 修正
                pUtf8[nUtf8+1] = ((pUcs2[nUcs2] & 0x0fc0) >> 6) | 0x80;
                pUtf8[nUtf8+2] = (pUcs2[nUcs2] & 0x003f) | 0x80;
            }
            nUtf8 += 3;
        }
    }
    if ( bCountOnly == FALSE) {
        pUtf8[nUtf8] = '\0';
    }

    return nUtf8;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CUnicodeF::utf8_to_utf16be
 *  機能概要：  UTF-8 -> UTF-16(BE/LE) へ文字列のコード変換
 *  引数    ：  (I) char *pUtf8Str - 変換元UTF-8文字列へのポインタ
 *              (O) int *nNumOut - 変換結果のUTF-16文字数．Byte数ではない
 *              (I) BOOL bBigEndian - ビッグエンディアンに変換するならTRUE
 *
 *  戻り値  ：  wchar_t* 変換後文字列へのポインタ
 *
 *  処理概要：
 * -----------------------------------------------------------------------
 */
wchar_t *CUnicodeF::utf8_to_utf16be(const char *pUtf8Str, int *nNumOut, BOOL bBigEndian)
{
    int nUtf8Num;
    wchar_t *pUcsStr;

    nUtf8Num = strlen(pUtf8Str); // UTF-8文字列には，'\0' がない

//    *nNumOut = utf8_to_utf16be_sub( NULL, pUtf8Str, nUtf8Num, TRUE, bBigEndian);

//    pUcsStr = (wchar_t *)calloc((*nNumOut + 1), sizeof(wchar_t));
//    utf8_to_utf16be_sub( pUcsStr, pUtf8Str, nUtf8Num, FALSE, bBigEndian);

	// すべて1バイト文字だったとするとこれが2バイトになるので、基本的に倍確保すれば間に合う
	pUcsStr = (wchar_t *)calloc(nUtf8Num+ 2, sizeof(wchar_t));
	*nNumOut = utf8_to_utf16be_sub( pUcsStr, pUtf8Str, nUtf8Num, FALSE, bBigEndian);

    return pUcsStr;
}

// UTF-8 -> Unicode(UCS-2) 下請け
int CUnicodeF::utf8_to_utf16be_sub( wchar_t *pUcs2, const char *pUtf8, int nUtf8Num,
                          BOOL bCountOnly, BOOL bBigEndian)
{
    int nUtf8, nUcs2 = 0;
    char cHigh, cLow;

    for ( nUtf8=0; nUtf8 < nUtf8Num;) {
        if ( ( pUtf8[nUtf8] & 0x80) == 0x00) { // 最上位ビット = 0
            if ( bCountOnly == FALSE) {
                pUcs2[nUcs2] = ( pUtf8[nUtf8] & 0x7f);
            }
            nUtf8 += 1;
        } else if ( ( pUtf8[nUtf8] & 0xe0) == 0xc0) { // 上位3ビット = 110
            if ( bCountOnly == FALSE) {
                pUcs2[nUcs2] = ( pUtf8[nUtf8] & 0x1f) << 6;
                pUcs2[nUcs2] |= ( pUtf8[nUtf8+1] & 0x3f);
            }
            nUtf8 += 2;
        } else {
            if ( bCountOnly == FALSE) {
                pUcs2[nUcs2] = ( pUtf8[nUtf8] & 0x0f) << 12;
                pUcs2[nUcs2] |= ( pUtf8[nUtf8+1] & 0x3f) << 6;
                pUcs2[nUcs2] |= ( pUtf8[nUtf8+2] & 0x3f);
            }
            nUtf8 += 3;
        }

        if ( bCountOnly == FALSE) {
            if ( !bBigEndian) {
                // リトルエンディアンにする処理
                cHigh = (pUcs2[nUcs2] & 0xff00) >> 8;
                cLow = (pUcs2[nUcs2] & 0x00ff);
                pUcs2[nUcs2] = (cLow << 8) | cHigh;
            }
        }

        nUcs2 += 1;
    }
    if ( bCountOnly == FALSE) {
        pUcs2[nUcs2] = L'\0';
    }

    return nUcs2;
}
