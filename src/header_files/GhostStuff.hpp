#define DONT_USE_SOCKET
#include "my-gists/ukagaka/sstp.hpp"
#include "my-gists/ukagaka/SFMO.hpp"
#include "my-gists/ukagaka/shiori_loader.hpp"

void CshioriErrorHandler(Cshiori::Error);
void CshioriWarningHandler(Cshiori::Warning);

inline Cshiori shiori{
	Cshiori::error_logger_type{
		CshioriErrorHandler,
		CshioriWarningHandler
	}
};
inline SSTP_link_n::SSTP_Direct_link_t linker({{L"Charset", L"UTF-8"}, {L"Sender", L"tama"}});
inline SFMO_t						   fmobj;

void LostGhostLink()noexcept;

void reload_shiori_of_baseware();
void unload_shiori_of_baseware();

void UpdateGhostModulestate();

bool ExecLoad(void);
void ExecRequest(const wchar_t* str);
void ExecUnload() noexcept;
bool ExecReload();
