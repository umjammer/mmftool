// PROLL.C: PianoRoll Window

// for Win98 or later.
#define _WIN32_WINDOWS 0x0410

#include <windows.h>
#include "../res/resource.h"
#include "runtime.h"
#include "smaf.h"
#include "main.h"
#include "proll.h"

#define POSBARHEIGHT 12

#define MMT_NONE	0
#define MMT_POSBAR	1
#define MMT_ONROLL	2

// 平均負荷が高くなる。確かに最後あたりを見ているときは負荷が低いんだけど…。
//#define PROLL_ENABLESKIPFIRST

typedef struct{
	UINT RollHeight;
	UINT RollWidth;
	UINT ScrollMag;
	BOOL NiagaraTouch;
	UINT DefaultGateTime;
	BOOL ShowOctave;

	int VisibleTrack;
	UINT CurrentPos;
	SMAF* smaf;
	LPCTSTR OptionString;

	int SoundingNote;	// preview
	EVENT* HilightedEvent;
	EVENT* MovingEvent;
	int MovingPosDelta;
	EVENT* SizingEvent;
	UINT PreModifyValue;	// For moving and sizing
	UINT PreModifySound;	// For moving

	UINT MouseMovingType;

#ifdef PROLL_ENABLESKIPFIRST
	UINT PrePosition;
	EVENT* PreFirstEvent;
#endif

	BOOL ReadOnly;

	UINT TempoGridShift;
	UINT TempoGridEvery;

	// for Debugging?
	BOOL ShowVisibleEventNum;
}PROLLOPTIONSTRUCT;
#define PROLLOPTION (*(PROLLOPTIONSTRUCT*)GetWindowLongPtr(hWnd, ))

extern MAINOPTION MainOption;

