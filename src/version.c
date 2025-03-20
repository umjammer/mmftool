#include <windows.h>
#include "runtime.h"
#include "version.h"

BOOL getFileVersion(VERSION* v, LPCTSTR modname)
{
	char* vi;
	DWORD size;

	v->major = 0;
	v->minor = 0;
	v->patch = 0;
	v->build = 0;

	size = GetFileVersionInfoSize(modname, 0);
	if(!(vi = halloc(size)))
		return FALSE;

	GetFileVersionInfo(modname, 0, size, vi);
	{
		VS_FIXEDFILEINFO* ver;

		if(!VerQueryValue(vi, "\\", &ver, NULL))
		{
			hfree(vi);
			return FALSE;
		}

		// name使い回し
		v->major = HIWORD(ver->dwFileVersionMS);
		v->minor = LOWORD(ver->dwFileVersionMS);
		v->patch = HIWORD(ver->dwFileVersionLS);
		v->build = LOWORD(ver->dwFileVersionLS);
	}

	hfree(vi);

	return TRUE;
}

BOOL getVersion(VERSION* v)
{
	char name[MAX_PATH + 1];

	GetModuleFileName(NULL, name, MAX_PATH);

	return getFileVersion(v, name);
}
