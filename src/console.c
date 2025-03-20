#include <windows.h>
#include "console.h"

int getone(void)
{
	DWORD im;
	BYTE tst;
	DWORD rea;
	HANDLE stdin;

	stdin = GetStdHandle(STD_INPUT_HANDLE);

	GetConsoleMode(stdin, &im);
	SetConsoleMode(stdin, im & ~ENABLE_LINE_INPUT & ~ENABLE_ECHO_INPUT);
	if(!ReadFile(stdin, &tst, 1, &rea, NULL))
		rea = 0;
	SetConsoleMode(stdin, im);

	if(rea == 0)
		return -1;
	else
		return tst;
}

int geteone(void)
{
	int g = getone();

	if(g == -1)
		return -1;
	else
	{
		if(' ' <= g)
		{
			TCHAR str[2] = {g, '\0'};

			putstr(str);
		}

		return g;
	}
}

void putstr(LPCTSTR str)
{
	DWORD wrote;

	WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), str, lstrlen(str), &wrote, NULL);

	return;
}

void putstrf(LPCTSTR str, ...)
{
	TCHAR ts[1024];
	va_list va;

	va_start(va, str);

	wvsprintf(ts, str, va);
	putstr(ts);

	va_end(va);

	return;
}

void setcolor(WORD flags)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), flags);

	return;
}
