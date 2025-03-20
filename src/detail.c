// for Win98 or later.
#define _WIN32_WINDOWS 0x0410

#include <windows.h>
#include "runtime.h"
#include "smaf.h"
#include "main.h"	// for trackcolors
#include "detail.h"

extern MAINOPTION MainOption;
extern UINT PlayingPos;

DWORD drawdetail(HWND hWnd, HDC hdc, int width, int height, SMAF* smaf, DWORD playingpos)
{
	HFONT hfold;

	SaveDC(hdc);

	SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
	SetBkMode(hdc, OPAQUE);

	{
		LOGFONT lf = {0};

		lf.lfCharSet = DEFAULT_CHARSET;
		strcopy(lf.lfFaceName, "ＭＳ ゴシック");
		lf.lfHeight = -12;

		hfold = SelectObject(hdc, CreateFontIndirect(&lf));
	}

	{
		SCROLLINFO vsi, hsi;

		vsi.cbSize = hsi.cbSize = sizeof(SCROLLINFO);
		vsi.fMask = hsi.fMask = SIF_POS;
		GetScrollInfo(hWnd, SB_VERT, &vsi);
		GetScrollInfo(hWnd, SB_HORZ, &hsi);

		{
			SIZE s;

			GetTextExtentPoint32(hdc, "A", 1, &s);
			MoveToEx(hdc, hsi.nPos * -s.cx, vsi.nPos * -s.cy, NULL);
		}
	}

	if(!smaf)
	{
		FillRectangle(hdc, 0, 0, width, height, GetSysColor(COLOR_3DFACE));
		SetBkColor(hdc, GetSysColor(COLOR_3DFACE));

		PutText(hdc, "SMAF not opened.");

		goto ret;
	}else
	{
		FillRectangle(hdc, 0, 0, width, height, GetSysColor(COLOR_WINDOW));
		SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
	}

	if(!smaf->data)
	{
		PutText(hdc, "SMAF data is empty.");
		PutText(hdc, "Please compile.");
		goto ret;
	}else
	{
		CHUNK mmmd;
		CHUNK dummy;

		PutText(hdc, "SMAF (compiled) data Info=====");

		if(!findchunk4(smaf->data, smaf->size, "MMMD", &mmmd))
		{
			PutText(hdc, "MMMD NOT FOUND");
			goto ret;
		}

		mmmd.size -= 2;	// CRC

		{
			CHUNK cnti;

			if(!findchunk4(mmmd.data, mmmd.size, "CNTI", &cnti))
			{
				PutText(hdc, "CNTI NOT FOUND");
				goto ret;
			}

			PutText(hdc, "CNTI info");

			if(cnti.data[0] == 0x00)
				PutText(hdc, "Contents class: YAMAHA");
			else
				PutText(hdc, "Contents class: Not yamaha %02X", cnti.data[0]);

			if((0x00 <= cnti.data[1] && cnti.data[1] <= 0x0F) || (0x30 <= cnti.data[1] && cnti.data[1] <= 0x33))
				PutText(hdc, "Contents type: Ringtone %02X", cnti.data[1]);
			else if((0x10 <= cnti.data[1] && cnti.data[1] <= 0x1F) || (0x40 <= cnti.data[1] && cnti.data[1] <= 0x42))
				PutText(hdc, "Contents type: Karaoke %02X", cnti.data[1]);
			else if((0x20 <= cnti.data[1] && cnti.data[1] <= 0x2F) || (0x50 <= cnti.data[1] && cnti.data[1] <= 0x53))
				PutText(hdc, "Contents type: CM %02X", cnti.data[1]);
			else
				PutText(hdc, "Contents type: RESERVED %02X", cnti.data[1]);

			switch(cnti.data[2])
			{
			case 0x00: PutText(hdc, "Contents code: Shift-JIS"); break;
			case 0x01: PutText(hdc, "Contents code: Latin-1"); break;
			case 0x02: PutText(hdc, "Contents code: EUC-KR"); break;
			case 0x03: PutText(hdc, "Contents code: GB-2312"); break;
			case 0x04: PutText(hdc, "Contents code: Big5"); break;
			case 0x05: PutText(hdc, "Contents code: KOI8-R"); break;
			case 0x06: PutText(hdc, "Contents code: TCVN-5773:1993"); break;
			case 0x20: PutText(hdc, "Contents code: UCS-2"); break;
			case 0x21: PutText(hdc, "Contents code: UCS-4"); break;
			case 0x22: PutText(hdc, "Contents code: UTF-7"); break;
			case 0x23: PutText(hdc, "Contents code: UTF-8"); break;
			case 0x24: PutText(hdc, "Contents code: UTF-16"); break;
			case 0x25: PutText(hdc, "Contents code: UTF-32"); break;
			default: PutText(hdc, "Contents code: RESERVED %02X", cnti.data[2]); break;
			}

			if(cnti.data[3] & 0x04)
				PutText(hdc, "CopyStatus: Edit NG");
			else
				PutText(hdc, "CopyStatus: Edit OK");
			if(cnti.data[3] & 0x02)
				PutText(hdc, "CopyStatus: Save NG");
			else
				PutText(hdc, "CopyStatus: Save OK");
			if(cnti.data[3] & 0x01)
				PutText(hdc, "CopyStatus: Trans NG");
			else
				PutText(hdc, "CopyStatus: Trans OK");

			PutText(hdc, "Copy Count: %d", cnti.data[4]);

			if(cnti.size > 5)
			{
				DWORD i, j;

				PutText(hdc, "Optional data followed.");

				for(i = 5; i < cnti.size; )
				{
					char tag[16];
					char data[256];
					char* p;

					tag[0] = cnti.data[i];
					tag[1] = cnti.data[i + 1];
					tag[2] = '\0';

					if(cnti.data[i] == 'V' && cnti.data[i + 1] == 'N')
						lstrcpy(tag, "Vendor");
					if(cnti.data[i] == 'C' && cnti.data[i + 1] == 'N')
						lstrcpy(tag, "Carrior");
					if(cnti.data[i] == 'C' && cnti.data[i + 1] == 'A')
						lstrcpy(tag, "Category");
					if(cnti.data[i] == 'S' && cnti.data[i + 1] == 'T')
						lstrcpy(tag, "Title");
					if(cnti.data[i] == 'A' && cnti.data[i + 1] == 'N')
						lstrcpy(tag, "Artist");
					if(cnti.data[i] == 'W' && cnti.data[i + 1] == 'W')
						lstrcpy(tag, "Word");
					if(cnti.data[i] == 'S' && cnti.data[i + 1] == 'W')
						lstrcpy(tag, "Music");
					if(cnti.data[i] == 'A' && cnti.data[i + 1] == 'W')
						lstrcpy(tag, "Edit");
					if(cnti.data[i] == 'C' && cnti.data[i + 1] == 'R')
						lstrcpy(tag, "Copyright");
					if(cnti.data[i] == 'G' && cnti.data[i + 1] == 'R')
						lstrcpy(tag, "CR-Group");
					if(cnti.data[i] == 'M' && cnti.data[i + 1] == 'I')
						lstrcpy(tag, "ManInfo");
					if(cnti.data[i] == 'C' && cnti.data[i + 1] == 'D')
						lstrcpy(tag, "Create");
					if(cnti.data[i] == 'U' && cnti.data[i + 1] == 'D')
						lstrcpy(tag, "Update");
					if(cnti.data[i] == ' ' && cnti.data[i + 1] == 'C')
						lstrcpy(tag, "Copy by?");
					i += 3;

					// 既にShift-JISとか考えてません…。
					for(j = i, p = data; /*Fact=IllegalFile->*/j < cnti.size; j++)
					{
						if(cnti.data[j] == ',')
						{
							if(cnti.data[j - 1] != '\\')
							{
								*p = '\0';
								break;
							}else
							{
								*(p - 1) = ',';
							}
						}else
						{
							*p = cnti.data[j];
							p++;
						}
					}

					//memcopy(data, cnti.data + i, j - i);
					//data[j - i] = '\0';

					i = j + 1;

					{
						TCHAR ts[256];
						wsprintf(ts, "%s: %s", tag, data);
						PutText(hdc, ts);
					}
				}
			}
		}

		{
			CHUNK opda;

			if(findchunk4(mmmd.data, mmmd.size, "OPDA", &opda))
			{
				CHUNK dch;
				PutText(hdc, "OPDA found.");

				if(findchunk3(opda.data, opda.size, "Dch", &dch))
				{
					DWORD i;
					PutText(hdc, "Dch found.");

					for(i = 0; i < dch.size; )
					{
						char tag[16];
						char data[256];

						tag[0] = dch.data[i];
						tag[1] = dch.data[i + 1];
						tag[2] = '\0';

						if(dch.data[i] == 'V' && dch.data[i + 1] == 'N')
							lstrcpy(tag, "Vendor");
						if(dch.data[i] == 'C' && dch.data[i + 1] == 'N')
							lstrcpy(tag, "Carrior");
						if(dch.data[i] == 'C' && dch.data[i + 1] == 'A')
							lstrcpy(tag, "Category");
						if(dch.data[i] == 'S' && dch.data[i + 1] == 'T')
							lstrcpy(tag, "Title");
						if(dch.data[i] == 'A' && dch.data[i + 1] == 'N')
							lstrcpy(tag, "Artist");
						if(dch.data[i] == 'W' && dch.data[i + 1] == 'W')
							lstrcpy(tag, "Word");
						if(dch.data[i] == 'S' && dch.data[i + 1] == 'W')
							lstrcpy(tag, "Music");
						if(dch.data[i] == 'A' && dch.data[i + 1] == 'W')
							lstrcpy(tag, "Edit");
						if(dch.data[i] == 'C' && dch.data[i + 1] == 'R')
							lstrcpy(tag, "Copyright");
						if(dch.data[i] == 'G' && dch.data[i + 1] == 'R')
							lstrcpy(tag, "CR-Group");
						if(dch.data[i] == 'M' && dch.data[i + 1] == 'I')
							lstrcpy(tag, "ManInfo");
						if(dch.data[i] == 'C' && dch.data[i + 1] == 'D')
							lstrcpy(tag, "Create");
						if(dch.data[i] == 'U' && dch.data[i + 1] == 'D')
							lstrcpy(tag, "Update");
						if(dch.data[i] == 'E' && dch.data[i + 1] == 'S')
							lstrcpy(tag, "EditState");
						if(dch.data[i] == 'V' && dch.data[i + 1] == 'C')
							lstrcpy(tag, "vCard");
						if(dch.data[i] == ' ' && dch.data[i + 1] == 'C')
							lstrcpy(tag, "Copy by?");
						i += 2;

						memcopy(data, dch.data + i + 2, SwapWord(dch.data + i));
						data[SwapWord(dch.data + i)] = '\0';

						i += SwapWord(dch.data + i) + 2;

						{
							TCHAR ts[256];
							wsprintf(ts, "%s: %s", tag, data);
							PutText(hdc, ts);
						}					
					
					}
				}
			}
		}

		{
			CHUNK mtr;

			if(findchunk3(mmmd.data, mmmd.size, "MTR", &mtr))
			{
				PutText(hdc, "MTR info");

				switch(mtr.data[0])
				{
				case 0x00: PutText(hdc, "Format Type: HandyPhone Standard (MA-1,2)"); break;
				case 0x01: PutText(hdc, "Format Type: Mobile Standard(Compress)"); break;
				case 0x02: PutText(hdc, "Format Type: Mobile Standard(No Compress) (MA-3,5)"); break;
				case 0x03: PutText(hdc, "Format Type: MA-7"); break;
				default: PutText(hdc, "Format Type: RESERVED %02X", mtr.data[0]); break;
				}

				switch(mtr.data[1])
				{
				case 0x00: PutText(hdc, "Sequence Type: Stream sequence"); break;
				case 0x01: PutText(hdc, "Sequence Type: Sub-sequence"); break;
				default: PutText(hdc, "Sequence Type: RESERVED %02X", mtr.data[0]); break;
				}

				switch(mtr.data[2])
				{
				case 0x00: PutText(hdc, "TimeBase(Duration): 1 msec"); break;
				case 0x01: PutText(hdc, "TimeBase(Duration): 2 msec"); break;
				case 0x02: PutText(hdc, "TimeBase(Duration): 4 msec"); break;
				case 0x03: PutText(hdc, "TimeBase(Duration): 5 msec"); break;
				case 0x10: PutText(hdc, "TimeBase(Duration): 10 msec"); break;
				case 0x11: PutText(hdc, "TimeBase(Duration): 20 msec"); break;
				case 0x12: PutText(hdc, "TimeBase(Duration): 40 msec"); break;
				case 0x13: PutText(hdc, "TimeBase(Duration): 50 msec"); break;
				default: PutText(hdc, "Sequence Type: RESERVED %02X", mtr.data[0]); break;
				}

				switch(mtr.data[3])
				{
				case 0x00: PutText(hdc, "TimeBase(GateTime): 1 msec"); break;
				case 0x01: PutText(hdc, "TimeBase(GateTime): 2 msec"); break;
				case 0x02: PutText(hdc, "TimeBase(GateTime): 4 msec"); break;
				case 0x03: PutText(hdc, "TimeBase(GateTime): 5 msec"); break;
				case 0x10: PutText(hdc, "TimeBase(GateTime): 10 msec"); break;
				case 0x11: PutText(hdc, "TimeBase(GateTime): 20 msec"); break;
				case 0x12: PutText(hdc, "TimeBase(GateTime): 40 msec"); break;
				case 0x13: PutText(hdc, "TimeBase(GateTime): 50 msec"); break;
				default: PutText(hdc, "Sequence Type: RESERVED %02X", mtr.data[0]); break;
				}

				if(mtr.data[0] == 0x00)
				{
					LPCTSTR chtypes[4] = {"NO CARE", "Melody", "No Melody", "Rhythm"};
					PutText(hdc, "Analysing MA-2 deep...");

					PutText(hdc, "Channel 1: KCS=%s VS=%s ChType=%s", (mtr.data[4] & 0x80) ? "ON" : "OFF", (mtr.data[4] & 0x40) ? "ON" : "OFF", chtypes[(mtr.data[4] >> 4) & 0x03]);
					PutText(hdc, "Channel 2: KCS=%s VS=%s ChType=%s", (mtr.data[4] & 0x08) ? "ON" : "OFF", (mtr.data[4] & 0x04) ? "ON" : "OFF", chtypes[mtr.data[4] & 0x03]);
					PutText(hdc, "Channel 3: KCS=%s VS=%s ChType=%s", (mtr.data[5] & 0x80) ? "ON" : "OFF", (mtr.data[5] & 0x40) ? "ON" : "OFF", chtypes[(mtr.data[5] >> 4) & 0x03]);
					PutText(hdc, "Channel 4: KCS=%s VS=%s ChType=%s", (mtr.data[5] & 0x08) ? "ON" : "OFF", (mtr.data[5] & 0x04) ? "ON" : "OFF", chtypes[mtr.data[5] & 0x03]);
				}

				if(mtr.data[0] == 0x02 || mtr.data[0] == 0x01)
				{
					PutText(hdc, "Analysing MA-3,5 deep...");

					{
						int i;
						for(i = 0; i < 16; i++)
						{
							TCHAR ts[1024];
							wsprintf(ts, "Channel %i: KCS=", i);
							switch(mtr.data[4 + i] >> 6)
							{
							case 0x00: lstrcat(ts, "None "); break;
							case 0x01: lstrcat(ts, "OFF "); break;
							case 0x02: lstrcat(ts, "ON "); break;
							case 0x03: lstrcat(ts, "RESERVED "); break;
							}
							if(mtr.data[4 + i] & 0x20)
								lstrcat(ts, "Vib ");
							if(mtr.data[4 + i] & 0x10)
								lstrcat(ts, "LED ");
							switch(mtr.data[4 + i] & 0x03)
							{
							case 0x00: lstrcat(ts, "Nocare "); break;
							case 0x01: lstrcat(ts, "Melody "); break;
							case 0x02: lstrcat(ts, "NON Melody "); break;
							case 0x03: lstrcat(ts, "Rhythm "); break;
							}
							PutText(hdc, ts);
						}
					}

					mtr.data += 4 + 16;

					if(findchunk4(mtr.data, mtr.size, "MspI", &dummy))
						PutText(hdc, "MspI Info found.");

					if(findchunk4(mtr.data, mtr.size, "Mtsu", &dummy))
						PutText(hdc, "Mtsu Setup found.");

					if(findchunk4(mtr.data, mtr.size, "Mtsq", &dummy))
						PutText(hdc, "Mtsq Sequence found.");

					if(findchunk4(mtr.data, mtr.size, "Mtsp", &dummy))
						PutText(hdc, "Mtsp PCM found.");
				}
			}
		}
	}

	PutText(hdc, "Internal EventList Info=====");

	{
		EVENT* e = smaf->events;
		DWORD count = 0;
		BOOL marked = FALSE;

		PutText(hdc, "SYNOPSIS: <Line No.> Duration Status(Data-Length): data... (> meaning: only CC/PC/Note/NOP/EOS.)");
		PutText(hdc, "-----");

		while(e)
		{
			TCHAR ts[1024];
			DWORD i;
			DWORD max;
			POINT pt;

			wsprintf(ts, "<%08d> %08dms %02X(%3d): ", count++, e->time, e->status, e->size);
			max = e->size;
			if(256 < max)
				max = 256;
			for(i = 0; i < max; i++)
			{
				TCHAR hex[4];
				wsprintf(hex, "%02X ", e->data[i]);
				lstrcat(ts, hex);
			}

			if((e->status & 0xF0) == 0x80)
			{
				TCHAR append[256];
				LPCTSTR note[12] = {TEXT("C-"), TEXT("C#"), TEXT("D-"), TEXT("D#"),
									TEXT("E-"), TEXT("F-"), TEXT("F#"), TEXT("G-"),
									TEXT("G#"), TEXT("A-"), TEXT("A#"), TEXT("B-"),};

				wsprintf(append, "> NoteOn %s%d", note[e->data[0] % 12], e->data[0] / 12 - 2);
				lstrcat(ts, append);
			}
			if((e->status & 0xF0) == 0x90)
			{
				TCHAR append[256];
				LPCTSTR note[12] = {TEXT("C-"), TEXT("C#"), TEXT("D-"), TEXT("D#"),
									TEXT("E-"), TEXT("F-"), TEXT("F#"), TEXT("G-"),
									TEXT("G#"), TEXT("A-"), TEXT("A#"), TEXT("B-"),};

				wsprintf(append, "> NoteOn %s%d", note[e->data[0] % 12], e->data[0] / 12 - 2);
				lstrcat(ts, append);
			}
			if((e->status & 0xF0) == 0xB0)
			{
				TCHAR append[256];
				LPCTSTR ccname[128] = {NULL};

				ccname[0]	= TEXT("BankSelect MSB");
				ccname[32]	= TEXT("BankSelect LSB");
				ccname[1]	= TEXT("Modulation");
				ccname[5]	= TEXT("Potamento time");
				ccname[7]	= TEXT("Volume");
				ccname[10]	= TEXT("Panpot");
				ccname[11]	= TEXT("Expression");
				ccname[64]	= TEXT("Hold");

				if(ccname[e->data[0]])
					wsprintf(append, "> ControlChange %s 0x%02X(%d)", ccname[e->data[0]], e->data[1], e->data[1]);
				else
					wsprintf(append, "> ControlChange %02X 0x%02X(%d)", e->data[0], e->data[1], e->data[1]);
				lstrcat(ts, append);
			}
			if(e->status == 0xFF)
			{
				if(e->data[0] == 0x00)
					lstrcat(ts, "> NOP");
				if(e->data[0] == 0x2F)
					lstrcat(ts, "> EOS");
			}

			GetCurrentPositionEx(hdc, &pt);

			{
				COLORREF oldfg = 0x80000000, oldbg = 0x80000000;

				if((e->status & 0xF0) == 0x80 || (e->status & 0xF0) == 0x90 || (e->status & 0xF0) == 0xB0 || (e->status & 0xF0) == 0xC0)
				{
					oldbg = SetBkColor(hdc, MainOption.TrackColors[e->status & 0x0F]);
					if(e->status & 0x08)
						oldfg = SetTextColor(hdc, RGB(255, 255, 255));
				}else
					if((e->status & 0xF0) == 0xF0 && e->data[0] == 0x00)
					{
						// light gray: korewa hidoi code
						DWORD fc = GetTextColor(hdc);
						DWORD bc = GetBkColor(hdc);
						struct cr{BYTE b;BYTE g;BYTE r;BYTE a;};
						struct cr f = *(struct cr*)&fc;
						struct cr b = *(struct cr*)&bc;
						struct cr x;
						x.r = (f.r + b.r * 3) / 4;
						x.g = (f.g + b.g * 3) / 4;
						x.b = (f.b + b.b * 3) / 4;

						oldfg = SetTextColor(hdc, RGB(x.r, x.g, x.b));
					}else
					{
						// dark gray: korewa hidoi code
						DWORD fc = GetTextColor(hdc);
						DWORD bc = GetBkColor(hdc);
						struct cr{BYTE b;BYTE g;BYTE r;BYTE a;};
						struct cr f = *(struct cr*)&fc;
						struct cr b = *(struct cr*)&bc;
						struct cr x;
						x.r = (f.r * 2 + b.r) / 3;
						x.g = (f.g * 2 + b.g) / 3;
						x.b = (f.b * 2 + b.b) / 3;

						oldfg = SetTextColor(hdc, RGB(x.r, x.g, x.b));
					}

				PutText(hdc, ts);

				//if((e->status & 0xF0) == 0x80 || (e->status & 0xF0) == 0x90 || (e->status & 0xF0) == 0xB0 || (e->status & 0xF0) == 0xC0
				//|| ((e->status & 0xF0) == 0xF0 && e->data[0] == 0x00))
				//{
					if(oldfg != 0x80000000)
						SetTextColor(hdc, oldfg);
					if(oldbg != 0x80000000)
						SetBkColor(hdc, oldbg);
				//}
			}

			if(!marked && playingpos <= e->time)
			{
				HPEN hpold;

				marked = TRUE;

				SaveDC(hdc);

				hpold = SelectObject(hdc, CreatePen(PS_SOLID, 1, RGB(255, 0, 0)));
				MoveToEx(hdc, 0, pt.y, NULL);
				LineTo(hdc, width, pt.y);
				DeleteObject(SelectObject(hdc, hpold));

				RestoreDC(hdc, -1);
			}

			e = e->next;
		}
	}

	PutText(hdc, "READY");
ret:
	{
		SIZE s;
		POINT p;
		SCROLLINFO vsi;

		vsi.cbSize = sizeof(SCROLLINFO);
		vsi.fMask = SIF_POS;
		GetScrollInfo(hWnd, SB_VERT, &vsi);

		GetTextExtentPoint32(hdc, "A", 1, &s);

		DeleteObject(SelectObject(hdc, hfold));

		GetCurrentPositionEx(hdc, &p);

		RestoreDC(hdc, -1);

		return p.y / s.cy + vsi.nPos;
	}
}

