#include <windows.h>
#include "runtime.h"
#include "smaf.h"
#include "console.h"
#include "cuimode.h"
#include "main.h"	// APPNAME

struct FSCBInfo
{
	LPCTSTR fname;
	UINT files;
	UINT current;
};

BOOL FilterSmafCB(DWORD curpos, DWORD total, UINT state, LPARAM param)
{
	if(total)
	{
		struct FSCBInfo* fi = (struct FSCBInfo*)param;
		putstrf("\r%5d/%5d %3d%%    %s", fi->current, fi->files, curpos * 100 / total, fi->fname);
	}

	return TRUE;
}

BOOL moveFileNoOverride(LPCTSTR fname, LPCTSTR destdir, LPTSTR newname)
{
	TCHAR dst[MAX_PATH];

	lstrcpy(dst, destdir);
	addEndYen(dst);
	lstrcat(dst, fname);

	if(MoveFile(fname, dst))
	{
		newname[0] = '\0';

		return TRUE;
	}else
	{
		if(GetLastError() == ERROR_ALREADY_EXISTS)
		{
			UINT no;
			TCHAR nfn[MAX_PATH];

			for(no = 1; no <= 999; no++)
			{
				memcopy(nfn, (BYTE*)fname, getLastChar(fname, '.') - fname);
				nfn[getLastChar(fname, '.') - fname] = '\0';
				{
					TCHAR kakko[16];

					wsprintf(kakko, "(%d)", no);
					lstrcat(nfn, kakko);
				}
				lstrcat(nfn, getLastChar(fname, '.'));

				lstrcpy(dst, destdir);
				addEndYen(dst);
				lstrcat(dst, nfn);

				if(MoveFile(fname, dst))
					break;
			}
			if(no <= 999)
			{
				lstrcpy(newname, nfn);
				return TRUE;
			}else
				return FALSE;
		}else
			return FALSE;
	}
}

