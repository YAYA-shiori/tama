void On_tamaOpen(HWND hEdit);
void On_tamaExit(HWND hEdit);

#define DONT_USE_SOCKET
#include "my-gists/ukagaka/sstp.hpp"
inline SSTP_link_n::SSTP_Direct_link_t linker(SSTP_link_n::SSTP_link_args_t{{L"Charset", L"UTF-8"}, {L"Sender", L"tama"}});
