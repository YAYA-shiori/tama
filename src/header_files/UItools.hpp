void EOS(int newaddsz)noexcept;

void WriteText(wstring_view text, int level) noexcept;

bool SetFontShape(int shapeid) noexcept;
bool SetFontShapeInit(int shapeid) noexcept;
int	 GetFontCharSet(wstring_view name)noexcept;
bool SetMyFont(const wchar_t *facename, int shapeid, int scf)noexcept;
bool SetMyBkColor(COLORREF col)noexcept;
bool SetDlgFont(HWND hDlgEdit)noexcept;
bool SetDlgBkColor(HWND hDlgEdit, COLORREF col)noexcept;

int CALLBACK EnumFontFamExProc(const ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int nFontType, LPARAM lParam);
