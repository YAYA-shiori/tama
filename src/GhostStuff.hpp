#define DONT_USE_SOCKET
#include "my-gists/ukagaka/sstp.hpp"
#include "my-gists/ukagaka/SFMO.hpp"
#include "my-gists/ukagaka/shiori_loader.hpp"

inline Cshiori						   shiori;
inline SSTP_link_n::SSTP_Direct_link_t linker({{L"Charset", L"UTF-8"}, {L"Sender", L"tama"}});
inline SFMO_t						   fmobj;

void LostGhostLink();

void reload_shiori_of_baseware();
void unload_shiori_of_baseware();

std::wstring get_shiori_path(std::wstring ghost_path);
void UpdateGhostModulestate();

bool ExecLoad(void);
void ExecRequest(const wchar_t* str);
void ExecUnload();
bool ExecReload();