void doFilter(void)
{
	putstr("Filtering...\n");

	{
		WIN32_FIND_DATA wfd;
		HANDLE hf;
		UINT files;
		UINT donefiles;

		if((hf = FindFirstFile("*.mmf", &wfd)) != INVALID_HANDLE_VALUE)
		{
			files = 1;
			while(FindNextFile(hf, &wfd))
				files++;
			FindClose(hf);
		}

		if((hf = FindFirstFile("*.mmf", &wfd)) != INVALID_HANDLE_VALUE)
		{
			putstr("Creating directory...");
			CreateDirectory("Filtered", NULL);
			CreateDirectory("Filtered\\MA1", NULL);
			CreateDirectory("Filtered\\MA2", NULL);
			CreateDirectory("Filtered\\MA3", NULL);
			CreateDirectory("Filtered\\MA5", NULL);
			CreateDirectory("Filtered\\MA7", NULL);
			CreateDirectory("Filtered\\Uta2", NULL);
			CreateDirectory("Filtered\\Uta3", NULL);
			CreateDirectory("Filtered\\LoadErr", NULL);
			CreateDirectory("Filtered\\Unknown", NULL);
			putstr("done.\n");

			SetSMAFSilentMode(TRUE);
			SetSMAFTypeCheckMode(TRUE);	// fast mode(a little foolish.)

			putstr("Filtering...\n");
			donefiles = 0;
			do{
				SMAF* smaf;
				struct FSCBInfo fi;

				fi.files = files;
				fi.current = donefiles + 1;
				fi.fname = wfd.cFileName;

				putstrf("%5d/%5d ...     %s", fi.current, fi.files, fi.fname);
				SetSMAFCallBack(FilterSmafCB, (LPARAM)&fi);	// TODO: KHcode, it passes LOCAL VALIABLE!!
				if(smaf = LoadSmaf(wfd.cFileName))
				{
					TCHAR movdir[32];
					TCHAR typestr[8];

					movdir[0] = '\0';

					putstrf("\r%5d/%5d MOVING  %s", fi.current, fi.files, wfd.cFileName);
					switch(smaf->format)
					{
					case SMAFF_MA1:		lstrcpy(movdir, "Filtered\\MA1");	lstrcpy(typestr, "MA-1");	break;
					case SMAFF_MA2:		lstrcpy(movdir, "Filtered\\MA2");	lstrcpy(typestr, "MA-2");	break;
					case SMAFF_MA3:		lstrcpy(movdir, "Filtered\\MA3");	lstrcpy(typestr, "MA-3");	break;
					case SMAFF_MA5:		lstrcpy(movdir, "Filtered\\MA5");	lstrcpy(typestr, "MA-5");	break;
					case SMAFF_MA7:		lstrcpy(movdir, "Filtered\\MA7");	lstrcpy(typestr, "MA-7");	break;
					case SMAFF_UTA2:	lstrcpy(movdir, "Filtered\\UTA2");	lstrcpy(typestr, "Uta2");	break;
					case SMAFF_UTA3:	lstrcpy(movdir, "Filtered\\UTA3");	lstrcpy(typestr, "Uta3");	break;
					default:
						{
							TCHAR nfn[MAX_PATH];

							if(moveFileNoOverride(wfd.cFileName, "Filtered\\Unknown", nfn))
							{
								if(!lstrlen(nfn))
									putstrf("\r%5d/%5d UNKNOWN %s\n", fi.current, fi.files, wfd.cFileName);
								else
									putstrf("\r%5d/%5d UNKNOWN %s\n            -> %s\n", fi.current, fi.files, wfd.cFileName, nfn);
							}else
							{
								putstrf("\r%5d/%5d UBERR   %s\n", fi.current, fi.files, typestr, wfd.cFileName);

								{
									LPTSTR lpBuffer;
									FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), LANG_USER_DEFAULT, (LPTSTR)&lpBuffer, 0, NULL);
									putstr(lpBuffer);
									LocalFree(lpBuffer);
								}
							}
						}
						break;
					}
					FreeSmaf(smaf);

					if(lstrlen(movdir))
					{
						TCHAR nfn[MAX_PATH];

						if(moveFileNoOverride(wfd.cFileName, movdir, nfn))
						{
							if(!lstrlen(nfn))
								putstrf("\r%5d/%5d %4s OK %s\n", fi.current, fi.files, typestr, wfd.cFileName);
							else
								putstrf("\r%5d/%5d %4s OK %s\n            -> %s\n", fi.current, fi.files, typestr, wfd.cFileName, nfn);
						}else
						{
							putstrf("\r%5d/%5d %4sBAD %s\n", fi.current, fi.files, typestr, wfd.cFileName);

							{
								LPTSTR lpBuffer;
								FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), LANG_USER_DEFAULT, (LPTSTR)&lpBuffer, 0, NULL);
								putstr(lpBuffer);
								LocalFree(lpBuffer);
							}
						}
					}
				}else
				{
					TCHAR nfn[MAX_PATH];

					if(moveFileNoOverride(wfd.cFileName, "Filtered\\LoadErr", nfn))
					{
						if(!lstrlen(nfn))
							putstrf("\r%5d/%5d LOADERR %s\n", fi.current, fi.files, wfd.cFileName);
						else
							putstrf("\r%5d/%5d LOADERR %s\n            -> %s\n", fi.current, fi.files, wfd.cFileName, nfn);
					}else
					{
						putstrf("\r%5d/%5d LMERR   %s\n", fi.current, fi.files, wfd.cFileName);

						{
							LPTSTR lpBuffer;
							FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), LANG_USER_DEFAULT, (LPTSTR)&lpBuffer, 0, NULL);
							putstr(lpBuffer);
							LocalFree(lpBuffer);
						}
					}
				}

				donefiles++;
			}while(FindNextFile(hf, &wfd));
			FindClose(hf);
			putstr("Done.\n");
		}else
			putstr("MMF not found.\n");
	}

	return;
}

void FilterMode(LPCTSTR dir)
{
	if(!AllocConsole())
		ExitProcess(0);

	SetConsoleTitle(APPNAME" FilterMode by Murachue - http://murachue.ddo.jp/");

	setcolor(BGWHITE | BGLIGHT);
	putstr("Welcome to "APPNAME" FilterMode.\n");
	setcolor(FGWHITE);

	if(lstrlen(dir))
	{
		putstrf("Directory specified: %s\n", dir);
		if(!SetCurrentDirectory(dir))
		{
			putstr("Change directory failed!!\n");
		}
		putstr("\n");
	}

	{
		TCHAR curdir[MAX_PATH];

		GetCurrentDirectory(sizeof(curdir), curdir);
		putstr("Current directory is: ");
		putstr(curdir);
		putstr("\n");
		putstr("FILTER OK(y/N)?");
	
		for(;;)
		{
			TCHAR c = geteone();

			c = LOWERCHAR(c);

			if(c == 'y')
			{
				putstr("\n");
				doFilter();
				break;
			}
			if(c == 'n')
				break;
			if(c == 0x0D)
			{
				putstr("n");
				break;
			}

			putstr("\n");
			putstr("FILTER OK(y/N)?");
		}
	}

	putstr("\n");
	putstr("Press [Enter] key to exit.\n");

	while(getone() != 0x0D)
		;

	FreeConsole();

	ExitProcess(0);
}

