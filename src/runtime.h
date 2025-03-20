// for UI
#define LOWERCHAR(c) ((('A' <= (c)) && ((c) <= 'Z')) ? ((c) + 'a' - 'A') : (c))

// for SMAF
#define SwapDword(data) (*((BYTE*)(data) + 0) << 24 | *((BYTE*)(data) + 1) << 16 | *((BYTE*)(data) + 2) << 8 | *((BYTE*)(data) + 3))
#define SwapWord(data) (*((BYTE*)(data) + 0) << 8 | *((BYTE*)(data) + 1))

//#define LEtoBE(data) ((((data) >> 24) & 0xFF) | ((((data) >> 16) & 0xFF) << 8) | ((((data) >> 8) & 0xFF) << 16) | (((data) & 0xFF) << 24))
#define StoreBEDword(d, s)\
{\
 *((BYTE*)(d)+0) = (BYTE)(((s)>>24)&0xFF);\
 *((BYTE*)(d)+1) = (BYTE)(((s)>>16)&0xFF);\
 *((BYTE*)(d)+2) = (BYTE)(((s)>>8)&0xFF);\
 *((BYTE*)(d)+3) = (BYTE)((s)&0xFF);\
}
#define StoreBEWord(d, s)\
{\
 *((BYTE*)(d)+0) = (BYTE)(((s)>>8)&0xFF);\
 *((BYTE*)(d)+1) = (BYTE)((s)&0xFF);\
}

#define COMPARE4(d, s) ((d)[0] != (s)[0] || (d)[1] != (s)[1] || (d)[2] != (s)[2] || (d)[3] != (s)[3])

void Tracef(LPCTSTR str, ...);

int FillRectangle(HDC hdc, int sx, int sy, int ex, int ey, COLORREF cr);
BOOL Line(HDC hdc, int sx, int sy, int ex, int ey, COLORREF cr);
void* halloc(DWORD size);
void* hrealloc(void* ptr, DWORD size);
BOOL hfree(void* ptr);
void PutText(HDC hdc, LPCTSTR str, ...);
int memcomp(BYTE* p1, BYTE* p2, DWORD size);
void memcopy(BYTE* dst, BYTE* src, DWORD size);
DWORD strbytes(LPCTSTR str);
void strcopy(LPTSTR dst, LPCTSTR src);
LPCTSTR basename(LPCTSTR str);
LPCTSTR skipfname(LPCTSTR str);
LPCTSTR skipspace(LPCTSTR str);
BOOL addEndYen(LPTSTR str);
LPCTSTR getLastChar(LPCTSTR str, TCHAR chr);
LPCTSTR getNextChar(LPCTSTR str, TCHAR chr);
int strtoint(LPCTSTR str);