static void draw(HWND hWnd, HDC hdc, int width, int height)
{
	UINT scrx, scry;
	PROLLOPTIONSTRUCT* PRollOption = (PROLLOPTIONSTRUCT*)GetWindowLongPtr(hWnd, 0);
	SMAF* smaf = PRollOption->smaf;	// faster faster...
	UINT ProcessEventNum = 0;
	UINT ProcessNoteEventNum = 0;
	UINT VisibleEventNum = 0;

	HFONT hfold;

	SetTextColor(hdc, RGB(0, 0, 0));
	SetBkMode(hdc, TRANSPARENT);

	FillRectangle(hdc, 0, 0, width, height, MainOption.PRollOption.BackColor);

	scrx = GetScrollPos(hWnd, SB_HORZ) * PRollOption->ScrollMag;
	scry = GetScrollPos(hWnd, SB_VERT);

	// set global font
	{
		LOGFONT lf = {0};

		lf.lfCharSet = DEFAULT_CHARSET;
		strcopy(lf.lfFaceName, MainOption.PRollOption.SystemTextFontFace);
		lf.lfHeight = MainOption.PRollOption.SystemTextFontSize;
		lf.lfWeight = MainOption.PRollOption.SystemTextFontWeight;

		hfold = SelectObject(hdc, CreateFontIndirect(&lf));
	}

	// draw pianoroll
	{
		int i;
		// 今ではMainOption使ってるかチェックに成り下がってる
		HPEN hpold = SelectObject(hdc, CreatePen(PS_SOLID, 5, RGB(220, 220, 220)));
		COLORREF oldcolor = SetTextColor(hdc, RGB(192, 0, 192));
		EVENT* e;

		// draw vertical-duration-base grid
		if(PRollOption->MovingEvent && PRollOption->RollWidth < PRollOption->smaf->dbase)
		{
			HPEN hpold;
			int start;

			hpold = SelectObject(hdc, CreatePen(PS_SOLID, 1, MainOption.PRollOption.DurationGridColor));

			start = -(int)PRollGetScroll(hWnd) % (int)PRollOption->smaf->dbase;
			//Tracef("%d,", start);
			for(i = start; i < width * (int)PRollOption->RollWidth; i += PRollOption->smaf->dbase)
			{
				MoveToEx(hdc, i / PRollOption->RollWidth, 0 + POSBARHEIGHT, NULL);
				LineTo(hdc, i / PRollOption->RollWidth, height);
			}

			DeleteObject(SelectObject(hdc, hpold));
		}

		// draw vertical-gate-base grid
		if(PRollOption->SizingEvent && PRollOption->RollWidth < PRollOption->smaf->gbase)
		{
			HPEN hpold;
			int start;

			hpold = SelectObject(hdc, CreatePen(PS_SOLID, 1, MainOption.PRollOption.GateGridColor));

			start = -(int)PRollGetScroll(hWnd) % (int)PRollOption->smaf->gbase;
			//Tracef("%d,", start);
			for(i = start; i < width * (int)PRollOption->RollWidth; i += PRollOption->smaf->gbase)
			{
				MoveToEx(hdc, i / PRollOption->RollWidth, 0 + POSBARHEIGHT, NULL);
				LineTo(hdc, i / PRollOption->RollWidth, height);
			}

			DeleteObject(SelectObject(hdc, hpold));
		}

		// draw horizontal grid
		{
			HPEN hpold;

			hpold = SelectObject(hdc, CreatePen(PS_SOLID, 1, MainOption.PRollOption.NoteGridColor));

			for(i = -1; i < 128; i++)
			{
				MoveToEx(hdc, 0, (127 - i) * PRollOption->RollHeight - scry + POSBARHEIGHT, NULL);
				LineTo(hdc, width, (127 - i) * PRollOption->RollHeight - scry + POSBARHEIGHT);

				if(!(i % 12))
				{
					FillRectangle(hdc,	0, (127 - i) * PRollOption->RollHeight - scry + 1 + POSBARHEIGHT,
										width, (127 - i + 1) * PRollOption->RollHeight - scry + POSBARHEIGHT, MainOption.PRollOption.NoteHilightGridColor);
				}
			}

			DeleteObject(SelectObject(hdc, hpold));
		}

		// draw vertical grid
		{
			HPEN hpold = SelectObject(hdc, CreatePen(PS_SOLID, 1, MainOption.PRollOption.TimeGridColor));

			for(i = -((int)scrx * (int)PRollOption->RollWidth % 100); i < width * (int)PRollOption->RollWidth; i += 100)
			{
				HPEN hpold;

				if(!((i + scrx * PRollOption->RollWidth) % (50 * PRollOption->RollWidth)))	// darker
					hpold = SelectObject(hdc, CreatePen(PS_SOLID, 1, MainOption.PRollOption.TimeGridHilightColor));

				MoveToEx(hdc, i / PRollOption->RollWidth, 0 + POSBARHEIGHT, NULL);
				LineTo(hdc, i / PRollOption->RollWidth, height);

				if(!((i + scrx * PRollOption->RollWidth) % (50 * PRollOption->RollWidth)))
					DeleteObject(SelectObject(hdc, hpold));
			}

			DeleteObject(SelectObject(hdc, hpold));
		}

		if(PRollOption->TempoGridEvery)
		{
			HPEN hpold;
			int start;

			hpold = SelectObject(hdc, CreatePen(PS_SOLID, 1, MainOption.PRollOption.TempoGridColor));

			start = ((int)PRollOption->TempoGridShift - (int)PRollGetScroll(hWnd)) % (int)PRollOption->TempoGridEvery;
			//Tracef("%d,", start);
			for(i = start; i < width * (int)PRollOption->RollWidth; i += PRollOption->TempoGridEvery)
			{
				MoveToEx(hdc, i / PRollOption->RollWidth, 0 + POSBARHEIGHT, NULL);
				LineTo(hdc, i / PRollOption->RollWidth, height);
			}

			DeleteObject(SelectObject(hdc, hpold));
		}

		if(smaf)
		{
#ifdef PROLL_ENABLESKIPFIRST
			if(scrx * PRollOption->RollWidth < PRollOption->PrePosition)
				PRollOption->PreFirstEvent = NULL;
#endif

			e = smaf->events;

#ifdef PROLL_ENABLESKIPFIRST
			if(PRollOption->PreFirstEvent)
				e = PRollOption->PreFirstEvent;

			PRollOption->PreFirstEvent = NULL;
#endif

			// Note draw loop
			for(; e; e = e->next)
			{
				DWORD gtime;

				ProcessEventNum++;

				if((e->status & 0xF0) != 0x80 && (e->status & 0xF0) != 0x90)	// Not NoteON
					continue;

				ProcessNoteEventNum++;

				if((UINT)((scrx + width) * PRollOption->RollWidth) < e->time)	// Over. no draw needed any more.
					break;

				if(PRollOption->VisibleTrack != -1 && (e->status & 0x0F) != PRollOption->VisibleTrack)	// NoVisible
					continue;

				gtime = getGateTime(e);

				if(e->time + gtime < scrx * PRollOption->RollWidth)
					continue;

#ifdef PROLL_ENABLESKIPFIRST
				if(!PRollOption->PreFirstEvent)
				{
					PRollOption->PreFirstEvent = e;
					PRollOption->PrePosition = scrx * PRollOption->RollWidth;
				}
#endif

				VisibleEventNum++;

#if defined(_DEBUG) && 0
				// draw duration line
				Line(hdc,	(e->time - e->duration) / PRollOption->RollWidth - scrx,
							((127 - e->data[0]) * 2 + 1) * PRollOption->RollHeight / 2 - scry + POSBARHEIGHT,
							e->time / PRollOption->RollWidth - scrx,
							((127 - e->data[0]) * 2 + 1) * PRollOption->RollHeight / 2 - scry + POSBARHEIGHT,
							MainOption.PRollOption.NoteEdgeColor);
#endif

				if(PRollOption->HilightedEvent == e)
				{
					// Draw vertical border (moving|sizing)
					UINT x = (PRollOption->HilightedEvent->time + (PRollOption->HilightedEvent->time + getGateTime(PRollOption->HilightedEvent)) * 4) / 5;

					Line(hdc,
						x / PRollOption->RollWidth - scrx, (127 - e->data[0]) * PRollOption->RollHeight - scry + POSBARHEIGHT - 2,
						x / PRollOption->RollWidth - scrx, ((127 - e->data[0]) + 1) * PRollOption->RollHeight - scry + POSBARHEIGHT + 2,
						MainOption.PRollOption.NoteHilightEdgeColor);
				}

				FillRectangle(hdc,	e->time / PRollOption->RollWidth - scrx,
									(127 - e->data[0]) * PRollOption->RollHeight - scry + POSBARHEIGHT,
									(e->time + gtime) / PRollOption->RollWidth - scrx,
									((127 - e->data[0]) + 1) * PRollOption->RollHeight - scry + POSBARHEIGHT,
									(PRollOption->HilightedEvent == e) ? MainOption.PRollOption.NoteHilightEdgeColor : MainOption.PRollOption.NoteEdgeColor);

				FillRectangle(hdc,	e->time / PRollOption->RollWidth - scrx + 1,
									(127 - e->data[0]) * PRollOption->RollHeight - scry + 1 + POSBARHEIGHT,
									(e->time + gtime) / PRollOption->RollWidth - scrx - 1,
									((127 - e->data[0]) + 1) * PRollOption->RollHeight - scry - 1 + POSBARHEIGHT,
									MainOption.TrackColors[e->status & 0x0F]);
			}

			// Draw info
			if(PRollOption->HilightedEvent && ((PRollOption->HilightedEvent->status & 0xF0) == 0x80 || (PRollOption->HilightedEvent->status & 0xF0) == 0x90))
			{
				TCHAR ts[80];
				SIZE s;
				LPCTSTR notename[12] = {"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"};
				EVENT* e = PRollOption->HilightedEvent;
				DWORD gtime = getGateTime(e);
				INT x;

				SaveDC(hdc);

				SetBkMode(hdc, OPAQUE);
				SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
				SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
				wsprintf(ts, "Track:%u Time:%u Gate:%u %s%d", (e->status & 0x0F) + 1, e->time, gtime, notename[e->data[0] % 12], e->data[0] / 12 - 2);

				GetTextExtentPoint32(hdc, ts, lstrlen(ts), &s);
				x = (e->time/* + gtime*/) / PRollOption->RollWidth - scrx;
				if(x < 0)
					x = 0;
				TextOut(hdc, x, (127 - e->data[0]) * PRollOption->RollHeight - scry + POSBARHEIGHT - s.cy, ts, lstrlen(ts));

				RestoreDC(hdc, -1);
			}

			if(PRollOption->MovingEvent)
			{
				EVENT* e = PRollOption->MovingEvent;
				DWORD gtime = getGateTime(e);

				Line(hdc,
					e->time / PRollOption->RollWidth - scrx, 0,
					e->time / PRollOption->RollWidth - scrx, height,
					MainOption.PRollOption.DurationLineColor);
				Line(hdc,
					(e->time + gtime) / PRollOption->RollWidth - scrx, 0,
					(e->time + gtime) / PRollOption->RollWidth - scrx, height,
					MainOption.PRollOption.DurationLineColor);
			}
			if(PRollOption->SizingEvent)
			{
				EVENT* e = PRollOption->SizingEvent;
				DWORD gtime = getGateTime(e);

				Line(hdc,
					e->time / PRollOption->RollWidth - scrx, 0,
					e->time / PRollOption->RollWidth - scrx, height,
					MainOption.PRollOption.GateLineColor);
				Line(hdc,
					(e->time + gtime) / PRollOption->RollWidth - scrx, 0,
					(e->time + gtime) / PRollOption->RollWidth - scrx, height,
					MainOption.PRollOption.GateLineColor);
			}
		}

		// draw horizontal grid text(ScoreLine)
		{
			COLORREF otc = SetTextColor(hdc, MainOption.PRollOption.NoteGridTextColor);
			LOGFONT lf = {0};
			HFONT hfold;

			lf.lfCharSet = DEFAULT_CHARSET;
			strcopy(lf.lfFaceName, MainOption.PRollOption.NoteGridTextFontFace);
			lf.lfHeight = MainOption.PRollOption.NoteGridTextFontSize;
			lf.lfWeight = MainOption.PRollOption.NoteGridTextFontWeight;

			hfold = SelectObject(hdc, CreateFontIndirect(&lf));

			if(PRollOption->ShowOctave)
			{
				// OnlyOctave.
				for(i = 0; i < 128; i += 12)
				{
					MoveToEx(hdc, 1, (127 - i) * PRollOption->RollHeight - scry + 1 + POSBARHEIGHT - 2 + MainOption.PRollOption.NoteGridTextYOffset, NULL);
					PutText(hdc, "Octave %d(%d)", i / 12 - 2, i);
				}
			}else
			{
				for(i = 0; i < 128; i++)
				{
					LPCTSTR oto[] = {TEXT("C"), TEXT("C#"), TEXT("D"), TEXT("D#"), TEXT("E"), TEXT("F"), TEXT("F#"), TEXT("G"), TEXT("G#"), TEXT("A"), TEXT("A#"), TEXT("B")};
					MoveToEx(hdc, 1, (127 - i) * PRollOption->RollHeight - scry + 1 + POSBARHEIGHT - 2 + MainOption.PRollOption.NoteGridTextYOffset, NULL);
					PutText(hdc, "%-2s%d=%d", oto[i % 12], i / 12 - 2, i);
				}
			}

			SetTextColor(hdc, otc);
			DeleteObject(SelectObject(hdc, hfold));
		}

		SetTextColor(hdc, oldcolor);
		DeleteObject(SelectObject(hdc, hpold));
	}

	// draw PosBar
	{
		COLORREF oldcolor = SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));
		int i;
		UINT rw = PRollOption->RollWidth;
		HFONT hfold;
		LOGFONT lf = {0};

		lf.lfCharSet = DEFAULT_CHARSET;
		strcopy(lf.lfFaceName, MainOption.PRollOption.PosbarTextFontFace);
		lf.lfHeight = MainOption.PRollOption.PosbarTextFontSize;
		lf.lfWeight = MainOption.PRollOption.PosbarTextFontWeight;

		hfold = SelectObject(hdc, CreateFontIndirect(&lf));

		FillRectangle(hdc, 0, 0, width, POSBARHEIGHT, GetSysColor(COLOR_BTNFACE));

		// draw vertical grid text(TimeLine)
		for(i = -((int)scrx * (int)rw % (/*なんでここ50じゃなくて100じゃないとうまく動かないんだ?->*/100 * (int)rw)); i < width * (int)rw; i += 100)
		{
			if((i + scrx * rw) % (50 * rw))
				continue;
			MoveToEx(hdc, i / (int)PRollOption->RollWidth + 2, 0, NULL);
			PutText(hdc, "%ums", i + scrx * rw);
		}

		SetTextColor(hdc, oldcolor);
		DeleteObject(SelectObject(hdc, hfold));
	}

	// Start
	if(smaf && smaf->start)
	{
		HPEN hpold = SelectObject(hdc, CreatePen(PS_SOLID, 1, MainOption.PRollOption.StartPosColor));

		MoveToEx(hdc, (smaf->start->time - smaf->start->duration) / PRollOption->RollWidth - scrx, 0, NULL);
		LineTo(hdc, (smaf->start->time - smaf->start->duration) / PRollOption->RollWidth - scrx, height);

		DeleteObject(SelectObject(hdc, hpold));
	}
	// End
	if(smaf && smaf->stop)
	{
		HPEN hpold = SelectObject(hdc, CreatePen(PS_SOLID, 1, MainOption.PRollOption.EndPosColor));

		MoveToEx(hdc, smaf->stop->time / PRollOption->RollWidth - scrx, 0, NULL);
		LineTo(hdc, smaf->stop->time / PRollOption->RollWidth - scrx, height);

		DeleteObject(SelectObject(hdc, hpold));
	}

	// Current
	{
		HPEN hpold = SelectObject(hdc, CreatePen(PS_SOLID, 1, MainOption.PRollOption.CurrentPosColor));

		MoveToEx(hdc, PRollOption->CurrentPos / PRollOption->RollWidth - scrx, 0, NULL);
		LineTo(hdc, PRollOption->CurrentPos / PRollOption->RollWidth - scrx, height);

		DeleteObject(SelectObject(hdc, hpold));
	}

	if(PRollOption->VisibleTrack != -1)
	{
		SIZE s;
		TCHAR ts[32];

		SaveDC(hdc);

		SetBkMode(hdc, OPAQUE);
		SetBkColor(hdc, RGB(255, 255, 255));
		SetTextColor(hdc, RGB(255, 0, 0));

		wsprintf(ts, "Viewing Track %d", PRollOption->VisibleTrack + 1);

		GetTextExtentPoint32(hdc, ts, lstrlen(ts), &s);
		TextOut(hdc, width - s.cx, height - s.cy, ts, lstrlen(ts));

		RestoreDC(hdc, -1);
	}
	if(PRollOption->ShowVisibleEventNum)
	{
		SIZE s;
		TCHAR ts[32];

		SaveDC(hdc);

		SetBkMode(hdc, OPAQUE);
		SetBkColor(hdc, RGB(255, 255, 255));
		SetTextColor(hdc, RGB(0, 0, 0));

		wsprintf(ts, "ProcessedEvents: %u Event(s)", ProcessEventNum);
		GetTextExtentPoint32(hdc, ts, lstrlen(ts), &s);
		TextOut(hdc, 0, height - s.cy * 3, ts, lstrlen(ts));

		wsprintf(ts, "ProcessedNoteEvents: %u Event(s)", ProcessNoteEventNum);
		TextOut(hdc, 0, height - s.cy * 2, ts, lstrlen(ts));

		wsprintf(ts, "VisibleEvents: %u Event(s)", VisibleEventNum);
		TextOut(hdc, 0, height - s.cy, ts, lstrlen(ts));

		RestoreDC(hdc, -1);
	}

	if(PRollOption->OptionString)
	{
		SaveDC(hdc);

		SetBkMode(hdc, OPAQUE);
		SetBkColor(hdc, RGB(255, 255, 255));
		SetTextColor(hdc, RGB(0, 0, 0));

		TextOut(hdc, 0, 0, PRollOption->OptionString, lstrlen(PRollOption->OptionString));

		RestoreDC(hdc, -1);
	}

	DeleteObject(SelectObject(hdc, hfold));

	return;
}

