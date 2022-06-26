#include <string>
using namespace std;

int HexStrToInt(const wchar_t *str);
void CutSpace(wstring &str);
bool Split(wstring &str, wstring &s0, wstring &s1, const wstring sepstr);
wchar_t *setlocaleauto(int category);
