void On_yatamaOpen(HWND hEdit);
void On_yatamaExit(HWND hEdit);

#define DONT_USE_SOCKET
#include "my-gists/ukagaka/sstp.hpp"
inline SSTP_link_n::SSTP_Direct_link_t linker(SSTP_link_n::SSTP_link_args_t{{L"Charset", L"UTF-8"}, {L"Sender", L"yatama"}});