static void Resize(HWND hWnd)
{
	PROLLOPTIONSTRUCT* PRollOption = (PROLLOPTIONSTRUCT*)GetWindowLongPtr(hWnd, 0);
	// If you want window-client size, use GetClientRect instead of lParam.

	SCROLLINFO si;

	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_DISABLENOSCROLL;
	si.nMin = 0;
	si.nMax = 65535;
	si.nPage = 10 / PRollOption->ScrollMag;
	if(si.nPage == 0)
		si.nPage = 1;
	SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);

	si.nMin = 0;
	si.nMax = 128 * PRollOption->RollHeight;
	si.nPage = PRollOption->RollHeight;
	SetScrollInfo(hWnd, SB_VERT, &si, TRUE);

	InvalidateRect(hWnd, NULL, FALSE);

	return;
}

EVENT* EventHittest(HWND hWnd, UINT time, int note)
{
	EVENT* e;
	EVENT* lastselect = NULL;
	PROLLOPTIONSTRUCT* PRollOption = (PROLLOPTIONSTRUCT*)GetWindowLongPtr(hWnd, 0);

	for(e = PRollOption->smaf->events; e; e = e->next)
	{
		BYTE st = e->status & 0xF0;

		if(time < e->time)
			break;

		if(st == 0x80 || st == 0x90)
		{
			if(PRollOption->VisibleTrack == -1 || (e->status & 0x0F) == PRollOption->VisibleTrack)
			{
				DWORD gt = getGateTime(e);
				if(e->data[0] == note && e->time <= time && time < (e->time + gt))
					lastselect = e;
			}
		}
	}

	return lastselect;
}

