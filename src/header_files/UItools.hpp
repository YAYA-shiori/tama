void EOS(int newaddsz);

bool SetFontShape(int shapeid);
bool SetFontShapeInit(int shapeid);
int	 GetFontCharSet(wstring name);
bool SetMyFont(const wchar_t *facename, int shapeid, int scf);
bool SetMyBkColor(COLORREF col);
bool SetDlgFont(HWND hDlgEdit);
bool SetDlgBkColor(HWND hDlgEdit, COLORREF col);

int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int nFontType, LPARAM lParam);