void doCTLS(void)
{
	putstr("Listing...\n");

	{
		WIN32_FIND_DATA wfd;
		HANDLE hf;
		UINT files;
		UINT donefiles;

		if((hf = FindFirstFile("*.mmf", &wfd)) != INVALID_HANDLE_VALUE)
		{
			files = 1;
			while(FindNextFile(hf, &wfd))
				files++;
			FindClose(hf);
		}

		if((hf = FindFirstFile("*.mmf", &wfd)) != INVALID_HANDLE_VALUE)
		{
			SetSMAFSilentMode(TRUE);
			SetSMAFTypeCheckMode(TRUE);	// fast mode(a little foolish.)

			donefiles = 0;
			do{
				SMAF* smaf;
				struct FSCBInfo fi;

				fi.files = files;
				fi.current = donefiles + 1;
				fi.fname = wfd.cFileName;

				putstrf("%5d/%5d ...     %s", fi.current, fi.files, fi.fname);
				SetSMAFCallBack(FilterSmafCB, (LPARAM)&fi);	// TODO: KHcode, it passes LOCAL VALIABLE!!
				if(smaf = LoadSmaf(wfd.cFileName))
				{
					TCHAR typestr[8];
					BYTE ct = 0xFF;

					switch(smaf->format)
					{
					case SMAFF_MA1:		lstrcpy(typestr, "MA-1");	break;
					case SMAFF_MA2:		lstrcpy(typestr, "MA-2");	break;
					case SMAFF_MA3:		lstrcpy(typestr, "MA-3");	break;
					case SMAFF_MA5:		lstrcpy(typestr, "MA-5");	break;
					case SMAFF_MA7:		lstrcpy(typestr, "MA-7");	break;
					case SMAFF_UTA2:	lstrcpy(typestr, "Uta2");	break;
					case SMAFF_UTA3:	lstrcpy(typestr, "Uta3");	break;
					default:			lstrcpy(typestr, "????");	break;
					}

					{
						CHUNK mmmd, cnti;

						if(findchunk4(smaf->data, smaf->size, "MMMD", &mmmd))
							if(findchunk4(mmmd.data, mmmd.size, "CNTI", &cnti))
								ct = cnti.data[1];
					}

					putstrf("\r%5d/%5d %4s=%02X %s\n", fi.current, fi.files, typestr, ct, wfd.cFileName);

					FreeSmaf(smaf);
				}else
					putstrf("\r%5d/%5d LOADERR %s\n", fi.current, fi.files, wfd.cFileName);

				donefiles++;
			}while(FindNextFile(hf, &wfd));
			FindClose(hf);
			putstr("Done.\n");
		}else
			putstr("MMF not found.\n");
	}

	return;
}

void ContentTypeLSMode(LPCTSTR dir)
{
	if(!AllocConsole())
		ExitProcess(0);

	SetConsoleTitle(APPNAME" ContentTypeLSMode by Murachue - http://murachue.ddo.jp/");

	setcolor(BGWHITE | BGLIGHT);
	putstr("Welcome to "APPNAME" ContentTypeLSMode.\n");
	setcolor(FGWHITE);

	if(lstrlen(dir))
	{
		putstrf("Directory specified: %s\n", dir);
		if(!SetCurrentDirectory(dir))
		{
			putstr("Change directory failed!!\n");
		}
		putstr("\n");
	}

	{
		TCHAR curdir[MAX_PATH];

		GetCurrentDirectory(sizeof(curdir), curdir);
		putstr("Current directory is: ");
		putstr(curdir);
		putstr("\n");
		putstr("LS OK(Y/n)?");

		for(;;)
		{
			TCHAR c = geteone();

			c = LOWERCHAR(c);

			if(c == 'y')
			{
				putstr("\n");
				doCTLS();
				break;
			}
			if(c == 'n')
				break;
			if(c == 0x0D)
			{
				putstr("y");
				doCTLS();
				break;
			}

			putstr("\n");
			putstr("LS OK(y/N)?");
		}
	}

	putstr("\n");
	putstr("Press [Enter] key to exit.\n");

	while(getone() != 0x0D)
		;

	FreeConsole();

	ExitProcess(0);
}