void SetEOS(SMAF* smaf)
{
	EVENT* ev;
	EVENT* eos;

	for(eos = smaf->events; eos; eos = eos->next)
	{
		// EOS
		if(eos->status == 0xFF && eos->data[0] == 0x2F && eos->data[1] == 0x00)
			break;
	}
	if(!eos)
	{
		eos = halloc(sizeof(EVENT));
		eos->status = 0xFF;
		eos->size = 2;
		eos->data = halloc(eos->size);
		eos->data[0] = 0x2F;
		eos->data[1] = 0x00;
	}else
		CutoutEvent(smaf, eos);

	eos->time = 0;

	for(ev = smaf->events; ev; ev = ev->next)
	{
		if(eos->time < ev->time)
			eos->time = ev->time;
		if(((ev->status & 0xF0) == 0x80) || ((ev->status & 0xF0) == 0x90))
		{
			UINT gtime = GetVVal(ev->data + (((ev->status & 0xF0) == 0x80) ? 1 : 2));
			if(eos->time < ev->time + gtime)
				eos->time = ev->time + gtime;
		}
	}

	/*
	if(!ev)
	{
		eos->time = 0;
	}else
	{
		eos->time = ev->time;
		if(((ev->status & 0xF0) == 0x80) || ((ev->status & 0xF0) == 0x90))
			eos->time += GetVVal(ev->data + (((ev->status & 0xF0) == 0x80) ? 1 : 2));
	}
	*/

	smaf->start = InsertEvent(smaf, eos);

	return;
}

