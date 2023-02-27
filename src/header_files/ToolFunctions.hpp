#include <string>
using namespace std;

int		 HexStrToInt(wstring_view str);
int		 StrToInt(wstring_view str);
bool	 Split(wstring &str, wstring &s0, wstring &s1, wstring_view sepstr);
wchar_t *setlocaleauto(int category);
