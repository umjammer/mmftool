#include <windows.h>
#include "runtime.h"

void Tracef(LPCTSTR str, ...)
{
#ifdef _DEBUG
	va_list va;
	TCHAR ts[1024];

	va_start(va, str);

	wvsprintf(ts, str, va);

	OutputDebugString(ts);

	va_end(va);

#endif
	return;
}

// see http://hp.vector.co.jp/authors/VA000092/win32/vc-tech.html
#ifdef _DEBUG
const TCHAR _stack_level_unmatch[] = TEXT("Stack level doesn't match!\ncalling convention is right?");
void __declspec(naked) _chkesp()
{
    __asm {
        jz ok;
        push offset _stack_level_unmatch;
        call dword ptr OutputDebugString;
        call dword ptr DebugBreak;
ok:
        ret;
    };
}
#endif

// BOOLじゃね?
int FillRectangle(HDC hdc, int sx, int sy, int ex, int ey, COLORREF cr)
{
	HBRUSH hbrs;
	int ret;
	RECT rc = {sx, sy, ex, ey};

	hbrs = CreateSolidBrush(cr);
	ret = FillRect(hdc, &rc, hbrs);
	DeleteObject(hbrs);

	return ret;
}

BOOL Line(HDC hdc, int sx, int sy, int ex, int ey, COLORREF cr)
{
	HPEN prepen;
	POINT prept;
	BOOL ret;

	prepen = SelectObject(hdc, CreatePen(PS_SOLID, 1, cr));
	MoveToEx(hdc, sx, sy, &prept);
	ret = LineTo(hdc, ex, ey);
	MoveToEx(hdc, prept.x, prept.y, NULL);
	DeleteObject(SelectObject(hdc, prepen));

	return ret;
}

void* halloc(DWORD size)
{
	return HeapAlloc(GetProcessHeap(), 0, size);
}

void* hrealloc(void* ptr, DWORD size)
{
	if(!ptr)
		return halloc(size);
	else
		return HeapReAlloc(GetProcessHeap(), 0, ptr, size);
}

BOOL hfree(void* ptr)
{
	return HeapFree(GetProcessHeap(), 0, ptr);
}

void PutText(HDC hdc, LPCTSTR str, ...)
{
	POINT p;
	SIZE s;
	va_list va;
	TCHAR ts[1024];

	va_start(va, str);

	wvsprintf(ts, str, va);

	GetCurrentPositionEx(hdc, &p);
	GetTextExtentPoint32(hdc, ts, lstrlen(ts), &s);

	// 逆にコストかかりそうだ
#if 0
	{
		HBITMAP hbm;
		BITMAP bm;

		hbm = GetCurrentObject(hdc, OBJ_BITMAP);
		GetObject(hbm, sizeof(bm), &bm);

		if(bm.bmHeight < p.y)
			goto end;
		if(p.y + s.cy < 0)
			goto end;
	}
#endif

	TextOut(hdc, p.x, p.y, ts, lstrlen(ts));

//end:
	MoveToEx(hdc, p.x, p.y + s.cy, NULL);

	va_end(va);

	return;
}

int memcomp(BYTE* p1, BYTE* p2, DWORD size)
{
	while(size && *p1++ == *p2++)
		size--;

	if(!size)
		return 0;
	if(*p1 < *p2)
		return -1;
	else
		return 1;
}

void memcopy(BYTE* dst, BYTE* src, DWORD size)
{
	while(size--)
		*dst++ = *src++;

	return;
}

DWORD strbytes(LPCTSTR str)
{
	DWORD bytes = 0;

	while(*str)
	{
		bytes += CharNext(str) - str;
		str = CharNext(str);
	}

	return bytes;
}

void strcopy(LPTSTR dst, LPCTSTR src)
{
	memcopy((BYTE*)dst, (BYTE*)src, strbytes(src));

	return;
}

LPCTSTR basename(LPCTSTR str)
{
	LPCTSTR last = str;

	while(*str)
	{
		if(*str == '\\')
			last = str + 1;
		str = CharNext(str);
	}

	return last;
}

LPCTSTR skipfname(LPCTSTR str)
{
	if(!str)
		return NULL;

	if(*str == '\0')
		return str;

	if(*str == '"')
	{
		str = CharNext(str);
		while(*str)
		{
			if(*str == '"')
				break;
			str = CharNext(str);
		}
		str = CharNext(str);
	}else
	{
		while(*str && *str != ' ')
			str = CharNext(str);
	}

	return str;
}

LPCTSTR skipspace(LPCTSTR str)
{
	if(*str == ' ')
		str = CharNext(str);

	return str;
}

BOOL addEndYen(LPTSTR str)
{
	LPTSTR p;
	int chrs, lastyen;

	for(p = str, chrs = 0, lastyen = 0; *p; p = CharNext(p), chrs++)
	{
		if(*p == TEXT('\\'))
			lastyen = chrs;
	}

	if(chrs - 1 > lastyen)
	{
		lstrcat(str, TEXT("\\"));
		return TRUE;
	}

	return FALSE;
}

LPCTSTR getLastChar(LPCTSTR str, TCHAR chr)
{
	LPCTSTR p = NULL;

	while(*str)
	{
		if(*str == chr)
			p = str;
		str = CharNext(str);
	}

	return p;
}

LPCTSTR getNextChar(LPCTSTR str, TCHAR chr)
{
	while(*str)
	{
		if(*str == chr)
			return str;
		str = CharNext(str);
	}

	return NULL;
}

int strtoint(LPCTSTR str)
{
	int val = 0;
	BOOL minus = FALSE;

	if(*str == '-')
	{
		minus = TRUE;
		str++;
	}

	while(*str)
	{
		if(*str < '0' || '9' < *str)
			break;
		val *= 10;
		val += *str - '0';

		str++;
	}

	if(minus)
		val = -val;

	return val;
}