LRESULT CALLBACK PRollWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PROLLOPTIONSTRUCT* PRollOption = (PROLLOPTIONSTRUCT*)GetWindowLongPtr(hWnd, 0);

	switch(msg)
	{
	case WM_NCCREATE:
		{
			DWORD wex;

			wex = GetWindowLong(hWnd, GWL_EXSTYLE);
			wex |= WS_EX_CLIENTEDGE;
			SetWindowLong(hWnd, GWL_EXSTYLE, wex);
		}
		break;
	case WM_CREATE:
		SetWindowLongPtr(hWnd, 0, (LONG_PTR)halloc(sizeof(PROLLOPTIONSTRUCT)));

		//SendMessage(hWnd, WM_PROLL, PROLLWM_RELOADCONFIG, 0);
		{
			PROLLOPTIONSTRUCT* PRollOption = (PROLLOPTIONSTRUCT*)GetWindowLongPtr(hWnd, 0);

			// Settings
			PRollOption->RollWidth = MainOption.PRollOption.RollWidth;
			PRollOption->RollHeight = MainOption.PRollOption.RollHeight;
			PRollOption->NiagaraTouch = MainOption.PRollOption.NiagaraTouch;
			PRollOption->DefaultGateTime = MainOption.PRollOption.DefaultGateTime;
			PRollOption->ScrollMag = MainOption.PRollOption.ScrollMag;
			PRollOption->ShowOctave = MainOption.PRollOption.ShowOctave;
			PRollOption->ReadOnly = MainOption.PRollOption.ReadOnly;

			// CurrentStatuses
			PRollOption->VisibleTrack = -1;
			PRollOption->CurrentPos = 0;
			PRollOption->smaf = NULL;
			PRollOption->OptionString = NULL;
			PRollOption->SoundingNote = -1;
			PRollOption->HilightedEvent = NULL;
			PRollOption->MovingEvent = NULL;
			PRollOption->SizingEvent = NULL;
			PRollOption->MouseMovingType = MMT_NONE;
#ifdef PROLL_ENABLESKIPFIRST
			PRollOption->PrePosition = 0;
			PRollOption->PreFirstEvent = NULL;
#endif
			PRollOption->TempoGridShift = 0;
			PRollOption->TempoGridEvery = 0;

			// DebugStatus
			PRollOption->ShowVisibleEventNum = FALSE;
		}

		break;
	case WM_DESTROY:
		hfree((void*)GetWindowLongPtr(hWnd, 0));
		break;
	case WM_MOUSEWHEEL:
		if(GetAsyncKeyState(VK_CONTROL) & 0x8000)
		{
			if((short)HIWORD(wParam) > 0)
				PostMessage(hWnd, WM_HSCROLL, SB_PAGELEFT, 0);
			else
				PostMessage(hWnd, WM_HSCROLL, SB_PAGERIGHT, 0);
		}else
		{
			if((short)HIWORD(wParam) > 0)
				PostMessage(hWnd, WM_VSCROLL, SB_PAGEUP, 0);
			else
				PostMessage(hWnd, WM_VSCROLL, SB_PAGEDOWN, 0);
		}
		return 0;
		// DefWindowProcするとMsg転送されたとき転送元と転送先で無限ループ
		//break;
	case WM_MOUSEACTIVATE:
		if(GetFocus() != hWnd)
		{
			SetFocus(hWnd);

			{
				UINT x;
				int y;
				POINT p;

				GetCursorPos(&p);
				ScreenToClient(hWnd, &p);

				x = PRollGetScroll(hWnd) + p.x * PRollOption->RollWidth;
				y = 127 - ((p.y + GetScrollPos(hWnd, SB_VERT) - POSBARHEIGHT) / PRollOption->RollHeight);

				if((INT)LOWORD(lParam) == HTCLIENT && PRollOption->smaf && 0 <= y && y <= 127)
					if(!EventHittest(hWnd, x, y))
						return MA_ACTIVATEANDEAT;
			}
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
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			RECT rc;

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

			draw(hWnd, hdc, rc.right, rc.bottom);

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
		}
		break;
	case WM_SIZE:
		Resize(hWnd);
		break;
	case WM_PROLL:
		switch(LOWORD(wParam))
		{
			/*
			// TODO: Obsolete
		//=== SYSTEM ===//
		case PROLLWM_RELOADCONFIG:
			break;
			*/

		//=== OPTION/COMMAND ===//
		case PROLLWM_SCROLLTO:
			{
				int newpos = ((int)lParam / (int)PRollOption->RollWidth + (short)HIWORD(wParam)) / (int)PRollOption->ScrollMag;
				if(newpos < 0)
					newpos = 0;
				SetScrollPos(hWnd, SB_HORZ, newpos, TRUE);
				InvalidateRect(hWnd, NULL, FALSE);
			}
			break;
		case PROLLWM_SETROLLWIDTH:
			{
				UINT time = PRollGetScroll(hWnd);
				PRollOption->RollWidth = lParam;
				SendMessage(hWnd, WM_PROLL, PROLLWM_SCROLLTO, time);
			}
			break;
		case PROLLWM_SETROLLHEIGHT:
			PRollOption->RollHeight = lParam;
			Resize(hWnd);
			break;
		case PROLLWM_SETVISIBLETRACK:
			PRollOption->VisibleTrack = lParam;
			SendMessage(GetParent(hWnd), WM_PROLLNOTIFY, PN_VTCHANGED, lParam);	// TODO:KHcode.....delete this....if you can...
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case PROLLWM_GETROLLWIDTH:
			return PRollOption->RollWidth;
		case PROLLWM_GETROLLHEIGHT:
			return PRollOption->RollHeight;
		case PROLLWM_GETVISIBLETRACK:
			return PRollOption->VisibleTrack;
		case PROLLWM_GETSCROLL:
			return GetScrollPos(hWnd, SB_HORZ) * PRollOption->ScrollMag * PRollOption->RollWidth;
		case PROLLWM_SETSMAF:
			PRollOption->HilightedEvent = NULL;
			PRollOption->smaf = (SMAF*)lParam;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case PROLLWM_SETCURRENTPOS:
			PRollOption->CurrentPos = lParam;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case PROLLWM_SETOPTIONSTRING:
			PRollOption->OptionString = (LPCTSTR)lParam;
			break;
		case PROLLWM_GETOPTIONSTRING:
			return (LRESULT)PRollOption->OptionString;
		case PROLLWM_SETREADONLY:
			PRollOption->ReadOnly = (BOOL)lParam;
			break;
		case PROLLWM_SETHILIGHTEVENT:
			PRollOption->HilightedEvent = (EVENT*)lParam;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case PROLLWM_SETNIAGARATOUCH:
			PRollOption->NiagaraTouch = (BOOL)lParam;
			break;
		case PROLLWM_GETNIAGARATOUCH:
			return PRollOption->NiagaraTouch;
		case PROLLWM_GETREADONLY:
			return PRollOption->ReadOnly;
		case PROLLWM_SETSHOWVISIBLEEVENTNUM:
			PRollOption->ShowVisibleEventNum = (BOOL)lParam;
			break;
		case PROLLWM_GETSHOWVISIBLEEVENTNUM:
			return PRollOption->ShowVisibleEventNum;
		case PROLLWM_GETHILIGHTEVENT:
			return (LRESULT)PRollOption->HilightedEvent;
		case PROLLWM_SETSCROLLMAG:
			{
				UINT scrl = PRollGetScroll(hWnd);
				PRollOption->ScrollMag = lParam;
				Resize(hWnd);
				SendMessage(hWnd, WM_PROLL, PROLLWM_SCROLLTO, scrl);
			}
			break;
		case PROLLWM_GETSCROLLMAG:
			return PRollOption->ScrollMag;
		case PROLLWM_SETTEMPOGRIDSHIFT:
			PRollOption->TempoGridShift = lParam;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case PROLLWM_SETTEMPOGRIDEVERY:
			PRollOption->TempoGridEvery = lParam;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		}
		break;
	case WM_LBUTTONDOWN:
		SetCapture(hWnd);
		if(HIWORD(lParam) < POSBARHEIGHT)
		{
			if(PRollOption->smaf)
			{
				PRollOption->CurrentPos = PRollGetScroll(hWnd) + LOWORD(lParam) * PRollOption->RollWidth;
				SendMessage(GetParent(hWnd), WM_PROLLNOTIFY, PN_CURRENTPOSCHANGED, PRollOption->CurrentPos);
				InvalidateRect(hWnd, NULL, FALSE);
				PRollOption->MouseMovingType = MMT_POSBAR;
			}
		}
		if(POSBARHEIGHT <= HIWORD(lParam))
		{
			UINT x = PRollGetScroll(hWnd) + LOWORD(lParam) * PRollOption->RollWidth;
			int y = 127 - ((HIWORD(lParam) + GetScrollPos(hWnd, SB_VERT) - POSBARHEIGHT) / PRollOption->RollHeight);

			if(0 <= y && y <= 127)
			{
				if(PRollOption->smaf)
				{
					EVENT* oldhl = PRollOption->HilightedEvent;
					PRollOption->HilightedEvent = EventHittest(hWnd, x, y);
					if(!PRollOption->ReadOnly && PRollOption->HilightedEvent)
					{
						if(
							(PRollOption->HilightedEvent->time + (PRollOption->HilightedEvent->time + getGateTime(PRollOption->HilightedEvent)) * 4) / 5 < x
							|| (PRollOption->HilightedEvent->time + getGateTime(PRollOption->HilightedEvent) - 2) < x
							)
						{
							POINT p;

							PRollOption->SizingEvent = PRollOption->HilightedEvent;

							p.x = (PRollOption->SizingEvent->time + getGateTime(PRollOption->SizingEvent) - PRollGetScroll(hWnd)) / PRollOption->RollWidth;
							p.y = HIWORD(lParam);
							ClientToScreen(hWnd, &p);
							SetCursorPos(p.x, p.y);

							PRollOption->PreModifyValue = getGateTime(PRollOption->SizingEvent);
						}else
						{
							PRollOption->MovingEvent = PRollOption->HilightedEvent;
							PRollOption->MovingPosDelta = PRollOption->MovingEvent->time - x;

							PRollOption->PreModifyValue = PRollOption->MovingEvent->time;
							PRollOption->PreModifySound = PRollOption->MovingEvent->data[0];
						}
					}

					if(oldhl != PRollOption->HilightedEvent)
						SendMessage(GetParent(hWnd), WM_PROLLNOTIFY, PN_EVENTHILIGHTED, (LPARAM)PRollOption->HilightedEvent);

					InvalidateRect(hWnd, NULL, FALSE);
				}

				PRollOption->SoundingNote = y;
				SendMessage(GetParent(hWnd), WM_PROLLNOTIFY, MAKEWPARAM(PN_CLICKED, y), x);

				PRollOption->MouseMovingType = MMT_ONROLL;
			}
		}
		break;
	case WM_LBUTTONDBLCLK:
		if(HIWORD(lParam) >= POSBARHEIGHT)
		{
			UINT x = PRollGetScroll(hWnd) + LOWORD(lParam) * PRollOption->RollWidth;
			int y = 127 - ((HIWORD(lParam) + GetScrollPos(hWnd, SB_VERT) - POSBARHEIGHT) / PRollOption->RollHeight);

			if(PRollOption->smaf && 0 <= y && y <= 127)
			{
				if(EventHittest(hWnd, x, y))
				{
					// setting, menu, etc.
					;
				}else
				{
					if(!PRollOption->ReadOnly)
					{
						// blank space->create note
						EVENT* e;
						DWORD gt = PRollOption->DefaultGateTime / PRollOption->smaf->gbase * PRollOption->smaf->gbase;
						UINT tr = (PRollOption->VisibleTrack == -1) ? 0 : PRollOption->VisibleTrack;

						e = halloc(sizeof(EVENT));
						e->time = x / PRollOption->smaf->dbase * PRollOption->smaf->dbase;	// aligned placing
						e->status = 0x90 | tr;
						e->size = 2 + SetVValSize(gt);
						e->data = halloc(e->size);
						e->data[0] = y;
						e->data[1] = 0x7F;	// velo
						SetVVal(e->data + 2, gt);

						PRollOption->smaf->start = InsertEvent(PRollOption->smaf, e);
						SetEOS(PRollOption->smaf);

						SendMessage(GetParent(hWnd), WM_PROLLNOTIFY, PN_EVENTCREATED, (LPARAM)e);

						InvalidateRect(hWnd, NULL, FALSE);
					}
				}
			}

			if(0 <= y && y <= 127)
				SendMessage(GetParent(hWnd), WM_PROLLNOTIFY, MAKEWPARAM(PN_DBLCLICKED, y), /*PRollGetScroll(hWnd) + LOWORD(lParam) * PRollOption->RollWidth*/x);
		}
		break;
	case WM_MOUSEMOVE:
		if(GetCapture() == hWnd)	// pressing...tenuki
		{
			if(PRollOption->MouseMovingType == MMT_POSBAR)
			{
				int x = (int)PRollGetScroll(hWnd) + (int)(short)LOWORD(lParam) * (int)PRollOption->RollWidth;

				if(x < 0)
					x = 0;

				PRollOption->CurrentPos = x;
				SendMessage(GetParent(hWnd), WM_PROLLNOTIFY, PN_CURRENTPOSCHANGED, PRollOption->CurrentPos);
				InvalidateRect(hWnd, NULL, FALSE);
			}
			if(PRollOption->MouseMovingType == MMT_ONROLL && POSBARHEIGHT <= HIWORD(lParam))
			{
				int x = (int)PRollGetScroll(hWnd) + (int)(short)LOWORD(lParam) * (int)PRollOption->RollWidth;
				int y = 127 - ((HIWORD(lParam) + GetScrollPos(hWnd, SB_VERT) - POSBARHEIGHT) / PRollOption->RollHeight);

				if(x < 0)
					x = 0;

				if(PRollOption->MovingEvent)
				{
					// align: for "duration-base dividing in convert" problem
					int time = (x + PRollOption->MovingPosDelta) / PRollOption->smaf->dbase * PRollOption->smaf->dbase;

					if(time < 0)
						time = 0;

					PRollOption->MovingEvent->time = time;
					// refresh duration
					CutoutEvent(PRollOption->smaf, PRollOption->MovingEvent);
					PRollOption->smaf->start = InsertEvent(PRollOption->smaf, PRollOption->MovingEvent);

					if(0 <= y && y <= 127)
						PRollOption->MovingEvent->data[0] = y;
					SetEOS(PRollOption->smaf);

					SendMessage(GetParent(hWnd), WM_PROLLNOTIFY, MAKEWPARAM(PN_EVENTMODIFYING, PNEM_MOVE), (LPARAM)PRollOption->MovingEvent);

					InvalidateRect(hWnd, NULL, FALSE);
				}
				if(PRollOption->SizingEvent)
				{
					EVENT* e = PRollOption->SizingEvent;

					if((e->status & 0xF0) == 0x80)
					{
						BYTE note = e->data[0];
						int newgt = x - e->time;

						if(newgt < (int)PRollOption->smaf->gbase)
							newgt = PRollOption->smaf->gbase;

						// align
						newgt = newgt / PRollOption->smaf->gbase * PRollOption->smaf->gbase;

						hfree(e->data);
						e->size = 1 + SetVValSize(newgt);
						e->data = halloc(e->size);
						e->data[0] = note;
						SetVVal(e->data + 1, newgt);
					}
					if((e->status & 0xF0) == 0x90)
					{
						BYTE note = e->data[0];
						BYTE vel = e->data[1];
						int newgt = x - e->time;

						if(newgt < (int)PRollOption->smaf->gbase)
							newgt = PRollOption->smaf->gbase;

						// align
						newgt = newgt / PRollOption->smaf->gbase * PRollOption->smaf->gbase;

						hfree(e->data);
						e->size = 2 + SetVValSize(newgt);
						e->data = halloc(e->size);
						e->data[0] = note;
						e->data[1] = vel;
						SetVVal(e->data + 2, newgt);
					}

					SetEOS(PRollOption->smaf);

					SendMessage(GetParent(hWnd), WM_PROLLNOTIFY, MAKEWPARAM(PN_EVENTMODIFYING, PNEM_SIZE), (LPARAM)PRollOption->SizingEvent);

					InvalidateRect(hWnd, NULL, FALSE);
				}
				if(PRollOption->NiagaraTouch && 0 <= y && y <= 127 && PRollOption->SoundingNote != y)
				{
					SendMessage(GetParent(hWnd), WM_PROLLNOTIFY, PN_RELEASED, 0);
					PRollOption->SoundingNote = y;
					SendMessage(GetParent(hWnd), WM_PROLLNOTIFY, MAKEWPARAM(PN_CLICKED, y), x);
				}
			}
		}
		break;
	case WM_LBUTTONUP:
		ReleaseCapture();
		if(PRollOption->MovingEvent || PRollOption->SizingEvent)
		{
			if(PRollOption->MovingEvent && (PRollOption->PreModifyValue != PRollOption->MovingEvent->time || PRollOption->PreModifySound != PRollOption->MovingEvent->data[0]))
				SendMessage(GetParent(hWnd), WM_PROLLNOTIFY, MAKEWPARAM(PN_EVENTMODIFIED, PNEM_MOVE), (LPARAM)PRollOption->MovingEvent);
			if(PRollOption->SizingEvent && PRollOption->PreModifyValue != getGateTime(PRollOption->SizingEvent))
				SendMessage(GetParent(hWnd), WM_PROLLNOTIFY, MAKEWPARAM(PN_EVENTMODIFIED, PNEM_SIZE), (LPARAM)PRollOption->SizingEvent);
			PRollOption->MovingEvent = NULL;
			PRollOption->SizingEvent = NULL;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		SendMessage(GetParent(hWnd), WM_PROLLNOTIFY, PN_RELEASED, 0);
		PRollOption->SoundingNote = -1;
		PRollOption->MouseMovingType = MMT_NONE;
		break;
	case WM_MBUTTONDOWN:
		if(HIWORD(lParam) >= POSBARHEIGHT)
		{
			UINT x = PRollGetScroll(hWnd) + LOWORD(lParam) * PRollOption->RollWidth;
			int y = 127 - ((HIWORD(lParam) + GetScrollPos(hWnd, SB_VERT) - POSBARHEIGHT) / PRollOption->RollHeight);

			if(0 <= y && y <= 127)
				SendMessage(GetParent(hWnd), WM_PROLLNOTIFY, MAKEWPARAM(PN_WHEELCLICKED, y), x);
		}
		return 0;
	case WM_RBUTTONDOWN:
		//削除で
		if(HIWORD(lParam) >= POSBARHEIGHT)
		{
			UINT x = PRollGetScroll(hWnd) + LOWORD(lParam) * PRollOption->RollWidth;
			int y = 127 - ((HIWORD(lParam) + GetScrollPos(hWnd, SB_VERT) - POSBARHEIGHT) / PRollOption->RollHeight);

			if(0 <= y && y <= 127 && PRollOption->smaf)
			{
				EVENT* e;
				if(e = EventHittest(hWnd, x, y))
				{
					PRollOption->HilightedEvent = e;
					SendMessage(GetParent(hWnd), WM_PROLLNOTIFY, PN_EVENTCONTEXT, (LPARAM)e);
				}else
					SendMessage(GetParent(hWnd), WM_PROLLNOTIFY, MAKEWPARAM(PN_CONTEXTMENU, y), x);
			}
		}
		break;
	case WM_KEYDOWN:
		switch((int)wParam)
		{
		case VK_DELETE:
			if(!(GetAsyncKeyState(VK_SHIFT) & 0x8000) && PRollOption->HilightedEvent)
			{
				EVENT* e = PRollOption->HilightedEvent;
				if(!PRollOption->ReadOnly)
				{
					SendMessage(GetParent(hWnd), WM_PROLLNOTIFY, PN_EVENTDELETING, (LPARAM)e);

					CutoutEvent(PRollOption->smaf, e);
					hfree(e->data);
					hfree(e);
					PRollOption->HilightedEvent = NULL;

					// TODO: DELETED need?
					SendMessage(GetParent(hWnd), WM_PROLLNOTIFY, PN_EVENTDELETED, 0);

					InvalidateRect(hWnd, NULL, FALSE);
				}
			}
			break;
		}
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

BOOL PRollInitialize(void)
{
	WNDCLASSEX wc = {0};

	wc.cbSize = sizeof(wc);
	wc.cbWndExtra = sizeof(PROLLOPTIONSTRUCT*);
	// ちょっとだけ高速化...Redraw時Bkgndで塗りつぶさなくなるから。
	//wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_MAIN));
	wc.lpfnWndProc = PRollWndProc;
	wc.lpszClassName = "MMFToolPRollWC";
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MAIN);
	wc.style = CS_DBLCLKS;

	if(!RegisterClassEx(&wc))
		return FALSE;

	return TRUE;
}
