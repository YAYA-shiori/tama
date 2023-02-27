void EOS(int newaddsz);

void WriteText(wstring_view text, int level);

bool SetFontShape(int shapeid);
bool SetFontShapeInit(int shapeid);
int	 GetFontCharSet(wstring_view name);
bool SetMyFont(const wchar_t *facename, int shapeid, int scf);
bool SetMyBkColor(COLORREF col);
bool SetDlgFont(HWND hDlgEdit);
bool SetDlgBkColor(HWND hDlgEdit, COLORREF col);

int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int nFontType, LPARAM lParam);
