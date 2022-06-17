#include <wtypes.h>
#include <string>
const wchar_t *LoadCStringFromResource(
	__in UINT		   stringID,
	__in_opt HINSTANCE instance = NULL);
std::wstring LoadStringFromResource(
	__in UINT		   stringID,
	__in_opt HINSTANCE instance = NULL);