LRESULT CALLBACK DetailWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static SMAF* smaf = NULL;
	static UINT line;

	switch(msg)
	{
	case WM_USER:
		smaf = (SMAF*)lParam;
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_MOUSEWHEEL:
		if((short)HIWORD(wParam) > 0)
			PostMessage(hWnd, WM_VSCROLL, SB_PAGEUP, 0);
		else
			PostMessage(hWnd, WM_VSCROLL, SB_PAGEDOWN, 0);
		break;
	case WM_KEYDOWN:
		switch((int)wParam)
		{
		case VK_UP:
			PostMessage(hWnd, WM_VSCROLL, SB_LINEUP, 0);
			break;
		case VK_DOWN:
			PostMessage(hWnd, WM_VSCROLL, SB_LINEDOWN, 0);
			break;
		case VK_LEFT:
			PostMessage(hWnd, WM_HSCROLL, SB_LINELEFT, 0);
			break;
		case VK_RIGHT:
			PostMessage(hWnd, WM_HSCROLL, SB_LINERIGHT, 0);
			break;
		case VK_PRIOR:
			PostMessage(hWnd, WM_VSCROLL, SB_PAGEUP, 0);
			break;
		case VK_NEXT:
			PostMessage(hWnd, WM_VSCROLL, SB_PAGEDOWN, 0);
			break;
		}
		break;
	case WM_HSCROLL:
	case WM_VSCROLL:
		{
			SCROLLINFO scr;

			scr.cbSize = sizeof(scr);
			scr.fMask = SIF_ALL;
			if(msg == WM_HSCROLL)
				GetScrollInfo(hWnd, SB_HORZ, &scr);
			if(msg == WM_VSCROLL)
				GetScrollInfo(hWnd, SB_VERT, &scr);

			switch(LOWORD(wParam))
			{
			//case SB_TOP:
			case SB_LEFT:
				scr.nPos = scr.nMin;
				break;
			//case SB_BOTTOM:
			case SB_RIGHT:
				scr.nPos = scr.nMax;
				break;
			//case SB_LINEUP:
			case SB_LINELEFT:
				if(scr.nMin < scr.nPos)
					scr.nPos--;
				break;
			//case SB_LINEDOWN:
			case SB_LINERIGHT:
				if(scr.nPos < scr.nMax - 1)
					scr.nPos++;
				break;
			//case SB_PAGEUP:
			case SB_PAGELEFT:
				scr.nPos -= scr.nPage;
				break;
			//case SB_PAGEDOWN:
			case SB_PAGERIGHT:
				scr.nPos += scr.nPage;
				break;
			case SB_THUMBPOSITION:
			case SB_THUMBTRACK:
				scr.nPos = HIWORD(wParam);
				break;
			}

			scr.fMask = SIF_POS;
			if(msg == WM_HSCROLL)
				SetScrollInfo(hWnd, SB_HORZ, &scr, TRUE);
			if(msg == WM_VSCROLL)
				SetScrollInfo(hWnd, SB_VERT, &scr, TRUE);

			InvalidateRect(hWnd, NULL, FALSE);
		}

		return 0;
	case WM_LBUTTONDOWN:
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			RECT rc;
			DWORD lins;

			HDC hdcbg;
			HBITMAP oldbmp;
			BITMAPINFO bi = {0};

			hdc = BeginPaint(hWnd, &ps);
			GetClientRect(hWnd, &rc);

			switch(MainOption.DrawFunc)
			{
			case 0:
				break;
			case 1:
				hdcbg = hdc;
				hdc = CreateCompatibleDC(hdcbg);

				bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				bi.bmiHeader.biPlanes = 1;
				bi.bmiHeader.biWidth = rc.right;
				bi.bmiHeader.biHeight = rc.bottom;
				bi.bmiHeader.biBitCount = 24;
				oldbmp = SelectObject(hdc, CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, NULL, NULL, 0));

				break;
			case 2:
				hdcbg = hdc;
				hdc = CreateCompatibleDC(hdcbg);

				oldbmp = SelectObject(hdc, CreateCompatibleBitmap(hdcbg, rc.right, rc.bottom));

				break;
			}

			lins = drawdetail(hWnd, hdc, rc.right, rc.bottom, smaf, PlayingPos);

			switch(MainOption.DrawFunc)
			{
			case 0:
				break;
			case 1:
			case 2:
				BitBlt(hdcbg, 0, 0, rc.right, rc.bottom, hdc, 0, 0, SRCCOPY);

				DeleteObject(SelectObject(hdc, oldbmp));
				DeleteDC(hdc);

				break;
			}

			EndPaint(hWnd, &ps);

			if(lins != line)
			{
				RECT rc;

				line = lins;
				GetClientRect(hWnd, &rc);
				SendMessage(hWnd, WM_SIZE, 0, MAKELPARAM(rc.right, rc.bottom));
			}
		}
		break;
	case WM_SIZE:
		{
			SCROLLINFO si;

			si.cbSize = sizeof(si);
			si.fMask = SIF_RANGE | SIF_PAGE | SIF_DISABLENOSCROLL;

			si.nMin = 0;
			si.nMax = 65535;
			si.nPage = 10;
			SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);

			si.nMin = 0;
			si.nMax = line;
			si.nPage = 3;
			SetScrollInfo(hWnd, SB_VERT, &si, TRUE);

			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	case WM_CLOSE:
		ShowWindow(hWnd, SW_HIDE);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}
