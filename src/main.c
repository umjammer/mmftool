// This program doesn't support Unicode.
// #define UNICODE

// for Win98 or later.
#define _WIN32_WINDOWS 0x0410

#include <windows.h>
#include <commctrl.h>
#include "runtime.h"
#include "..\res\resource.h"
#include "bit.h"
#include "smaf.h"
#include "emusmw5.h"
#include "main.h"
#include "detail.h"
#include "version.h"
#include "proll.h"
#include "console.h"
#include "cuimode.h"
#include "exlayer.h"
#include "voice.h"

#define INIFILENAME "mmftool.ini"

MAINOPTION MainOption;

// TODO: go2MAINWNDVAL!
UINT PlayingPos = 0;

//?
static HWND hMainWnd;
static HWND hDetailWnd;
static HWND hToolWnd;
static HWND hListWnd;
static HWND hPRollWnd;
static HWND hStatusWnd;

//static VOICE voices[256];
static VOICE* voices = NULL;
static UINT voicesmax = 0;

COLORREF DefaultTrackColors[16] = {
	RGB(192, 192, 192),
	RGB(255, 128, 128),
	RGB(128, 255, 128),
	RGB(128, 128, 255),
	RGB(255, 255, 128),
	RGB(128, 255, 255),
	RGB(255, 128, 255),
	RGB(255, 255, 255),
	RGB(  0,   0,   0),
	RGB(128,   0,   0),
	RGB(  0, 128,   0),
	RGB(  0,   0, 128),
	RGB(128, 128,   0),
	RGB(  0, 128, 128),
	RGB(128,   0, 128),
	RGB(128, 128, 128),
};
COLORREF DefaultTrackTextColors[16] = {
	RGB(  0,   0,   0),
	RGB(  0,   0,   0),
	RGB(  0,   0,   0),
	RGB(  0,   0,   0),
	RGB(  0,   0,   0),
	RGB(  0,   0,   0),
	RGB(  0,   0,   0),
	RGB(  0,   0,   0),
	RGB(255, 255, 255),
	RGB(255, 255, 255),
	RGB(255, 255, 255),
	RGB(255, 255, 255),
	RGB(255, 255, 255),
	RGB(255, 255, 255),
	RGB(255, 255, 255),
	RGB(255, 255, 255),
};

#define IEV_INTEGER			1
#define IEV_STRING			2
static struct tagIniEntries
{
	LPCTSTR section;
	LPCTSTR key;
	UINT type;
	UINT num;
	int* val;
	int def;	// value or pointer to array

	UINT bufsize;	// For IEV_STRING
}IniEntries[] = {
	// Main
	{	"Settings",	"DrawFunc",					IEV_INTEGER,	1,	&MainOption.DrawFunc,				2},
	{	"Settings",	"FollowPlay",				IEV_INTEGER,	1,	&MainOption.FollowPlay,				0},
	{	"Settings",	"FollowSpace",				IEV_INTEGER,	1,	&MainOption.FollowSpace,			-100},
	{	"Settings",	"PlayVisibleOnly",			IEV_INTEGER,	1,	&MainOption.PlayVisibleOnly,		FALSE},
	{	"Settings",	"DontPreview",				IEV_INTEGER,	1,	&MainOption.DontPreview,			FALSE},
	{	"Settings",	"EventListViewSize",		IEV_INTEGER,	1,	&MainOption.EventListViewSize,		200},
	{	"Settings",	"PlayPosAlignByMagScale",	IEV_INTEGER,	1,	&MainOption.PlayPosAlignByMagScale,	FALSE},

	{	"Settings",	"DefaultDurationBase",		IEV_INTEGER,	1,	&MainOption.DefaultDurationBase,	4},
	{	"Settings",	"DefaultGateBase",			IEV_INTEGER,	1,	&MainOption.DefaultGateBase,		4},

	{	"Settings",	"RefreshRate",				IEV_INTEGER,	1,	&MainOption.RefreshRate,			50},
	{	"Settings",	"xDetailRefreshRate",		IEV_INTEGER,	1,	&MainOption.DetailRefreshRate,		100},

	{	"Settings",	"EventListFontFace",		IEV_STRING,		1,	(int*)MainOption.EventListFontFace,	(DWORD)(LPCTSTR)"ＭＳ ゴシック",	LF_FACESIZE},
	{	"Settings",	"EventListFontSize",		IEV_INTEGER,	1,	&MainOption.EventListFontSize,		12},

	{	"Settings",	"TrackColors",				IEV_INTEGER,	16,	MainOption.TrackColors,				(DWORD)DefaultTrackColors},
	{	"Settings",	"TrackTextColors",			IEV_INTEGER,	16,	MainOption.TrackTextColors,			(DWORD)DefaultTrackTextColors},

	// Emu
	{	"Settings",	"Emu.Tempo",				IEV_INTEGER,	1,	&MainOption.EmuOption.Tempo,		100},
	// TODO: make atoi
	{	"Settings",	"Emu.Keypitch",				IEV_INTEGER,	1,	&MainOption.EmuOption.Keypitch,		0},
	{	"Settings",	"Emu.Volume",				IEV_INTEGER,	1,	&MainOption.EmuOption.Volume,		127},

	// PianoRoll
	{	"Settings",	"PRoll.RollHeight",			IEV_INTEGER,	1,	&MainOption.PRollOption.RollHeight,			9},
	{	"Settings",	"PRoll.RollWidth",			IEV_INTEGER,	1,	&MainOption.PRollOption.RollWidth,			25},
	{	"Settings",	"PRoll.NiagaraTouch",		IEV_INTEGER,	1,	&MainOption.PRollOption.NiagaraTouch,		TRUE},
	{	"Settings",	"PRoll.DefaultGateTime",	IEV_INTEGER,	1,	&MainOption.PRollOption.DefaultGateTime,	500},
	{	"Settings",	"PRoll.ScrollMag",			IEV_INTEGER,	1,	&MainOption.PRollOption.ScrollMag,			1},
	{	"Settings",	"PRoll.ShowOctave",			IEV_INTEGER,	1,	&MainOption.PRollOption.ShowOctave,			FALSE},
	{	"Settings",	"PRoll.ReadOnly",			IEV_INTEGER,	1,	&MainOption.PRollOption.ReadOnly,			FALSE},

	// PianoRoll ColorSettings
	{	"Settings",	"PRoll.BackColor",				IEV_INTEGER,	1,	&MainOption.PRollOption.BackColor,				RGB(255, 255, 255)},
	{	"Settings",	"PRoll.NoteEdgeColor",			IEV_INTEGER,	1,	&MainOption.PRollOption.NoteEdgeColor,			RGB(128, 64, 128)},
	{	"Settings",	"PRoll.NoteHilightEdgeColor",	IEV_INTEGER,	1,	&MainOption.PRollOption.NoteHilightEdgeColor,	RGB(255, 0, 0)},
	{	"Settings",	"PRoll.NoteGridColor",			IEV_INTEGER,	1,	&MainOption.PRollOption.NoteGridColor,			RGB(220, 220, 220)},
	{	"Settings",	"PRoll.NoteHilightGridColor",	IEV_INTEGER,	1,	&MainOption.PRollOption.NoteHilightGridColor,	RGB(240, 240, 240)},
	{	"Settings",	"PRoll.NoteGridTextColor",		IEV_INTEGER,	1,	&MainOption.PRollOption.NoteGridTextColor,		RGB(192, 192, 192)},
	{	"Settings",	"PRoll.TimeGridColor",			IEV_INTEGER,	1,	&MainOption.PRollOption.TimeGridColor,			RGB(220, 220, 220)},
	{	"Settings",	"PRoll.TimeGridHilightColor",	IEV_INTEGER,	1,	&MainOption.PRollOption.TimeGridHilightColor,	RGB(192, 192, 192)},
	{	"Settings",	"PRoll.DurationGridColor",		IEV_INTEGER,	1,	&MainOption.PRollOption.DurationGridColor,		RGB(240, 240, 240)},
	{	"Settings",	"PRoll.GateGridColor",			IEV_INTEGER,	1,	&MainOption.PRollOption.GateGridColor,			RGB(240, 240, 240)},
	{	"Settings",	"PRoll.TempoGridColor",			IEV_INTEGER,	1,	&MainOption.PRollOption.TempoGridColor,			RGB(160, 160, 220)},
	{	"Settings",	"PRoll.DurationLineColor",		IEV_INTEGER,	1,	&MainOption.PRollOption.DurationLineColor,		RGB(128, 192, 128)},
	{	"Settings",	"PRoll.GateLineColor",			IEV_INTEGER,	1,	&MainOption.PRollOption.GateLineColor,			RGB(128, 128, 192)},

	{	"Settings",	"PRoll.CurrentPosColor",	IEV_INTEGER,	1,	&MainOption.PRollOption.CurrentPosColor,	RGB(255, 0, 0)},
	{	"Settings",	"PRoll.StartPosColor",		IEV_INTEGER,	1,	&MainOption.PRollOption.StartPosColor,		RGB(0, 128, 0)},
	{	"Settings",	"PRoll.EndPosColor",		IEV_INTEGER,	1,	&MainOption.PRollOption.EndPosColor,		RGB(128, 0, 128)},

	{	"Settings",	"PRoll.SystemTextFontFace",		IEV_STRING,		1,	(int*)MainOption.PRollOption.SystemTextFontFace,		(DWORD)(LPCTSTR)"ＭＳ ゴシック",	LF_FACESIZE},
	{	"Settings",	"PRoll.SystemTextFontSize",		IEV_INTEGER,	1,	&MainOption.PRollOption.SystemTextFontSize,				12},
	{	"Settings",	"PRoll.SystemTextFontWeight",	IEV_INTEGER,	1,	&MainOption.PRollOption.SystemTextFontWeight,			FW_NORMAL},

	{	"Settings",	"PRoll.PosbarTextFontFace",		IEV_STRING,		1,	(int*)MainOption.PRollOption.PosbarTextFontFace,		(DWORD)(LPCTSTR)"ＭＳ ゴシック",	LF_FACESIZE},
	{	"Settings",	"PRoll.PosbarTextFontSize",		IEV_INTEGER,	1,	&MainOption.PRollOption.PosbarTextFontSize,				12},
	{	"Settings",	"PRoll.PosbarTextFontWeight",	IEV_INTEGER,	1,	&MainOption.PRollOption.PosbarTextFontWeight,			FW_NORMAL},

	{	"Settings",	"PRoll.NoteGridTextFontFace",	IEV_STRING,		1,	(int*)MainOption.PRollOption.NoteGridTextFontFace,	(DWORD)(LPCTSTR)"ＭＳ ゴシック",	LF_FACESIZE},
	{	"Settings",	"PRoll.NoteGridTextFontSize",	IEV_INTEGER,	1,	&MainOption.PRollOption.NoteGridTextFontSize,		12},
	{	"Settings",	"PRoll.NoteGridTextFontWeight",	IEV_INTEGER,	1,	&MainOption.PRollOption.NoteGridTextFontWeight,		FW_NORMAL},
	{	"Settings",	"PRoll.NoteGridTextYOffset",	IEV_INTEGER,	1,	&MainOption.PRollOption.NoteGridTextYOffset,		0},

	//==Template==
	/*
	{	"Settings",	"",	IEV_INTEGER,	1,	&MainOption.,	},
	*/
};

typedef struct{
	// Core
	BOOL emusounding;
	BOOL midisounding;
	SMAF* smaf;

	// Shift-Option
	int inputtingnum;
	char inputtingtype;
	BOOL isinputting;
	BOOL isinputfirst;
	char inputescapestr[16];

	// Event-PRoll divider
	int separate_offset;

	// MIDI Note Test
	int sndy;
	int sndch;

	int pch;

	// for EventList-PianoRoll sync
	EVENT* selectedevent;
} MAINWNDVAL;

BOOL CALLBACK AboutDlgWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if(msg == WM_INITDIALOG)
	{
		VERSION v;
		TCHAR ts[64];
		TCHAR exedir[MAX_PATH];
		TCHAR fname[MAX_PATH];

		if(getVersion(&v))
		{
			wsprintf(ts, "Version %d.%d.%d.%d", v.major, v.minor, v.patch, v.build);
			SetDlgItemText(hWnd, IDC_VERSION, ts);
		}

		GetModuleFileName(NULL, exedir, sizeof(exedir));
		*(LPTSTR)basename(exedir) = '\0';
		addEndYen(exedir);

		lstrcpy(fname, exedir);
		lstrcat(fname, "M5_EmuSmw5.dll");
		if(getFileVersion(&v, fname))
		{
			wsprintf(ts, "Version %d.%d.%d.%d", v.major, v.minor, v.patch, v.build);
			SetDlgItemText(hWnd, IDC_SMW5VERSION, ts);
		}

		lstrcpy(fname, exedir);
		lstrcat(fname, "M5_EmuHw.dll");
		if(getFileVersion(&v, fname))
		{
			wsprintf(ts, "Version %d.%d.%d.%d", v.major, v.minor, v.patch, v.build);
			SetDlgItemText(hWnd, IDC_HWVERSION, ts);
		}

		return TRUE;
	}
	if(msg == WM_COMMAND && (wParam == IDOK || wParam == IDCANCEL))
		EndDialog(hWnd, wParam);

	return FALSE;
}

BOOL CALLBACK SmafSBDlgWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if(msg == WM_INITDIALOG)
		return TRUE;
	if(msg == WM_COMMAND && LOWORD(wParam) == IDCANCEL)
		PostMessage(hWnd, WM_CLOSE, 0, 0);
	return FALSE;
}

BOOL SmafCB(DWORD cur, DWORD size, UINT state, LPARAM param)
{
	static HWND hpwnd = NULL;

	switch(state)
	{
	case SCBSTATE_BEGIN:
		EnableWindow(hMainWnd, FALSE);
		hpwnd = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_PROGRESS), hMainWnd, SmafSBDlgWndProc);
		SendMessage(GetDlgItem(hpwnd, IDC_PROGRESS), PBM_SETRANGE, 0, MAKELPARAM(0, 1000));
		SendMessage(GetDlgItem(hpwnd, IDC_PROGRESS), PBM_SETPOS, 0, 0);

		ShowWindow(hpwnd, SW_SHOW);
		SetForegroundWindow(hpwnd);

		break;
	case SCBSTATE_PROCESSING:
		SendMessage(GetDlgItem(hpwnd, IDC_PROGRESS), PBM_SETPOS, (WPARAM)(WORD)(cur * 1000 / size), 0);
		RedrawWindow(hpwnd, NULL, NULL, RDW_UPDATENOW);
		{
			MSG msg;

			while(PeekMessage(&msg, hpwnd, 0, 0, PM_REMOVE))
			{
				if(msg.message == WM_CLOSE)
					if(MessageBox(hpwnd, "Abort converting?\n\n(NOTICE: if you say Yes, You SHOULD NOT compile or save! some events will be lost!)", "Question", MB_ICONQUESTION | MB_YESNO) == IDYES)
						return FALSE;
					else
						continue;
				if(IsDialogMessage(hpwnd, &msg))
					continue;
				DispatchMessage(&msg);
			}
		}
		break;
	case SCBSTATE_FINISH:
		DestroyWindow(hpwnd);
		hpwnd = NULL;
		EnableWindow(hMainWnd, TRUE);
		SetForegroundWindow(hMainWnd);
		break;
	}

	return TRUE;
}

void getTrackMenuItem(int id, LPTSTR str, COLORREF* fg, COLORREF* bg)
{
	COLORREF dummy;

	if(!fg)
		fg = &dummy;
	if(!bg)
		bg = &dummy;

	// TODO: fg color to calc from bg
	switch(id)
	{
	case ID_VTRACK1:	*fg = MainOption.TrackTextColors[0];	*bg = MainOption.TrackColors[0];	lstrcpy(str, "1:Track1(Silver)");		break;
	case ID_VTRACK2:	*fg = MainOption.TrackTextColors[1];	*bg = MainOption.TrackColors[1];	lstrcpy(str, "2:Track2(Red)");			break;
	case ID_VTRACK3:	*fg = MainOption.TrackTextColors[2];	*bg = MainOption.TrackColors[2];	lstrcpy(str, "3:Track3(Green)");		break;
	case ID_VTRACK4:	*fg = MainOption.TrackTextColors[3];	*bg = MainOption.TrackColors[3];	lstrcpy(str, "4:Track4(Blue)");			break;
	case ID_VTRACK5:	*fg = MainOption.TrackTextColors[4];	*bg = MainOption.TrackColors[4];	lstrcpy(str, "5:Track5(Yellow)");		break;
	case ID_VTRACK6:	*fg = MainOption.TrackTextColors[5];	*bg = MainOption.TrackColors[5];	lstrcpy(str, "6:Track6(Cyan)");			break;
	case ID_VTRACK7:	*fg = MainOption.TrackTextColors[6];	*bg = MainOption.TrackColors[6];	lstrcpy(str, "7:Track7(Purple)");		break;
	case ID_VTRACK8:	*fg = MainOption.TrackTextColors[7];	*bg = MainOption.TrackColors[7];	lstrcpy(str, "8:Track8(White)");		break;
	case ID_VTRACK9:	*fg = MainOption.TrackTextColors[8];	*bg = MainOption.TrackColors[8];	lstrcpy(str, "Q:Track9(Black)");		break;
	case ID_VTRACK10:	*fg = MainOption.TrackTextColors[9];	*bg = MainOption.TrackColors[9];	lstrcpy(str, "W:Track10(DarkRed)");		break;
	case ID_VTRACK11:	*fg = MainOption.TrackTextColors[10];	*bg = MainOption.TrackColors[10];	lstrcpy(str, "E:Track11(DarkGreen)");	break;
	case ID_VTRACK12:	*fg = MainOption.TrackTextColors[11];	*bg = MainOption.TrackColors[11];	lstrcpy(str, "R:Track12(DarkBlue)");	break;
	case ID_VTRACK13:	*fg = MainOption.TrackTextColors[12];	*bg = MainOption.TrackColors[12];	lstrcpy(str, "T:Track13(DarkYellow)");	break;
	case ID_VTRACK14:	*fg = MainOption.TrackTextColors[13];	*bg = MainOption.TrackColors[13];	lstrcpy(str, "Y:Track14(DarkCyan)");	break;
	case ID_VTRACK15:	*fg = MainOption.TrackTextColors[14];	*bg = MainOption.TrackColors[14];	lstrcpy(str, "U:Track15(DarkPurple)");	break;
	case ID_VTRACK16:	*fg = MainOption.TrackTextColors[15];	*bg = MainOption.TrackColors[15];	lstrcpy(str, "I:Track16(Gray)");		break;
	}

	return;
}

void seek(SMAF* smaf, UINT ppos, BOOL emuseek, BOOL forcemove)
{
	PlayingPos = ppos;
	SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETCURRENTPOS, PlayingPos);

	if(emuseek)
	{
		if(!EmuSeek(PlayingPos - (smaf->start->time - smaf->start->duration)))
		{
			MessageBox(hMainWnd, "Seek failed", "Error", MB_ICONERROR | MB_OK);
		}
	}

	if(forcemove)
	{
		if(MainOption.FollowPlay)
			SendMessage(hPRollWnd, WM_PROLL, MAKEWPARAM(PROLLWM_SCROLLTO, MainOption.FollowSpace), PlayingPos);
			//SetScrollPos(hMainWnd, SB_HORZ, (int)(PlayingPos) / (int)RollWidth + FollowSpace, TRUE);
	}else
	{
		if(MainOption.FollowPlay == 1 || ((MainOption.FollowPlay == 2 || MainOption.FollowPlay == 3) && GetAsyncKeyState(VK_SHIFT) & 0x8000))
			SendMessage(hPRollWnd, WM_PROLL, MAKEWPARAM(PROLLWM_SCROLLTO, MainOption.FollowSpace), PlayingPos);
			//SetScrollPos(hMainWnd, SB_HORZ, (int)(PlayingPos) / (int)RollWidth + FollowSpace, TRUE);
	}

	InvalidateRect(hMainWnd, NULL, FALSE);
	InvalidateRect(hDetailWnd, NULL, FALSE);

	return;
}

void InitMidiMode(void)
{
	// terminate
	// after
#if 0
	Emu526829();

	// MONOPHOLIC EXCLUSIVES?
	//*
	{
		BYTE cmd[] = {0xF0, 0x43, 0x79, 0x06, 0x7F, 0x13, 0x00, 0xF7};
		if(EmuSetMidiMsg(cmd, sizeof(cmd)))
		{
			MessageBox(NULL, "InitMidiMode F0 43 79 06 7F 13 00 F7 failed", "Error", MB_ICONERROR | MB_OK);
			return;
		}
	}
	{
		BYTE cmd[] = {0xF0, 0x43, 0x79, 0x06, 0x7F, 0x11, 0x00, 0xF7};
		if(EmuSetMidiMsg(cmd, sizeof(cmd)))
		{
			MessageBox(NULL, "InitMidiMode F0 43 79 06 7F 11 00 F7 failed", "Error", MB_ICONERROR | MB_OK);
			return;
		}
	}
	//*/

	//*
	Emu5282();

	// いらなくね?
	{
		BYTE cmd[] = {0xF0, 0x43, 0x79, 0x06, 0x7F, 0x13, 0x00, 0xF7};
		if(EmuSetMidiMsg(cmd, sizeof(cmd)))
		{
			MessageBox(NULL, "SetMidiMsgX1 failed", "Error", MB_ICONERROR | MB_OK);
			return;
		}
	}
	{
		BYTE cmd[] = {0xF0, 0x43, 0x79, 0x06, 0x7F, 0x11, 0x00, 0xF7};
		if(EmuSetMidiMsg(cmd, sizeof(cmd)))
		{
			MessageBox(NULL, "SetMidiMsgX2 failed", "Error", MB_ICONERROR | MB_OK);
			return;
		}
	}

	// いらなくね?
#if 0
	// CC BankSelect
	{
		BYTE cmd[] = {0xB0, 0x00, 0x7C};
		if(EmuSetMidiMsg(cmd, sizeof(cmd)))
		{
			MessageBox(NULL, "SetMidiMsg5 failed", "Error", MB_ICONERROR | MB_OK);
			return;
		}
	}
	{
		BYTE cmd[] = {0xB0, 0x20, 0x00};
		if(EmuSetMidiMsg(cmd, sizeof(cmd)))
		{
			MessageBox(NULL, "SetMidiMsg6 failed", "Error", MB_ICONERROR | MB_OK);
			return;
		}
	}
	// PC
	{
		BYTE cmd[] = {0xC0, 0};
		if(EmuSetMidiMsg(cmd, sizeof(cmd)))
		{
			MessageBox(NULL, "SetMidiMsg7 failed", "Error", MB_ICONERROR | MB_OK);
			return;
		}
	}
#endif
	//*/

	//*
	// unknown exclusive
	{
		BYTE cmd[] = {0xF0, 0x43, 0x79, 0x06, 0x7F, 0x02, 0x7C, 0x00, 0x00, 0x00, 0xF7};
		if(EmuSetMidiMsg(cmd, sizeof(cmd)))
		{
			MessageBox(NULL, "SetMidiMsgX4 failed", "Error", MB_ICONERROR | MB_OK);
			return;
		}
	}
	//*/
#endif

	// Reset
	{
		BYTE cmd[] = {0xF0, 0x43, 0x79, 0x06, 0x7F, 0x7F, 0xF7};
		if(EmuSetMidiMsg(cmd, sizeof(cmd)))
		{
			MessageBox(NULL, "SetMidiMsg MIDIReset failed", "Error", MB_ICONERROR | MB_OK);
			return;
		}
	}

	EmuMIDISetVolume();

	PresetVoices(voices, voicesmax);

	return;
}

void settitle(HWND hWnd, SMAF* smaf, BOOL emusounding, BOOL midisounding)
{
	if(!smaf)
		SetWindowText(hWnd, APPNAME);
	else
	{
		if(emusounding)
		{
			TCHAR ts[256];
			DWORD emuinfo = EmuInfo();

			wsprintf(ts, APPNAME" [%s] EMU:%05ums ", smaf->name, PlayingPos);
			if(emuinfo & 0x01)
				lstrcat(ts, "LED ");
			if(emuinfo & 0x02)
				lstrcat(ts, "Vib ");
			SetWindowText(hWnd, ts);
		}else
			if(midisounding)
			{
				TCHAR ts[256];
				//DWORD emuinfo = EmuInfo();	// Emuだけだが、MIDIでもできるようにしたい。

				wsprintf(ts, APPNAME" [%s] MIDI:%05ums", smaf->name, PlayingPos);
				SetWindowText(hWnd, ts);
			}else
			{
				TCHAR ts[32];

				wsprintf(ts, APPNAME" [%s] %05ums", smaf->name, PlayingPos);
				SetWindowText(hWnd, ts);
			}
	}

	return;
}

void setStatus(SMAF* smaf, BOOL emuplay, BOOL midiplay)
{
	//=== TOOLBAR ===//
	if(smaf)
	{
		SendMessage(hToolWnd, TB_SETSTATE, ID_CLOSE,			TBSTATE_ENABLED);
		SendMessage(hToolWnd, TB_SETSTATE, ID_COMPILESMAF,		TBSTATE_ENABLED);
		if(smaf->data)
			SendMessage(hToolWnd, TB_SETSTATE, ID_SAVESMAF,		TBSTATE_ENABLED);

		if(emuplay || midiplay)
		{
			SendMessage(hToolWnd, TB_SETSTATE, ID_PLAY,			0);
			SendMessage(hToolWnd, TB_SETSTATE, ID_MIDIPLAY,		0);
			SendMessage(hToolWnd, TB_SETSTATE, ID_STOP,			TBSTATE_ENABLED);
		}else
		{
			SendMessage(hToolWnd, TB_SETSTATE, ID_PLAY,			TBSTATE_ENABLED);
			SendMessage(hToolWnd, TB_SETSTATE, ID_MIDIPLAY,		TBSTATE_ENABLED);
			SendMessage(hToolWnd, TB_SETSTATE, ID_STOP,			0);
		}
	}else
	{
		SendMessage(hToolWnd, TB_SETSTATE, ID_CLOSE,			0);
		SendMessage(hToolWnd, TB_SETSTATE, ID_COMPILESMAF,		0);
		SendMessage(hToolWnd, TB_SETSTATE, ID_SAVESMAF,			0);

		SendMessage(hToolWnd, TB_SETSTATE, ID_PLAY,				0);
		SendMessage(hToolWnd, TB_SETSTATE, ID_MIDIPLAY,			0);
		SendMessage(hToolWnd, TB_SETSTATE, ID_STOP,				0);
	}

	if(emuplay)
		SendMessage(hToolWnd, TB_SETSTATE, ID_MIDIRESET,		0);
	else
		SendMessage(hToolWnd, TB_SETSTATE, ID_MIDIRESET,		TBSTATE_ENABLED);

	if(SendMessage(hPRollWnd, WM_PROLL, PROLLWM_GETNIAGARATOUCH, 0))
		SendMessage(hToolWnd, TB_SETSTATE, ID_TOGGLENIAGARA,	TBSTATE_ENABLED | TBSTATE_CHECKED);
	else
		SendMessage(hToolWnd, TB_SETSTATE, ID_TOGGLENIAGARA,	TBSTATE_ENABLED);

	if(MainOption.DontPreview)
		SendMessage(hToolWnd, TB_SETSTATE, ID_TOGGLENP,			TBSTATE_ENABLED | TBSTATE_CHECKED);
	else
		SendMessage(hToolWnd, TB_SETSTATE, ID_TOGGLENP,			TBSTATE_ENABLED);

	if(MainOption.PRollOption.ReadOnly)
		SendMessage(hToolWnd, TB_SETSTATE, ID_TOGGLERO,			TBSTATE_ENABLED | TBSTATE_CHECKED);
	else
		SendMessage(hToolWnd, TB_SETSTATE, ID_TOGGLERO,			TBSTATE_ENABLED);

	//=== STATUS BAR ===//
	SendMessage(hStatusWnd, SB_SETTEXT, 0, (LPARAM)APPNAME" by Murachue - http://murachue.ddo.jp/");
	if(!smaf)
		SendMessage(hStatusWnd, SB_SETTEXT, 1, (LPARAM)"\tEmpty");
	else
	{
		switch(smaf->format)
		{
		case SMAFF_NONE:	SendMessage(hStatusWnd, SB_SETTEXT, 1, (LPARAM)"\tNone");	break;
		case SMAFF_MA1:		SendMessage(hStatusWnd, SB_SETTEXT, 1, (LPARAM)"\tMA-1");	break;
		case SMAFF_MA2:		SendMessage(hStatusWnd, SB_SETTEXT, 1, (LPARAM)"\tMA-2");	break;
		case SMAFF_MA3:		SendMessage(hStatusWnd, SB_SETTEXT, 1, (LPARAM)"\tMA-3");	break;
		case SMAFF_MA5:		SendMessage(hStatusWnd, SB_SETTEXT, 1, (LPARAM)"\tMA-5");	break;
		case SMAFF_MA7:		SendMessage(hStatusWnd, SB_SETTEXT, 1, (LPARAM)"\tMA-7");	break;
		case SMAFF_UTA2:	SendMessage(hStatusWnd, SB_SETTEXT, 1, (LPARAM)"\tEseuta MA-2");	break;
		case SMAFF_UTA3:	SendMessage(hStatusWnd, SB_SETTEXT, 1, (LPARAM)"\tEseuta MA-3");	break;
		default:			SendMessage(hStatusWnd, SB_SETTEXT, 1, (LPARAM)"\t???");	break;
		}
	}

	return;
}

void setEventListItem(int itemid, EVENT* e)
{
	TCHAR ts[256];

	wsprintf(ts, "%u", e->time);
	ListView_SetItemText(hListWnd, itemid, 0, ts);

	switch(e->status & 0xF0)
	{
	case 0x80:
	case 0x90:
	case 0xB0:
	case 0xC0:
	case 0xE0:
		wsprintf(ts, "%u", (e->status & 0x0F) + 1);
		break;
	default:
		wsprintf(ts, "-");
		break;
	}
	ListView_SetItemText(hListWnd, itemid, 1, ts);

	switch(e->status & 0xF0)
	{
	case 0x80:
	case 0x90:
		{
			LPCTSTR note[12] = {"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"};

			wsprintf(ts, "%s%d", note[e->data[0] % 12], e->data[0] / 12 - 2);
			if((e->status & 0xF0) == 0x90)
				wsprintf(ts, "%s(v%d)", ts, e->data[1]);
		}
		break;
	case 0xB0:
		lstrcpy(ts, "CC");
		break;
	case 0xC0:
		lstrcpy(ts, "PC");
		break;
	case 0xE0:
		lstrcpy(ts, "PB");
		break;
	case 0xF0:
		if(e->size == 1 && e->data[0] == 0x00)
			lstrcpy(ts, "NOP");
		else
			if(e->size == 2 && e->data[0] == 0x2F && e->data[1] == 0x00)
				lstrcpy(ts, "EOS");
			else
			{
				wsprintf(ts, "Exclusive(%d)", GetVVal(e->data));
			}
		break;
	default:
		wsprintf(ts, "%02X", e->status);
		break;
	}
	ListView_SetItemText(hListWnd, itemid, 2, ts);

	if(e->data)
	{
		switch(e->status & 0xF0)
		{
		case 0x80:
		case 0x90:
			wsprintf(ts, "%dms", GetVVal(e->data + 1 + (((e->status & 0xF0) == 0x90) ? 1 : 0)));
			break;
		default:
			{
			UINT i;
			UINT max;

			max = e->size;
			if(64 < max)
				max = 64;

			lstrcpy(ts, "");

			for(i = (e->status == 0xF0) ? GetVValSize(e->data) : 0; i < max; i++)
			{
				wsprintf(ts, "%s%02X ", ts, e->data[i]);
			}
			if(64 < e->size)
				wsprintf(ts, "%s...", ts);
			}
			break;
		}

		ListView_SetItemText(hListWnd, itemid, 3, ts);
	}
}

void RefreshEventList(SMAF* smaf, int track)
{
	int preselid = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);
	EVENT* presel = NULL;

	if(preselid != -1)
	{
		LV_ITEM lvi;

		lvi.iItem = preselid;
		lvi.iSubItem = 0;
		lvi.mask = LVIF_PARAM;

		ListView_GetItem(hListWnd, &lvi);

		presel = (EVENT*)lvi.lParam;
	}

	/*
	if(smaf && (EVENT*)SendMessage(hPRollWnd, WM_PROLL, PROLLWM_GETHILIGHTEVENT, 0) != NULL)
	{
		EVENT* hl = (EVENT*)SendMessage(hPRollWnd, WM_PROLL, PROLLWM_GETHILIGHTEVENT, 0);

		for(presel = smaf->events; presel; presel = presel->next)
			if(presel == hl)
				break;
	}
	*/

	ListView_DeleteAllItems(hListWnd);

	if(smaf && smaf->events)
	{
		LV_ITEM lvi;
		EVENT* e;

		lvi.mask = LVIF_PARAM;

		for(e = smaf->events; e; e = e->next)
		{
			if(track != -1)
			{
				BYTE st = e->status & 0xF0;

				if(	st == 0x80
					|| st == 0x90
					|| st == 0xB0
					|| st == 0xC0
					|| st == 0xE0
					)
					if((e->status & 0x0F) != track)
						continue;
			}

			lvi.iItem = ListView_GetItemCount(hListWnd);
			lvi.iSubItem = 0;
			lvi.lParam = (LPARAM)e;
			ListView_InsertItem(hListWnd, &lvi);

			setEventListItem(lvi.iItem, e);

			if(e == presel)
			{
				ListView_SetItemState(hListWnd, lvi.iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				preselid = lvi.iItem;
			}
		}

		if(presel != NULL)
			ListView_EnsureVisible(hListWnd, preselid, FALSE);
	}

	return;
}

void OpenSMAFCommand(HWND hWnd, SMAF** smaf, LPCTSTR fname)
{
	if(*smaf)
		SendMessage(hWnd, WM_COMMAND, ID_CLOSE, 0);

	PlayingPos = 0;

	if(*smaf = LoadSmaf(fname))
	{
		//SetScrollPos(hWnd, SB_HORZ, 0, TRUE);
		//SetScrollPos(hWnd, SB_VERT, 0, TRUE);

		SendMessage(hDetailWnd, WM_USER, 0, (LPARAM)*smaf);
		SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETSMAF, (LPARAM)*smaf);
		SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SCROLLTO, 0);

		settitle(hWnd, *smaf, FALSE, FALSE);
		setStatus(*smaf, FALSE, FALSE);
		RefreshEventList(*smaf, SendMessage(hPRollWnd, WM_PROLL, PROLLWM_GETVISIBLETRACK, 0));

		ShowWindow(hListWnd, SW_SHOW);
		ShowWindow(hPRollWnd, SW_SHOW);
		InvalidateRect(hWnd, NULL, TRUE);
	}

	return;
}

void setvtrack(int track)
{
	SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETVISIBLETRACK, track);
	if(MainOption.PlayVisibleOnly)
		EmuMIDISetSoloCh(track);
	InvalidateRect(hPRollWnd, NULL, FALSE);

	return;
}

void formatso(BYTE inputtingtype, int inputtingnum, char* inputescapestr)
{
	LPTSTR str = (LPTSTR)SendMessage(hPRollWnd, WM_PROLL, PROLLWM_GETOPTIONSTRING, 0);

	switch(inputtingtype)
	{
	case 0:
		break;
	case VK_ESCAPE:
		if(!lstrcmp(inputescapestr, "SVEN")){	wsprintf(str, "ShowVisibleEventNum = %d", inputtingnum); break;}
		if(!lstrcmp(inputescapestr, "GB")){		wsprintf(str, "GateBase = %d", inputtingnum); break;}
		if(!lstrcmp(inputescapestr, "DB")){		wsprintf(str, "DurationBase = %d", inputtingnum); break;}

		wsprintf(str, "(Escape)%s = %d", inputescapestr, inputtingnum);
		break;
	case VK_OEM_2:	wsprintf(str, "HELP: View readme. sorry.");				break;
	case 'R':		wsprintf(str, "Refresh Rate: %d [ms]", inputtingnum);	break;
	case 'D':
		switch(inputtingnum)
		{
		case 0:
			wsprintf(str, "Draw Function: %d: No DoubleBuffer", inputtingnum);
			break;
		case 1:
			wsprintf(str, "Draw Function: %d: DoubleBuffer use CreateDIBSection(SLOW)", inputtingnum);
			break;
		case 2:
			wsprintf(str, "Draw Function: %d: DoubleBuffer use CreateCompatibleBitmap(Fast?)", inputtingnum);
			break;
		default:
			wsprintf(str, "Draw Function: %d (ILLEGAL VALUE)", inputtingnum);
			break;
		}
		break;
	case 'H':	wsprintf(str, "PianoRoll's Height: %d", inputtingnum);			break;
	case 'T':	wsprintf(str, "Tempo: %d", inputtingnum);						break;
	case 'K':	wsprintf(str, "KeyPitch: %d", inputtingnum);						break;
	case 'W':	wsprintf(str, "PianoRoll's Width divider: %d", inputtingnum);	break;
	case 'F':
		switch(inputtingnum)
		{
		case 0:
			wsprintf(str, "Follow play: %d: NoFollow", inputtingnum);
			break;
		case 1:
			wsprintf(str, "Follow play: %d: AlwaysFollow", inputtingnum);
			break;
		case 2:
			wsprintf(str, "Follow play: %d: FollowOnShift", inputtingnum);
			break;
		case 3:
			wsprintf(str, "Follow play: %d: PageFollow", inputtingnum);
			break;
		default:
			wsprintf(str, "Follow play: %d (ILLEGAL VALUE)", inputtingnum);
			break;
		}
		break;
	case 'S':	wsprintf(str, "Follow space: %d [px]", -inputtingnum);				break;
	case 'G':
		wsprintf(str, "GoTo: %d [ms]", inputtingnum);
		break;
	case 'V':	wsprintf(str, "Volume: %d", inputtingnum);						break;
	case 'M':	wsprintf(str, "MoveTo: %d", inputtingnum);						break;
	case 'O':	wsprintf(str, "PlayVisibleOnly: %d", inputtingnum);				break;
	case 'L':	wsprintf(str, "MagScale: %d", inputtingnum);				break;
	case 'A':	wsprintf(str, "PlayPosAlignByMagScale: %d", inputtingnum);		break;
	default:	wsprintf(str, "INVALID TYPE");									break;
	}

	return;
}

BOOL doCompile(SMAF* smaf, BOOL emusounding, BOOL midisounding)
{
	BYTE* dt;
	DWORD ds;

	if(emusounding || midisounding)	// dangerous!
	{
		MessageBox(hMainWnd, "Now playing, compiling is dangerous!!\nAbort compilation.", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	if(dt = CompileSmaf(smaf, &ds))
	{
		hfree(smaf->data);
		smaf->data = dt;
		smaf->size = ds;

		SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETHILIGHTEVENT, 0);

		ReloadSmaf(smaf);
		setStatus(smaf, emusounding, midisounding);

		InvalidateRect(hDetailWnd, NULL, FALSE);
		InvalidateRect(hPRollWnd, NULL, FALSE);

		return TRUE;
	}

	return FALSE;
}

int getClickedStatusPane(HWND hWnd, int x)
{
	UINT parts;
	INT* pw;
	UINT pane;
	RECT rc;

	if(!(parts = SendMessage(hWnd, SB_GETPARTS, 0, (LPARAM)NULL)))
		return -1;

	GetClientRect(hWnd, &rc);

	pw = halloc(parts * sizeof(INT));

	SendMessage(hWnd, SB_GETPARTS, parts, (LPARAM)pw);
	for(pane = 0; pane < parts; pane++)
		if(x < (pw[pane] == -1 ? rc.right : pw[pane]))
			break;
	hfree(pw);

	if(pane == parts)
		return -1;
	else
		return pane;
}

LRESULT CALLBACK StatusWndSub(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// NOT Window-independent code!
	static DWORD PreSec;
	static BOOL TempoMode = FALSE;
	static UINT Tempo;
	static BOOL Metronoming;
	static BOOL Blinking;

	switch(msg)
	{
	case WM_APP:
		SetWindowLong(hWnd, GWL_USERDATA, GetWindowLong(hWnd, GWL_WNDPROC));
		return 0;
	case WM_LBUTTONDOWN:
		if(wParam & MK_CONTROL)
		{
			SendMessage(hWnd, WM_MBUTTONDOWN, 0, lParam);
			return 0;
		}

		if(TempoMode)
			if(getClickedStatusPane(hWnd, LOWORD(lParam)) == 1)
			{
				TCHAR ts[32];
				DWORD tick = GetTickCount();
				DWORD diff = tick - PreSec;

				// Tempo = 1000 / diff * 60
				Tempo = 60000 / diff;
				if(Tempo < 1)
					Tempo = 1;
				wsprintf(ts, "\t%u(%ums)", Tempo, diff);

				if(Blinking)
					SendMessage(hWnd, WM_TIMER, 1, (LPARAM)NULL);

				SendMessage(hWnd, SB_SETTEXT, 1 | SBT_POPOUT, (LPARAM)ts);

				SendMessage(GetParent(hWnd), WM_APP, 0, diff);

				if(Metronoming)
				{
					KillTimer(hWnd, 0);
					SetTimer(hWnd, 0, (1000*1000) / ((Tempo*1000) / 60), NULL);
				}

				PreSec = tick;
			}
		break;
	case WM_LBUTTONDBLCLK:
		switch(getClickedStatusPane(hWnd, LOWORD(lParam)))
		{
		case 0:
			if(MessageBox(hWnd, "Jump to website?\n(http://murachue.ddo.jp/)", "Question", MB_ICONQUESTION | MB_YESNO) == IDYES)
				ShellExecute(hWnd, NULL, "http://murachue.ddo.jp/", NULL, NULL, SW_SHOWDEFAULT);
			break;
		case 1:
			if(TempoMode)
				SendMessage(hWnd, WM_LBUTTONDOWN, wParam, lParam);
			else
			{
				SendMessage(hWnd, SB_SETTEXT, 1 | SBT_POPOUT, (LPARAM)"\tTempoChecker");
				TempoMode = TRUE;
				PreSec = GetTickCount();
				Tempo = 0;
				Metronoming = FALSE;
			}
			break;
		}
		break;
	case WM_MBUTTONDOWN:
		if(TempoMode)
		{
			HMENU hpm;
			MENUITEMINFO mii;
			TCHAR ts[16];

			hpm = CreatePopupMenu();

			mii.cbSize = sizeof(mii);
			mii.fMask = MIIM_ID | MIIM_STATE | MIIM_TYPE;

			mii.fType = MFT_STRING;	mii.wID = 1;	mii.fState = Metronoming ? (MFS_CHECKED|MFS_DISABLED) : 0;	mii.dwTypeData = "Metronome";	InsertMenuItem(hpm, GetMenuItemCount(hpm), TRUE, &mii);
			mii.fType = MFT_SEPARATOR;														InsertMenuItem(hpm, GetMenuItemCount(hpm), TRUE, &mii);
			mii.fType = MFT_STRING;	mii.wID = 60;	mii.fState = (Tempo == mii.wID) ? MFS_CHECKED : 0;	wsprintf(ts, "%u", mii.wID); mii.dwTypeData = ts;	InsertMenuItem(hpm, GetMenuItemCount(hpm), TRUE, &mii);
			mii.fType = MFT_STRING;	mii.wID = 72;	mii.fState = (Tempo == mii.wID) ? MFS_CHECKED : 0;	wsprintf(ts, "%u", mii.wID); mii.dwTypeData = ts;	InsertMenuItem(hpm, GetMenuItemCount(hpm), TRUE, &mii);
			mii.fType = MFT_STRING;	mii.wID = 80;	mii.fState = (Tempo == mii.wID) ? MFS_CHECKED : 0;	wsprintf(ts, "%u", mii.wID); mii.dwTypeData = ts;	InsertMenuItem(hpm, GetMenuItemCount(hpm), TRUE, &mii);
			mii.fType = MFT_STRING;	mii.wID = 90;	mii.fState = (Tempo == mii.wID) ? MFS_CHECKED : 0;	wsprintf(ts, "%u", mii.wID); mii.dwTypeData = ts;	InsertMenuItem(hpm, GetMenuItemCount(hpm), TRUE, &mii);
			mii.fType = MFT_STRING;	mii.wID = 120;	mii.fState = (Tempo == mii.wID) ? MFS_CHECKED : 0;	wsprintf(ts, "%u", mii.wID); mii.dwTypeData = ts;	InsertMenuItem(hpm, GetMenuItemCount(hpm), TRUE, &mii);
			mii.fType = MFT_STRING;	mii.wID = 150;	mii.fState = (Tempo == mii.wID) ? MFS_CHECKED : 0;	wsprintf(ts, "%u", mii.wID); mii.dwTypeData = ts;	InsertMenuItem(hpm, GetMenuItemCount(hpm), TRUE, &mii);
			mii.fType = MFT_STRING;	mii.wID = 160;	mii.fState = (Tempo == mii.wID) ? MFS_CHECKED : 0;	wsprintf(ts, "%u", mii.wID); mii.dwTypeData = ts;	InsertMenuItem(hpm, GetMenuItemCount(hpm), TRUE, &mii);
			mii.fType = MFT_STRING;	mii.wID = 180;	mii.fState = (Tempo == mii.wID) ? MFS_CHECKED : 0;	wsprintf(ts, "%u", mii.wID); mii.dwTypeData = ts;	InsertMenuItem(hpm, GetMenuItemCount(hpm), TRUE, &mii);
			mii.fType = MFT_SEPARATOR;														InsertMenuItem(hpm, GetMenuItemCount(hpm), TRUE, &mii);
			mii.fType = MFT_STRING;	mii.wID = 12;	mii.fState = 0;	mii.dwTypeData = "-10";	InsertMenuItem(hpm, GetMenuItemCount(hpm), TRUE, &mii);
			mii.fType = MFT_STRING;	mii.wID = 11;	mii.fState = 0;	mii.dwTypeData = "-5";	InsertMenuItem(hpm, GetMenuItemCount(hpm), TRUE, &mii);
			mii.fType = MFT_STRING;	mii.wID = 10;	mii.fState = 0;	mii.dwTypeData = "-1";	InsertMenuItem(hpm, GetMenuItemCount(hpm), TRUE, &mii);
			mii.fType = MFT_STRING;	mii.wID = 20;	mii.fState = 0;	mii.dwTypeData = "+1";	InsertMenuItem(hpm, GetMenuItemCount(hpm), TRUE, &mii);
			mii.fType = MFT_STRING;	mii.wID = 21;	mii.fState = 0;	mii.dwTypeData = "+5";	InsertMenuItem(hpm, GetMenuItemCount(hpm), TRUE, &mii);
			mii.fType = MFT_STRING;	mii.wID = 22;	mii.fState = 0;	mii.dwTypeData = "+10";	InsertMenuItem(hpm, GetMenuItemCount(hpm), TRUE, &mii);
			mii.fType = MFT_SEPARATOR;														InsertMenuItem(hpm, GetMenuItemCount(hpm), TRUE, &mii);
			mii.fType = MFT_STRING;	mii.wID = 30;	mii.fState = 0;	mii.dwTypeData = "Now!";	InsertMenuItem(hpm, GetMenuItemCount(hpm), TRUE, &mii);

			{
				POINT p;
				int ret;

				p.x = LOWORD(lParam);
				p.y = HIWORD(lParam);
				ClientToScreen(hWnd, &p);
				ret = (int)TrackPopupMenu(hpm, TPM_RETURNCMD, p.x, p.y, 0, hWnd, NULL);

				if(ret == 1)
				{
					if(Tempo == 0)
					{
						if(Blinking)
							SendMessage(hWnd, WM_TIMER, 1, (LPARAM)NULL);

						if(!SendMessage(hWnd, SB_ISSIMPLE, 0, 0))
						{
							TCHAR ts[32];

							Tempo = 60;

							// Do not use float..._ftoi referenced by compiler...there is no C-Runtime...
							wsprintf(ts, "\t%u(%ums)", Tempo, (1000*1000) / ((Tempo*1000) / 60));

							SendMessage(hWnd, SB_SETTEXT, 1 | SBT_POPOUT, (LPARAM)ts);

							SendMessage(GetParent(hWnd), WM_APP, 0, (1000*1000) / ((Tempo*1000) / 60));
						}
					}

					Metronoming = TRUE;
					SetTimer(hWnd, 0, (1000*1000) / ((Tempo*1000) / 60), NULL);
					Blinking = FALSE;
				}

				if(10 <= ret && ret < 30)
				{
					switch(ret)
					{
					case 10:	Tempo = (0 < (Tempo - 1)) ? (Tempo - 1) : 1;		break;
					case 11:	Tempo = (0 < (Tempo - 5)) ? (Tempo - 5) : 1;		break;
					case 12:	Tempo = (0 < (Tempo - 10)) ? (Tempo - 10) : 1;		break;
					case 20:	Tempo = ((Tempo + 1) < 600) ? (Tempo + 1) : 600;	break;
					case 21:	Tempo = ((Tempo + 5) < 600) ? (Tempo + 5) : 600;	break;
					case 22:	Tempo = ((Tempo + 10) < 600) ? (Tempo + 10) : 600;	break;
					}

					if(Blinking)
						SendMessage(hWnd, WM_TIMER, 1, (LPARAM)NULL);

					if(!SendMessage(hWnd, SB_ISSIMPLE, 0, 0))
					{
						TCHAR ts[32];

						// Do not use float..._ftoi referenced by compiler...there is no C-Runtime...
						wsprintf(ts, "\t%u(%ums)", Tempo, (1000*1000) / ((Tempo*1000) / 60));

						SendMessage(hWnd, SB_SETTEXT, 1 | SBT_POPOUT, (LPARAM)ts);

						SendMessage(GetParent(hWnd), WM_APP, 0, (1000*1000) / ((Tempo*1000) / 60));
					}

					if(Metronoming)
					{
						KillTimer(hWnd, 0);
						SetTimer(hWnd, 0, (1000*1000) / ((Tempo*1000) / 60), NULL);
					}
				}

				if(ret == 30)
				{
					if(Metronoming)
					{
						KillTimer(hWnd, 0);
						SetTimer(hWnd, 0, (1000*1000) / ((Tempo*1000) / 60), NULL);
					}
					SendMessage(GetParent(hWnd), WM_APP, 0, (1000*1000) / ((Tempo*1000) / 60));
				}

				if(60 <= ret)
				{
					if(Blinking)
						SendMessage(hWnd, WM_TIMER, 1, (LPARAM)NULL);

					if(!SendMessage(hWnd, SB_ISSIMPLE, 0, 0))
					{
						TCHAR ts[32];

						Tempo = ret;

						// Do not use float..._ftoi referenced by compiler...there is no C-Runtime...
						wsprintf(ts, "\t%u(%ums)", Tempo, (1000*1000) / ((Tempo*1000) / 60));

						SendMessage(hWnd, SB_SETTEXT, 1 | SBT_POPOUT, (LPARAM)ts);
			
						SendMessage(GetParent(hWnd), WM_APP, 0, (1000*1000) / ((Tempo*1000) / 60));
					}

					if(Metronoming)
					{
						KillTimer(hWnd, 0);
						SetTimer(hWnd, 0, (1000*1000) / ((Tempo*1000) / 60), NULL);
					}
				}
			}
		}
		break;
	case WM_RBUTTONDOWN:
		if(TempoMode && getClickedStatusPane(hWnd, LOWORD(lParam)) == 1)
		{
			if(Metronoming)
			{
				KillTimer(hWnd, 0);
				if(Blinking)
					SendMessage(hWnd, WM_TIMER, 1, (LPARAM)NULL);
			}
			TempoMode = FALSE;
			{
				DRAWITEMSTRUCT dis;

				dis.hwndItem = hWnd;
				dis.itemID = 256;

				SendMessage(GetParent(hWnd), WM_DRAWITEM, 0, (LPARAM)&dis);
			}
			SendMessage(GetParent(hWnd), WM_APP, 0, 0);
		}
		break;
	case WM_TIMER:
		if(wParam == 0)
		{
			if(!SendMessage(hWnd, SB_ISSIMPLE, 0, 0))
			{
				TCHAR ts[32];
				LPTSTR ods;

				SendMessage(hWnd, SB_GETTEXT, 1, (LPARAM)ts);
				ods = halloc(strbytes(ts) + 1);
				lstrcpy(ods, ts + 1);
				Blinking = TRUE;
				SendMessage(hWnd, SB_SETTEXT, 1 | SBT_OWNERDRAW, (LPARAM)ods);

				SetTimer(hWnd, 1, 50, NULL);
			}
		}
		if(wParam == 1)
		{
			if(!SendMessage(hWnd, SB_ISSIMPLE, 0, 0))
			{
				TCHAR ts[32];
				LPTSTR ods;

				ods = (LPTSTR)SendMessage(hWnd, SB_GETTEXT, 1, (LPARAM)NULL);
				ts[0] = '\t';
				lstrcpy(ts + 1, ods);
				SendMessage(hWnd, SB_SETTEXT, 1 | SBT_POPOUT, (LPARAM)ts);
				Blinking = FALSE;
				hfree(ods);

				KillTimer(hWnd, 1);
			}
		}
		break;
	case WM_DRAWITEM:
		if(((DRAWITEMSTRUCT*)lParam)->hwndItem == hStatusWnd)
		{
			RECT rc = ((DRAWITEMSTRUCT*)lParam)->rcItem;
			HDC hdc = ((DRAWITEMSTRUCT*)lParam)->hDC;
			LPCTSTR istr = (LPCTSTR)((DRAWITEMSTRUCT*)lParam)->itemData;
			SIZE s;

			SaveDC(hdc);

			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, GetSysColor(COLOR_BTNFACE));
			FillRectangle(hdc, rc.left, rc.top, rc.right, rc.bottom, GetSysColor(COLOR_BTNTEXT));
			GetTextExtentPoint32(hdc, istr, lstrlen(istr), &s);
			TextOut(hdc, rc.left + (rc.right - rc.left - s.cx) / 2, rc.top + (rc.bottom - rc.top - s.cy) / 2, istr, lstrlen(istr));

			RestoreDC(hdc, -1);
		}
		break;
	case SB_SETTEXT:
		if(TempoMode)
			if(!SendMessage(hWnd, SB_ISSIMPLE, 0, 0))
				if((!(wParam & SBT_POPOUT)) && (!(wParam & SBT_OWNERDRAW)))
					return 0;
		break;
	case SB_SIMPLE:
		if(Blinking)
			SendMessage(hWnd, WM_TIMER, 1, (LPARAM)NULL);
		break;
	}

	return CallWindowProc((WNDPROC)GetWindowLong(hWnd, GWL_USERDATA), hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK PRollWndSub(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Window-independent code.

	switch(msg)
	{
	case WM_APP:
		SetWindowLong(hWnd, GWL_USERDATA, GetWindowLong(hWnd, GWL_WNDPROC));
		return 0;
	case WM_KEYDOWN:
	case WM_KEYUP:
		if(('0' <= (int)wParam && (int)wParam <= '9')
			|| ('A' <= (int)wParam && (int)wParam <= 'Z')
			|| (int)wParam == VK_SHIFT
			|| (int)wParam == VK_ESCAPE
			|| (int)wParam == VK_BACK
			|| (int)wParam == VK_DELETE
			|| (int)wParam == VK_OEM_MINUS
			|| (int)wParam == VK_OEM_2
			|| (int)wParam == VK_LEFT
			|| (int)wParam == VK_RIGHT
			)
			SendMessage(GetParent(hWnd), msg, wParam, lParam);
		//return 0;
		break;
	}

	return CallWindowProc((WNDPROC)GetWindowLong(hWnd, GWL_USERDATA), hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK ListWndSub(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Window-independent code.

	switch(msg)
	{
	case WM_APP:
		SetWindowLong(hWnd, GWL_USERDATA, GetWindowLong(hWnd, GWL_WNDPROC));
		return 0;
	case WM_KEYDOWN:
	case WM_KEYUP:
		if(('0' <= (int)wParam && (int)wParam <= '9')
			|| ('A' <= (int)wParam && (int)wParam <= 'Z')
			|| (int)wParam == VK_DELETE
			|| (int)wParam == VK_INSERT
			|| (int)wParam == VK_SPACE
			)
			SendMessage(GetParent(hWnd), msg, wParam, lParam);
		//return 0;
		break;
	}

	return CallWindowProc((WNDPROC)GetWindowLong(hWnd, GWL_USERDATA), hWnd, msg, wParam, lParam);
}

void SaveConfig(BOOL saveAll)
{
	int i;
	TCHAR inifile[MAX_PATH];

	{
		LPTSTR last;
		TCHAR lastsix[7];
		GetModuleFileName(NULL, inifile, sizeof(inifile));
		last = (LPTSTR)getLastChar(inifile, '.');
		// TODO: if module file name length lesser than 6...
		lstrcpyn(lastsix, last - 6, 7);
		if(!lstrcmpi(lastsix, "_debug"))
			last -= 6;
		lstrcpy(last, ".ini");
	}

	for(i = 0; i < sizeof(IniEntries) / sizeof(struct tagIniEntries); i++)
	{
		TCHAR ts[32];

		if(IniEntries[i].num == 1)
		{	// Value
			BOOL found = FALSE;

			GetPrivateProfileString(IniEntries[i].section, IniEntries[i].key, "", ts, sizeof(ts), inifile);
			if(lstrlen(ts))
				found = TRUE;

			if(IniEntries[i].type == IEV_INTEGER)
			{
				if(saveAll || found || IniEntries[i].def != *IniEntries[i].val)	// If changed
				{
					wsprintf(ts, "%d", *IniEntries[i].val);
					WritePrivateProfileString(IniEntries[i].section, IniEntries[i].key, ts, inifile);
				}
			}
			if(IniEntries[i].type == IEV_STRING)
			{
				if(saveAll || found || lstrcmp((LPTSTR)IniEntries[i].def, (LPTSTR)IniEntries[i].val))
					WritePrivateProfileString(IniEntries[i].section, IniEntries[i].key, (LPTSTR)IniEntries[i].val, inifile);
			}
		}else
		{	// Array
			UINT count;

			for(count = 0; count < IniEntries[i].num; count++)
			{
				TCHAR kn[64];
				BOOL found = FALSE;

				lstrcpy(kn, IniEntries[i].key);
				wsprintf(ts, "[%u]", count);
				lstrcat(kn, ts);

				GetPrivateProfileString(IniEntries[i].section, kn, "", ts, sizeof(ts), inifile);
				if(lstrlen(ts))
					found = TRUE;

				if(IniEntries[i].type == IEV_INTEGER)
				{
					if(saveAll || found || ((int*)IniEntries[i].def)[count] != IniEntries[i].val[count])	// If changed
					{
						wsprintf(ts, "%d", IniEntries[i].val[count]);
						WritePrivateProfileString(IniEntries[i].section, kn, ts, inifile);
					}
				}
				if(IniEntries[i].type == IEV_STRING)
				{
					if(saveAll || found || lstrcmp((LPTSTR)IniEntries[i].def, (LPTSTR)IniEntries[i].val + count*IniEntries[i].bufsize))
					{
						MessageBox(NULL, "IEV_STRING Array write will bug.", "WARNING", MB_ICONERROR | MB_OK);
						WritePrivateProfileString(IniEntries[i].section, kn, (LPTSTR)(IniEntries[i].val + count*IniEntries[i].bufsize), inifile);
					}
				}
			}
		}
	}
}

void CheckProblems(SMAF* smaf)
{
	HWND hWnd = hMainWnd;	// TODO: tenuki

	{
		CHUNK cnti;

		if(findchunk4(smaf->data + 8, smaf->size, "CNTI", &cnti))
		{
			if(cnti.data[0] != 0x00)
			{
				if(MessageBox(hWnd, "Contents Class is not YAMAHA!\nFix?", "Question", MB_ICONQUESTION | MB_YESNO) == IDYES)
					cnti.data[0] = 0x00;
			}else
				MessageBox(hWnd, "Contents Class is YAMAHA.", "Note", MB_ICONASTERISK | MB_OK);

			switch(smaf->format)
			{
			case SMAFF_MA1:
			case SMAFF_MA2:
			case SMAFF_MA7:
				MessageBox(hWnd, "MA-1,2 or 7 not support now. sorry.", "Note", MB_ICONASTERISK | MB_OK);
				break;
			case SMAFF_MA3:
				if(cnti.data[1] < 0x32 || 0x33 < cnti.data[1])
				{
					if(MessageBox(hWnd, "Contents Type may be corrupt!\nFix?", "Question", MB_ICONQUESTION | MB_YESNO) == IDYES)
						cnti.data[1] = 0x32;
				}else
					MessageBox(hWnd, "Contents Type is may be OK.", "Note", MB_ICONASTERISK | MB_OK);
				break;
			case SMAFF_MA5:
				if(cnti.data[1] < 0x34 || 0x38 < cnti.data[1])
				{
					if(MessageBox(hWnd, "Contents Type may be corrupt!\nFix?", "Question", MB_ICONQUESTION | MB_YESNO) == IDYES)
						cnti.data[1] = 0x34;
				}else
					MessageBox(hWnd, "Contents Type is may be OK.", "Note", MB_ICONASTERISK | MB_OK);
				break;
			default:
				MessageBox(hWnd, "I don't know it's contents type.", "Note", MB_ICONASTERISK | MB_OK);
				break;
			}
		}else
			MessageBox(hWnd, "CNTI not found!\nCan't fix!", "Error", MB_ICONERROR | MB_OK);
	}

	{
		CHUNK mmmd;

		if(findchunk4(smaf->data, smaf->size, "MMMD", &mmmd))
		{
			if(SwapWord(mmmd.data + mmmd.size - 2) != CalcCrc(smaf->data, 8 + mmmd.size - 2))
			{
				if(MessageBox(hWnd, "CRC is incorrect!\nFix?", "Question", MB_ICONQUESTION | MB_YESNO) == IDYES)
				{
					StoreBEWord(mmmd.data + mmmd.size - 2, CalcCrc(smaf->data, 8 + mmmd.size - 2));
				}
			}else
				MessageBox(hWnd, "CRC is correct.", "Note", MB_ICONASTERISK | MB_OK);
		}else
			MessageBox(hWnd, "MMMD not found!", "Error", MB_ICONERROR | MB_OK);
	}
}

void MainWnd_Create(HWND hWnd)
{
	MAINWNDVAL* mwv = (MAINWNDVAL*)GetWindowLong(hWnd, 0);

	{
		TCHAR fn[MAX_PATH];

		GetModuleFileName(NULL, fn, sizeof(fn));
		lstrcpy((LPTSTR)basename(fn), "DefMA3_16.vm3");

		voicesmax = LoadVoices(&voices, fn);
		InitMidiMode();
	}

	{
		RECT rc;

		GetClientRect(hWnd, &rc);

		hToolWnd = CreateWindowEx(0, TOOLBARCLASSNAME, "", WS_CHILD | WS_VISIBLE | CCS_TOP | CCS_NODIVIDER | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT, 0, 0, 0, 0, hWnd, NULL, GetModuleHandle(NULL), NULL);
		SendMessage(hToolWnd, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);

		SendMessage(hToolWnd, TB_SETIMAGELIST, 0, (LPARAM)ImageList_LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_MAINTB), 16, 4, RGB(255, 0, 255), IMAGE_BITMAP, LR_DEFAULTCOLOR/*<-これいるの?*/));

		{
			TBBUTTON tbb[] = {
				{0,		ID_NEWSMAF,			TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0,	0},
				{1,		ID_OPENSMAF,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0,	0},
				{2,		ID_COMPILESMAF,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0,	0},
				{3,		ID_SAVESMAF,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0,	0},
				{4,		ID_CLOSE,			TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0,	0},
				{0,		0,					0,					TBSTYLE_SEP,	0,	0},
				{5,		ID_PLAY,			TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0,	0},
				{6,		ID_MIDIPLAY,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0,	0},
				{7,		ID_STOP,			TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0,	0},
				{8,		ID_REWIND,			TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0,	0},
				{9,		ID_MIDIRESET,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0,	0},
				{0,		0,					0,					TBSTYLE_SEP,	0,	0},
				{12,	ID_TOGGLENIAGARA,	TBSTATE_ENABLED,	TBSTYLE_CHECK,	0,	0},
				{13,	ID_TOGGLERO,		TBSTATE_ENABLED,	TBSTYLE_CHECK,	0,	0},
				{14,	ID_TOGGLENP,		TBSTATE_ENABLED,	TBSTYLE_CHECK,	0,	0},
				{0,		0,					0,					TBSTYLE_SEP,	0,	0},
				{10,	ID_DETAILWND,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0,	0},
				{0,		0,					0,					TBSTYLE_SEP,	0,	0},
				{11,	ID_HELPINDEX,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0,	0},
			};

			SendMessage(hToolWnd, TB_ADDBUTTONS, sizeof(tbb) / sizeof(TBBUTTON), (LPARAM)tbb);
		}

		hListWnd = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, "", WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER | LVS_OWNERDRAWFIXED, 0, 0, 0, 0, hWnd, NULL, GetModuleHandle(NULL), NULL);
		SendMessage(hListWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT/* | LVS_EX_FLATSB*/);
		{
			LV_COLUMN lvc;

			lvc.mask = LVCF_TEXT | LVCF_WIDTH;
			lvc.pszText = "Time";
			lvc.cx = 50;
			ListView_InsertColumn(hListWnd, 0, &lvc);

			lvc.mask = LVCF_TEXT | LVCF_WIDTH;
			lvc.pszText = "Track";
			lvc.cx = 30;
			ListView_InsertColumn(hListWnd, 1, &lvc);

			lvc.mask = LVCF_TEXT | LVCF_WIDTH;
			lvc.pszText = "Event";
			lvc.cx = 80;
			ListView_InsertColumn(hListWnd, 2, &lvc);

			lvc.mask = LVCF_TEXT | LVCF_WIDTH;
			lvc.pszText = "Data";
			lvc.cx = 80;
			ListView_InsertColumn(hListWnd, 3, &lvc);
		}
		ListWndSub(hListWnd, WM_APP, 0, 0), SetWindowLong(hListWnd, GWL_WNDPROC, (LONG)ListWndSub);

		hPRollWnd = CreateWindowEx(0, "MMFToolPRollWC", "", WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL, 0, 0, 0, 0, hWnd, NULL, GetModuleHandle(NULL), NULL);
		PRollWndSub(hPRollWnd, WM_APP, 0, 0), SetWindowLong(hPRollWnd, GWL_WNDPROC, (LONG)PRollWndSub);

		hStatusWnd = CreateWindowEx(0, STATUSCLASSNAME, "", WS_CHILD | WS_VISIBLE | CCS_BOTTOM | SBARS_SIZEGRIP, 0, 0, 0, 0, hWnd, NULL, GetModuleHandle(NULL), NULL);
		StatusWndSub(hStatusWnd, WM_APP, 0, 0), SetWindowLong(hStatusWnd, GWL_WNDPROC, (LONG)StatusWndSub);

		{
			int pw[] = {0, 0};

			SendMessage(hStatusWnd, SB_SETPARTS, sizeof(pw) / sizeof(int), (LPARAM)pw);

			//SendMessage(hStatusWnd, SB_SETTEXT, 0, (LPARAM)"Ready");
			//SendMessage(hStatusWnd, SB_SETTEXT, 1, (LPARAM)"\tEmpty");
		}
	}

	setStatus(mwv->smaf, mwv->emusounding, mwv->midisounding);

	SetSMAFMPWnd(hWnd);
	SetSMAFCallBack(SmafCB, 0);
	SetEmuMPWnd(hWnd);

	ShowWindow(hListWnd, SW_HIDE);
	ShowWindow(hPRollWnd, SW_HIDE);

	return;
}

BOOL MainWnd_ShiftOption(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MAINWNDVAL* mwv = (MAINWNDVAL*)GetWindowLong(hWnd, 0);

	switch(msg)
	{
	case WM_KEYDOWN:
		if((int)wParam == VK_SHIFT)
			if(!mwv->isinputting)
			{
				mwv->isinputting = TRUE;
				mwv->isinputfirst = TRUE;
				mwv->inputtingnum = 0;
				mwv->inputtingtype = 0;

				{
					LPTSTR str;

					str = halloc(256);
					lstrcpy(str, "Shift-Option?");
					SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETOPTIONSTRING, (LPARAM)str);
					InvalidateRect(hPRollWnd, NULL, FALSE);
				}

				return TRUE;
			}

		if(mwv->isinputting)
		{
			if('0' <= (int)wParam && (int)wParam <= '9')
			{
				if(mwv->isinputfirst)
				{
					mwv->isinputfirst = FALSE;
					mwv->inputtingnum = 0;
				}
				mwv->inputtingnum = mwv->inputtingnum * 10 + (int)wParam - '0';
				formatso(mwv->inputtingtype, mwv->inputtingnum, mwv->inputescapestr);
				InvalidateRect(hPRollWnd, NULL, FALSE);
			}
			if((int)wParam == VK_OEM_MINUS)
			{
				mwv->inputtingnum = -mwv->inputtingnum;
				formatso(mwv->inputtingtype, mwv->inputtingnum, mwv->inputescapestr);
				InvalidateRect(hPRollWnd, NULL, FALSE);
			}
			if((int)wParam == VK_BACK || (int)wParam == VK_DELETE)
			{
				if(mwv->inputtingtype == VK_ESCAPE)
				{
					if((int)wParam == VK_DELETE)
						if(lstrlen(mwv->inputescapestr))
						{
							// TODO: not lstrlen, it is strbytes!
							*(char*)(mwv->inputescapestr + lstrlen(mwv->inputescapestr) - 1) = '\0';
							mwv->isinputfirst = TRUE;

							mwv->inputtingnum = 0;
							// TODO: 下と一緒に
							if(!lstrcmp(mwv->inputescapestr, "SVEN"))	mwv->inputtingnum = SendMessage(hPRollWnd, WM_PROLL, PROLLWM_GETSHOWVISIBLEEVENTNUM, 0);
							if(mwv->smaf && !lstrcmp(mwv->inputescapestr, "GB"))	mwv->inputtingnum = mwv->smaf->gbase;
							if(mwv->smaf && !lstrcmp(mwv->inputescapestr, "DB"))	mwv->inputtingnum = mwv->smaf->dbase;
						}
					if((int)wParam == VK_BACK)
					{
						mwv->isinputfirst = FALSE;
						mwv->inputtingnum = mwv->inputtingnum / 10;
					}
				}else
				{
					mwv->isinputfirst = FALSE;
					mwv->inputtingnum = mwv->inputtingnum / 10;
				}
				formatso(mwv->inputtingtype, mwv->inputtingnum, mwv->inputescapestr);
				InvalidateRect(hPRollWnd, NULL, FALSE);
			}
			if(('A' <= (int)wParam && (int)wParam <= 'Z') || (int)wParam == VK_ESCAPE || (int)wParam == VK_OEM_2/* ? key */)
			{
				mwv->isinputfirst = TRUE;

				if(mwv->inputtingtype == VK_ESCAPE)
				{
					if('A' <= (int)wParam && (int)wParam <= 'Z')
					{
						if(lstrlen(mwv->inputescapestr) < 15)
						{
							char ts[2] = {(char)wParam, '\0'};
							lstrcat(mwv->inputescapestr, ts);

							mwv->inputtingnum = 0;
							// TODO: 上と一緒に
							if(!lstrcmp(mwv->inputescapestr, "SVEN"))
								mwv->inputtingnum = SendMessage(hPRollWnd, WM_PROLL, PROLLWM_GETSHOWVISIBLEEVENTNUM, 0);
							if(mwv->smaf && !lstrcmp(mwv->inputescapestr, "GB"))	mwv->inputtingnum = mwv->smaf->gbase;
							if(mwv->smaf && !lstrcmp(mwv->inputescapestr, "DB"))	mwv->inputtingnum = mwv->smaf->dbase;
						}
					}
					if((int)wParam == VK_ESCAPE)
						mwv->inputtingtype = 0;
				}else
				{
					mwv->inputtingtype = (char)wParam;
					switch(mwv->inputtingtype)
					{
					case 'R':	mwv->inputtingnum = MainOption.RefreshRate;										break;
					case 'D':	mwv->inputtingnum = MainOption.DrawFunc;											break;
					case 'H':	mwv->inputtingnum = SendMessage(hPRollWnd, WM_PROLL, PROLLWM_GETROLLHEIGHT, 0);	break;
					case 'T':	mwv->inputtingnum = EmuGetTempo();												break;
					case 'K':	mwv->inputtingnum = EmuGetKeypitch();											break;
					case 'W':	mwv->inputtingnum = SendMessage(hPRollWnd, WM_PROLL, PROLLWM_GETROLLWIDTH, 0);	break;
					case 'F':	mwv->inputtingnum = MainOption.FollowPlay;										break;
					case 'S':	mwv->inputtingnum = -MainOption.FollowSpace;										break;
					case 'G':
						if(mwv->smaf)
							mwv->inputtingnum = PlayingPos;
						break;
					case 'V':	mwv->inputtingnum = EmuGetVolume();												break;
					case 'M':	mwv->inputtingnum = PRollGetScroll(hPRollWnd);		break;
					case 'O':	mwv->inputtingnum = MainOption.PlayVisibleOnly;									break;
					case 'L':	mwv->inputtingnum = SendMessage(hPRollWnd, WM_PROLL, PROLLWM_GETSCROLLMAG, 0);	break;
					case 'A':	mwv->inputtingnum = MainOption.PlayPosAlignByMagScale;							break;
					case VK_ESCAPE:
						lstrcpy(mwv->inputescapestr, "");
						mwv->inputtingnum = 0;
						break;
					}
				}
				formatso(mwv->inputtingtype, mwv->inputtingnum, mwv->inputescapestr);
				InvalidateRect(hPRollWnd, NULL, FALSE);
			}

			return TRUE;
		}
		break;
	case WM_KEYUP:
		if((int)wParam == VK_SHIFT)
			if(mwv->isinputting)
			{
				mwv->isinputting = FALSE;
				switch(mwv->inputtingtype)
				{
				case VK_ESCAPE:
					if(!lstrcmp(mwv->inputescapestr, "SVEN"))
					{
						SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETSHOWVISIBLEEVENTNUM, mwv->inputtingnum);
					}
					if(!lstrcmp(mwv->inputescapestr, "GB") || !lstrcmp(mwv->inputescapestr, "DB"))
					{
						if(mwv->smaf && 0 < mwv->inputtingnum && mwv->inputtingnum <= 50)	// TODO: 本当は1,2,4,5,10,20,40,50でチェックしろ
						{
							EVENT* e, *pree;

							if(!lstrcmp(mwv->inputescapestr, "GB"))
								mwv->smaf->gbase = mwv->inputtingnum;
							if(!lstrcmp(mwv->inputescapestr, "DB"))
								mwv->smaf->dbase = mwv->inputtingnum;

							for(e = mwv->smaf->events, pree = NULL; e; pree = e, e = e->next)
							{
								e->time = e->time / mwv->smaf->dbase * mwv->smaf->dbase;
								if(pree)
									e->duration = e->time - pree->time;
								if((e->status & 0xF0) == 0x80 || (e->status & 0xF0) == 0x90)
								{
									UINT ofs = ((e->status & 0xF0) == 0x90) ? 2 : 1;
									SetVVal(e->data + ofs, GetVVal(e->data + ofs) / mwv->smaf->gbase * mwv->smaf->gbase);
								}
							}
						}
					}
					break;
				case 'R':
					if(0 < mwv->inputtingnum)
						MainOption.RefreshRate = mwv->inputtingnum;
					if(mwv->emusounding || mwv->midisounding)
					{
						KillTimer(hWnd, 0);
						SetTimer(hWnd, 0, MainOption.RefreshRate, NULL);
					}
					break;
				case 'D':
					if(0 <= mwv->inputtingnum && mwv->inputtingnum <= 2)
						MainOption.DrawFunc = mwv->inputtingnum;
					break;
				case 'H':
					if(0 < mwv->inputtingnum)
					{
						MainOption.PRollOption.RollHeight = mwv->inputtingnum;
						SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETROLLHEIGHT, mwv->inputtingnum);
					}
					break;
				case 'T':
					if(70 <= mwv->inputtingnum && mwv->inputtingnum <= 130)
					{
						// テンポ(%) between 70 to 130.
						MainOption.EmuOption.Tempo = mwv->inputtingnum;
						EmuSetTempo(mwv->inputtingnum);
					}
					break;
				case 'K':
					if(-12 <= mwv->inputtingnum && mwv->inputtingnum <= 12)
					{
						MainOption.EmuOption.Keypitch = mwv->inputtingnum;
						EmuSetKeypitch(mwv->inputtingnum);
					}
					break;
				case 'W':
					if(0 < mwv->inputtingnum)
					{
						MainOption.PRollOption.RollWidth = mwv->inputtingnum;
						SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETROLLWIDTH, mwv->inputtingnum);
					}
					break;
				case 'F':
					if(0 <= mwv->inputtingnum && mwv->inputtingnum <= 3)
						MainOption.FollowPlay = mwv->inputtingnum;
					break;
				case 'S':
					MainOption.FollowSpace = -mwv->inputtingnum;
					break;
				case 'G':
					if(mwv->smaf && 0 <= mwv->inputtingnum)
					{
						seek(mwv->smaf, mwv->inputtingnum, mwv->emusounding || mwv->midisounding, TRUE);

						settitle(hWnd, mwv->smaf, mwv->emusounding, mwv->midisounding);
					}
					break;
				case 'V':
					if(0 <= mwv->inputtingnum && mwv->inputtingnum <= 127)
					{
						// 音量 between 0 to 127
						MainOption.EmuOption.Volume = mwv->inputtingnum;
						EmuSetVolume(mwv->inputtingnum);
					}
					break;
				case 'M':
					if(0 <= mwv->inputtingnum)
						SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SCROLLTO, mwv->inputtingnum);
					break;
				case 'O':
					if(0 <= mwv->inputtingnum && mwv->inputtingnum <= 1)
					{
						MainOption.PlayVisibleOnly = mwv->inputtingnum;
						if(MainOption.PlayVisibleOnly)
							EmuMIDISetSoloCh(SendMessage(hPRollWnd, WM_PROLL, PROLLWM_GETVISIBLETRACK, 0));
						else
							EmuMIDISetSoloCh(-1);
					}
					break;
				case 'L':
					if(0 <= mwv->inputtingnum)
					{
						MainOption.PRollOption.ScrollMag = mwv->inputtingnum;
						SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETSCROLLMAG, mwv->inputtingnum);
					}
					break;
				case 'A':
					if(0 <= mwv->inputtingnum && mwv->inputtingnum <= 1)
						MainOption.PlayPosAlignByMagScale = mwv->inputtingnum;
					break;
				}
				hfree((void*)SendMessage(hPRollWnd, WM_PROLL, PROLLWM_GETOPTIONSTRING, 0));
				SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETOPTIONSTRING, 0);
				InvalidateRect(hPRollWnd, NULL, FALSE);
			}
		break;
	}

	return FALSE;
}

void MainWnd_Command(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MAINWNDVAL* mwv = (MAINWNDVAL*)GetWindowLong(hWnd, 0);

	switch(HIWORD(wParam))
	{
	case 0:	// Command
	case 1:	// Accelerator
		switch(LOWORD(wParam))
		{
		case ID_NEWSMAF:
			if(mwv->smaf)
				SendMessage(hWnd, WM_COMMAND, ID_CLOSE, 0);
			if(!(mwv->smaf = CreateSmaf(MainOption.DefaultDurationBase, MainOption.DefaultGateBase)))
				MessageBox(hWnd, "CreateSmaf failed", "Error", MB_ICONERROR | MB_OK);
			lstrcpy(mwv->smaf->name, "<untitled>");

			// insert master volume set
			{
				EVENT* e;

				e = halloc(sizeof(EVENT));
				e->time = 0;
				e->duration = 0;
				e->status = 0xF0;
				e->size = 8;
				e->data = halloc(e->size);
				e->data[0] = 0x07;
				e->data[1] = 0x43;
				e->data[2] = 0x79;
				e->data[3] = 0x06;
				e->data[4] = 0x7F;
				e->data[5] = 0x00;
				e->data[6] = 127;	// vol
				e->data[7] = 0xF7;

				mwv->smaf->start = InsertEvent(mwv->smaf, e);
			}

			//SetScrollPos(hWnd, SB_HORZ, 0, TRUE);
			//SetScrollPos(hWnd, SB_VERT, 0, TRUE);

			SendMessage(hDetailWnd, WM_USER, 0, (LPARAM)mwv->smaf);
			SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETSMAF, (LPARAM)mwv->smaf);
			SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SCROLLTO, 0);

			RefreshEventList(mwv->smaf, SendMessage(hPRollWnd, WM_PROLL, PROLLWM_GETVISIBLETRACK, 0));
			settitle(hWnd, mwv->smaf, mwv->emusounding, mwv->midisounding);
			setStatus(mwv->smaf, mwv->emusounding, mwv->midisounding);

			ShowWindow(hListWnd, SW_SHOW);
			ShowWindow(hPRollWnd, SW_SHOW);
			InvalidateRect(hWnd, NULL, TRUE);

			break;
		case ID_OPENSMAF:
			{
				OPENFILENAME ofn = {0};
				TCHAR fname[MAX_PATH];

				ofn.lStructSize = sizeof(ofn);
				ofn.Flags = OFN_FILEMUSTEXIST;
				ofn.hInstance = GetModuleHandle(NULL);
				ofn.hwndOwner = hWnd;
				ofn.lpstrTitle = "Open SMAF";
				fname[0] = '\0';
				ofn.lpstrFile = fname;
				ofn.nMaxFile = sizeof(fname);
				ofn.lpstrFilter = "SMAF file(*.mmf)\0*.mmf\0SMAF(Motion?) file(*.mqf)\0*.mqf\0";

				if(GetOpenFileName(&ofn))
					OpenSMAFCommand(hWnd, &mwv->smaf, fname);
			}
			break;
		case ID_RELOAD:
			if(mwv->smaf)
			{
				TCHAR fn[MAX_PATH];
				lstrcpy(fn, mwv->smaf->path);
				OpenSMAFCommand(hWnd, &mwv->smaf, fn);
			}
			break;
		case ID_CLOSE:
			if(mwv->smaf)
			{
				if(mwv->emusounding)
					SendMessage(hWnd, WM_COMMAND, ID_STOP, 0);
				if(mwv->midisounding)
					SendMessage(hWnd, WM_COMMAND, ID_STOP, 0);

				// go 0ms
				SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETCURRENTPOS, 0);

				mwv->smaf = FreeSmaf(mwv->smaf);

				SendMessage(hDetailWnd, WM_USER, 0, (LPARAM)mwv->smaf);
				SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETSMAF, (LPARAM)mwv->smaf);

				RefreshEventList(mwv->smaf, SendMessage(hPRollWnd, WM_PROLL, PROLLWM_GETVISIBLETRACK, 0));
				settitle(hWnd, mwv->smaf, mwv->emusounding, mwv->midisounding);
				setStatus(mwv->smaf, mwv->emusounding, mwv->midisounding);

				ShowWindow(hListWnd, SW_HIDE);
				ShowWindow(hPRollWnd, SW_HIDE);
				InvalidateRect(hWnd, NULL, TRUE);
			}
			break;
		case ID_COMPILESMAF:
			if(mwv->smaf)
			{
				if(!doCompile(mwv->smaf, mwv->emusounding, mwv->midisounding))
					MessageBox(hWnd, "SMAF compile failed", "CompileSMAF Error", MB_ICONERROR | MB_OK);
				else
				{
					RefreshEventList(mwv->smaf, SendMessage(hPRollWnd, WM_PROLL, PROLLWM_GETVISIBLETRACK, 0));
					MessageBox(hWnd, "SMAF successfully compiled.\nTest with Emuplay or save SMAF.", "CompileSMAF", MB_ICONASTERISK | MB_OK);
				}
			}else
				MessageBox(hWnd, "SMAF not opened", "CompileSMAF Error", MB_ICONERROR | MB_OK);
			break;
		case ID_SAVESMAF:
			if(mwv->smaf)
			{
				if(!mwv->smaf->data)
				{
					if(MessageBox(hWnd, "SMAF data is empty(It means not compiled.)\nCompile now?\n\nYes: Compile, and saveas.\nNo: Abort saving process.", "Question", MB_ICONQUESTION | MB_YESNO) == IDYES)
						if(!doCompile(mwv->smaf, mwv->emusounding, mwv->midisounding))
							MessageBox(hWnd, "SMAF compile failed!\nAbort save.", "CompileSMAF before save Error", MB_ICONERROR | MB_OK);
				}
				if(mwv->smaf->data)
				{
					OPENFILENAME ofn = {0};
					TCHAR fname[MAX_PATH];

					ofn.lStructSize = sizeof(ofn);
					ofn.Flags = OFN_FILEMUSTEXIST;
					ofn.hInstance = GetModuleHandle(NULL);
					ofn.hwndOwner = hWnd;
					ofn.lpstrTitle = "Compile and Save SMAF: Don't forget compile before save!";
					fname[0] = '\0';
					ofn.lpstrFile = fname;
					ofn.nMaxFile = sizeof(fname);
					ofn.lpstrFilter = "SMAF file(*.mmf)\0*.mmf\0";

					if(GetSaveFileName(&ofn))
					{
						HANDLE hf;

						if((hf = CreateFile(fname, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL)) == INVALID_HANDLE_VALUE)
							MessageBox(hWnd, "Can't open file to write", "Error", MB_ICONERROR | MB_OK);
						else
						{
							DWORD wr;

							WriteFile(hf, mwv->smaf->data, mwv->smaf->size, &wr, NULL);
							CloseHandle(hf);

							lstrcpy(mwv->smaf->name, basename(fname));

							settitle(hWnd, mwv->smaf, mwv->emusounding, mwv->midisounding);
						}
					}
				}
			}else
				MessageBox(hWnd, "SMAF not opened", "Error", MB_ICONERROR | MB_OK);
			break;
		case ID_EXIT:
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		case ID_HELPINDEX:
			MessageBox(hWnd, "There are no help.", "sorry", MB_ICONERROR | MB_OK);
			break;
		case ID_ABOUT:
			DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutDlgWndProc, 0);
			break;
		case ID_PLAY:
			if(mwv->smaf && !mwv->emusounding && !mwv->midisounding)
			{
				UINT startfrom = PlayingPos;
				UINT beginpoint = 0;

				SetWindowText(hWnd, APPNAME" <Starting play...>");

				if(mwv->smaf->start)
				{
					beginpoint = mwv->smaf->start->time - mwv->smaf->start->duration;
					if(startfrom < beginpoint)
						startfrom = beginpoint;
				}

				SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETREADONLY, TRUE);

				EmuSMAFPlay(mwv->smaf, startfrom - beginpoint);
				mwv->emusounding = TRUE;
				SetTimer(hWnd, 0, MainOption.RefreshRate, NULL);
				SetTimer(hWnd, 1, 10, NULL);

				setStatus(mwv->smaf, mwv->emusounding, mwv->midisounding);
			}
			break;
		case ID_MIDIPLAY:
			if(mwv->smaf && !mwv->midisounding && !mwv->emusounding)
			{
				UINT startfrom = PlayingPos;
				UINT beginpoint = 0;

				SetWindowText(hWnd, APPNAME" <Starting play...>");

				if(mwv->smaf->start)
				{
					beginpoint = mwv->smaf->start->time - mwv->smaf->start->duration;
					if(startfrom < beginpoint)
						startfrom = beginpoint;
				}

				SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETREADONLY, TRUE);

				EmuMIDIPlay(mwv->smaf, startfrom - beginpoint);
				mwv->midisounding = TRUE;
				SetTimer(hWnd, 0, MainOption.RefreshRate, NULL);
				SetTimer(hWnd, 1, 10, NULL);

				setStatus(mwv->smaf, mwv->emusounding, mwv->midisounding);
			}
			break;
		case ID_STOP:
			if(mwv->smaf && mwv->emusounding)
			{
				SetWindowText(hWnd, APPNAME" <Stopping play...>");

				EmuSMAFStop();
				InitMidiMode();
				mwv->emusounding = FALSE;
				KillTimer(hWnd, 0);
				KillTimer(hWnd, 1);

				if(!(SendMessage(hToolWnd, TB_GETSTATE, (WPARAM)ID_TOGGLERO, 0) & TBSTATE_CHECKED))
					SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETREADONLY, FALSE);

				settitle(hWnd, mwv->smaf, mwv->emusounding, mwv->midisounding);
				setStatus(mwv->smaf, mwv->emusounding, mwv->midisounding);

				InvalidateRect(hWnd, NULL, FALSE);
			}
			if(mwv->smaf && mwv->midisounding)
			{
				SetWindowText(hWnd, APPNAME" <Stopping play...>");

				EmuMIDIStop();
				mwv->midisounding = FALSE;
				KillTimer(hWnd, 0);
				KillTimer(hWnd, 1);

				if(!(SendMessage(hToolWnd, TB_GETSTATE, (WPARAM)ID_TOGGLERO, 0) & TBSTATE_CHECKED))
					SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETREADONLY, FALSE);

				settitle(hWnd, mwv->smaf, mwv->emusounding, mwv->midisounding);
				setStatus(mwv->smaf, mwv->emusounding, mwv->midisounding);

				InvalidateRect(hWnd, NULL, FALSE);
			}
			break;
		case ID_REWIND:
			if(mwv->smaf)
			{
				if(mwv->smaf->start)
					seek(mwv->smaf, mwv->smaf->start->time - mwv->smaf->start->duration, mwv->emusounding || mwv->midisounding, FALSE);
				else
					seek(mwv->smaf, 0, mwv->emusounding || mwv->midisounding, FALSE);

				settitle(hWnd, mwv->smaf, mwv->emusounding, mwv->midisounding);
			}
			break;
		case ID_MIDIRESET:
			InitMidiMode();

			break;
		case ID_DETAILWND:
			ShowWindow(hDetailWnd, SW_SHOW);
			SetForegroundWindow(hDetailWnd);
			break;
		case ID_VTRACKALL:	setvtrack(-1);	break;
		case ID_VTRACK1:	setvtrack(0);	break;
		case ID_VTRACK2:	setvtrack(1);	break;
		case ID_VTRACK3:	setvtrack(2);	break;
		case ID_VTRACK4:	setvtrack(3);	break;
		case ID_VTRACK5:	setvtrack(4);	break;
		case ID_VTRACK6:	setvtrack(5);	break;
		case ID_VTRACK7:	setvtrack(6);	break;
		case ID_VTRACK8:	setvtrack(7);	break;
		case ID_VTRACK9:	setvtrack(8);	break;
		case ID_VTRACK10:	setvtrack(9);	break;
		case ID_VTRACK11:	setvtrack(10);	break;
		case ID_VTRACK12:	setvtrack(11);	break;
		case ID_VTRACK13:	setvtrack(12);	break;
		case ID_VTRACK14:	setvtrack(13);	break;
		case ID_VTRACK15:	setvtrack(14);	break;
		case ID_VTRACK16:	setvtrack(15);	break;
		case ID_TOGGLENIAGARA:
			if(SendMessage(hToolWnd, TB_GETSTATE, (WPARAM)ID_TOGGLENIAGARA, 0) & TBSTATE_CHECKED)
			{
				MainOption.PRollOption.NiagaraTouch = TRUE;
				SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETNIAGARATOUCH, (LPARAM)TRUE);
			}else
			{
				MainOption.PRollOption.NiagaraTouch = FALSE;
				SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETNIAGARATOUCH, (LPARAM)FALSE);
			}
			setStatus(mwv->smaf, mwv->emusounding, mwv->midisounding);
			break;
		case ID_TOGGLERO:
			if(!mwv->emusounding && !mwv->midisounding)
				if(SendMessage(hToolWnd, TB_GETSTATE, (WPARAM)ID_TOGGLERO, 0) & TBSTATE_CHECKED)
				{
					MainOption.PRollOption.ReadOnly = TRUE;
					SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETREADONLY, (LPARAM)TRUE);
				}else
				{
					MainOption.PRollOption.ReadOnly = FALSE;
					SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETREADONLY, (LPARAM)FALSE);
				}
			break;
		case ID_CHECKCRC:	// 隠し予定
			if(mwv->smaf && mwv->smaf->data)
				CheckProblems(mwv->smaf);
			break;
		case ID_TOGGLENP:
			MainOption.DontPreview = !MainOption.DontPreview;
			setStatus(mwv->smaf, mwv->emusounding, mwv->midisounding);
			break;
		case ID_SAVESETTINGS:
			SaveConfig((GetAsyncKeyState(VK_SHIFT) & 0x8000) ? TRUE : FALSE);
			break;
		}
		break;
	}

	return;
}

void MainWnd_PRollNotify(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MAINWNDVAL* mwv = (MAINWNDVAL*)GetWindowLong(hWnd, 0);

	switch(LOWORD(wParam))
	{
	case PN_CURRENTPOSCHANGED:
		if(mwv->smaf)
		{
			UINT pos = lParam;
			if(mwv->smaf->start && pos < (mwv->smaf->start->time - mwv->smaf->start->duration))
				pos = mwv->smaf->start->time - mwv->smaf->start->duration;
			seek(mwv->smaf, pos, mwv->emusounding || mwv->midisounding, FALSE);

			settitle(hWnd, mwv->smaf, mwv->emusounding, mwv->midisounding);
		}
		break;
	case PN_CLICKED:
		if(MainOption.DontPreview)
			break;

		if(mwv->emusounding)
			break;

		// MasterVolume
		EmuMIDISetVolume();

		{
			int vt = SendMessage(hPRollWnd, WM_PROLL, PROLLWM_GETVISIBLETRACK, 0);
			UINT tr = (vt == -1) ? 0 : vt;
			int bm = -1, bl = -1, pc = -1;

			// TODO: 位置(lParam)による音色調整(PCとかCCで変わるから)

			if(mwv->smaf && SendMessage(hPRollWnd, WM_PROLL, PROLLWM_GETVISIBLETRACK, 0) != -1)
			{
				EVENT* ev;

				for(ev = mwv->smaf->events; ev; ev = ev->next)
				{
					if((ev->status & 0x0F) == (BYTE)tr)
					{
						if((ev->status & 0xF0) == 0xB0)
						{
							if(ev->data[0] == 0x00)
								bm = ev->data[1];
							if(ev->data[0] == 0x20)
								bl = ev->data[1];
						}
						if((ev->status & 0xF0) == 0xC0)
							pc = ev->data[0];

						if(bm != -1 && bl != -1 && pc != -1)
							break;
					}
				}

				if(bm != -1 && bl != -1 && pc != -1)
					for(ev = mwv->smaf->events; ev; ev = ev->next)
					{
						if(ev->status == 0xF0)
						{
							BYTE cmd[] = {0x43, 0x79, 0x06, 0x7F, 0x01, bm, bl, pc};
							if(!memcomp(ev->data + 1, cmd, sizeof(cmd)))
							{
								DoEvent(ev, FALSE);
								break;
							}

							cmd[2] = 0x07;
							if(!memcomp(ev->data + 1, cmd, sizeof(cmd)))
							{
								DoEvent(ev, FALSE);
								break;
							}
						}
					}
			}

			if(bm == -1)
				if(GetAsyncKeyState(VK_SHIFT) & 0x8000)
					bm = 125;	// drum
				else
					bm = 124;
			if(bl == -1)
				bl = 0;
			if(pc == -1)
				pc = mwv->pch;

			// CC BankSelect
			{
				BYTE cmd[] = {0xB0 | tr, 0x00, bm};
				if(EmuSetMidiMsg(cmd, 3))
				{
					MessageBox(hWnd, "SetMidiMsg5 failed", "Error", MB_ICONERROR | MB_OK);
					break;
				}
			}
			{
				BYTE cmd[] = {0xB0 | tr, 0x20, bl};
				if(EmuSetMidiMsg(cmd, 3))
				{
					MessageBox(hWnd, "SetMidiMsg6 failed", "Error", MB_ICONERROR | MB_OK);
					break;
				}
			}
			// PC
			{
				BYTE cmd[] = {0xC0 | tr, pc};
				if(EmuSetMidiMsg(cmd, 2))
				{
					MessageBox(hWnd, "SetMidiMsg7 failed", "Error", MB_ICONERROR | MB_OK);
					break;
				}
			}
			// NoteON
			{
				BYTE cmd[] = {0x90 | tr, (BYTE)HIWORD(wParam), 0x7F};
				mwv->sndch = tr;
				if(EmuSetMidiMsg(cmd, 3))
				{
					MessageBox(hWnd, "SetMidiMsg8[sounding note] failed", "Error", MB_ICONERROR | MB_OK);
					break;
				}
			}

			mwv->sndy = HIWORD(wParam);
		}

		break;
	case PN_RELEASED:
		if(mwv->emusounding)
			break;

		if(mwv->sndy == -1)
			break;

		// NoteOFF
		{
			BYTE cmd[] = {0x90 | mwv->sndch, mwv->sndy, 0x00};
			if(EmuSetMidiMsg(cmd, sizeof(cmd)))
			{
				MessageBox(hWnd, "SetMidiMsg8[sounding] failed", "Error", MB_ICONERROR | MB_OK);
				break;
			}
		}

		break;
	case PN_WHEELCLICKED:
		{
			HMENU hm[9] = {0};
			POINT pt;
			int gr;
			int ret;

			hm[0] = CreatePopupMenu();

			for(gr = 0; gr < 8; gr++)
			{
				int i;

				hm[gr + 1] = CreatePopupMenu();

				for(i = 0; i < 16; i++)
				{
					char ts[32];

					wsprintf(ts, "%d: %s", gr * 16 + i, voices[gr * 16 + i].name);
					InsertMenu(hm[gr + 1], i, MF_BYPOSITION | MF_STRING | ((mwv->pch == gr * 16 + i) ? MF_CHECKED : 0), gr * 16 + i + 100, ts);
				}

				{
					char ts[32];
					wsprintf(ts, "Inst# %d - %d", gr * 16, gr * 16 + 15);
					InsertMenu(hm[0], gr, MF_BYPOSITION | MF_POPUP | ((gr * 16 <= mwv->pch && mwv->pch <= gr * 16 + 15) ? MF_CHECKED : 0), (UINT)hm[gr + 1], ts);
				}
			}

			GetCursorPos(&pt);
			ret = (int)TrackPopupMenu(hm[0], TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, NULL);

			for(gr = 0; gr < 8; gr++)
				DestroyMenu(hm[gr + 1]);

			DestroyMenu(hm[0]);

			if(ret >= 100)
				mwv->pch = ret - 100;
		}
		break;
	case PN_EVENTCREATED:
	//case PN_EVENTDELETING:	// TODO: only delete one line.
	case PN_EVENTDELETED:	// TODO: obsolete
		RefreshEventList(mwv->smaf, SendMessage(hPRollWnd, WM_PROLL, PROLLWM_GETVISIBLETRACK, 0));
		break;
	case PN_EVENTMODIFYING:
	case PN_EVENTMODIFIED:	// will obsolete...?
		switch(HIWORD(wParam))
		{
		case PNEM_MOVE:
			{
				int nxtid, id;
				LV_FINDINFO lvfi;

				lvfi.flags = LVFI_PARAM;

				lvfi.lParam = lParam;
				id = ListView_FindItem(hListWnd, -1, &lvfi);

				if(id != -1)
				{
					ListView_DeleteItem(hListWnd, id);

					lvfi.lParam = (LPARAM)((EVENT*)lParam)->next;
					nxtid = ListView_FindItem(hListWnd, -1, &lvfi);
					// TODO: TRACK ONLY VIEW failed!!
					if(nxtid != -1)
					{
						LV_ITEM lvi;

						lvi.mask = LVIF_PARAM;
						lvi.iItem = nxtid;
						lvi.iSubItem = 0;
						lvi.lParam = lParam;
						ListView_InsertItem(hListWnd, &lvi);

						setEventListItem(lvi.iItem, (EVENT*)lParam);

						ListView_SetItemState(hListWnd, lvi.iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
						ListView_EnsureVisible(hListWnd, lvi.iItem, FALSE);
					}else
						RefreshEventList(mwv->smaf, SendMessage(hPRollWnd, WM_PROLL, PROLLWM_GETVISIBLETRACK, 0));
				}else
					RefreshEventList(mwv->smaf, SendMessage(hPRollWnd, WM_PROLL, PROLLWM_GETVISIBLETRACK, 0));
			}
			break;
			break;
		case PNEM_SIZE:
			{
				int id;
				LV_FINDINFO lvfi;

				lvfi.flags = LVFI_PARAM;
				lvfi.lParam = lParam;

				if((id = ListView_FindItem(hListWnd, -1, &lvfi)) != -1)
				{
					setEventListItem(id, (EVENT*)lParam);
				}
			}
			break;
		}
		break;
	case PN_VTCHANGED:
		RefreshEventList(mwv->smaf, (int)lParam);
		break;
	case PN_EVENTHILIGHTED:
		{
			/*
			EVENT* e = NULL;
			UINT i;

			for(i = 0, e = mwv->smaf->events; e; i++, e = e->next)
				// LV_SIS' Semi-colon no need.
				if(e == (EVENT*)lParam)
					break;
			*/

			if((EVENT*)lParam != mwv->selectedevent)
			{
				mwv->selectedevent = (EVENT*)lParam;
				//OutputDebugString("PR");

				ListView_SetItemState(hListWnd, -1, 0, LVIS_FOCUSED | LVIS_SELECTED);

				{
					int i;
					LV_FINDINFO lvfi;

					lvfi.flags = LVFI_PARAM;
					lvfi.lParam = lParam;

					if((i = ListView_FindItem(hListWnd, -1, &lvfi)) != -1)
					{
						ListView_SetItemState(hListWnd, i, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
						ListView_EnsureVisible(hListWnd, i, FALSE);
					}
				}
			}
		}
		break;
	case PN_EVENTCONTEXT:
		{
			HMENU hm;
			POINT p;
			UINT ret;

			SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETHILIGHTEVENT, lParam);

			hm = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_PIANOROLL));

			{
				HMENU hpopup;
				UINT i, max;

				hpopup = CreatePopupMenu();
				max = GetMenuItemCount(hm);
				for(i = 0; i < max; i++)
				{
					MENUITEMINFO mii;
					TCHAR ts[256];

					mii.cbSize = sizeof(mii);
					mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
					mii.dwTypeData = ts;
					mii.cch = sizeof(ts) - 2;

					GetMenuItemInfo(hm, i, TRUE, &mii);
					InsertMenuItem(hpopup, i, TRUE, &mii);
				}

				DestroyMenu(hm);
				hm = hpopup;
			}

			GetCursorPos(&p);
			ret = (UINT)TrackPopupMenu(hm, TPM_RETURNCMD, p.x, p.y, 0, hWnd, NULL);
			DestroyMenu(hm);

			switch(ret)
			{
			case ID_PRSCROLLTO:
				SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SCROLLTO, ((EVENT*)lParam)->time);
				break;
			case ID_PRPLAYEMUHERE:
				PlayingPos = ((EVENT*)lParam)->time;
				SendMessage(hWnd, WM_COMMAND, ID_PLAY, (LPARAM)NULL);
				break;
			case ID_PRPLAYMIDIHERE:
				PlayingPos = ((EVENT*)lParam)->time;
				SendMessage(hWnd, WM_COMMAND, ID_MIDIPLAY, (LPARAM)NULL);
				break;
			case ID_PRONLYTRACK:
				if( (((EVENT*)lParam)->status & 0xF0) == 0x80 || (((EVENT*)lParam)->status & 0xF0) == 0x90 )
				{
					setvtrack(((EVENT*)lParam)->status & 0x0F);
				}
				break;
			case ID_PRSETSTART:
				mwv->smaf->start = (EVENT*)lParam;
				if(PlayingPos < mwv->smaf->start->time)
					PlayingPos = mwv->smaf->start->time;
				InvalidateRect(hPRollWnd, NULL, FALSE);
				settitle(hWnd, mwv->smaf, mwv->emusounding, mwv->midisounding);
				break;
			case ID_PRSETSTOP:
				mwv->smaf->stop = (EVENT*)lParam;
				if(mwv->smaf->stop->time < PlayingPos)
					PlayingPos = mwv->smaf->stop->time;
				InvalidateRect(hPRollWnd, NULL, FALSE);
				settitle(hWnd, mwv->smaf, mwv->emusounding, mwv->midisounding);
				break;
			case ID_PRDELETE:
				if(!SendMessage(hPRollWnd, WM_PROLL, PROLLWM_GETREADONLY, 0))
				{
					SendMessage(hWnd, WM_PROLLNOTIFY, PN_EVENTDELETING, lParam);

					CutoutEvent(mwv->smaf, (EVENT*)lParam);
					hfree(((EVENT*)lParam)->data);
					hfree((EVENT*)lParam);
					SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETHILIGHTEVENT, (LPARAM)NULL);

					// TODO: obsolete?
					SendMessage(hWnd, WM_PROLLNOTIFY, PN_EVENTDELETED, lParam);
				}
				break;
			}
		}
		break;
	}

	return;
}

// TODO: MainWndProc is too big, divide...
//  but, where is good to place "smaf"...?
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MAINWNDVAL* mwv = (MAINWNDVAL*)GetWindowLong(hWnd, 0);

	switch(msg)
	{
	case WM_CREATE:
		{
			MAINWNDVAL* mwv;

			if(!(mwv = halloc(sizeof(MAINWNDVAL))))
			{
				MessageBox(hWnd, "Allocate WindowVariable FAILED!!", "Fatal Error", MB_ICONERROR | MB_OK);
				return -1;
			}

			// Initialize variables
			mwv->emusounding = FALSE;
			mwv->midisounding = FALSE;
			mwv->smaf = NULL;
			mwv->isinputting = FALSE;
			mwv->pch = 0;
			mwv->selectedevent = NULL;

			SetWindowLong(hWnd, 0, (LONG)mwv);

			// TODO: add error check!
			MainWnd_Create(hWnd);
		}
		break;
	case WM_MOUSEWHEEL:
		SendMessage(hPRollWnd, msg, wParam, lParam);	// bypass to PianoRoll
		break;
	case WM_KEYDOWN:
		if(!MainWnd_ShiftOption(hWnd, msg, wParam, lParam))
		{
			// No accel keys pressed?
			if(!(GetAsyncKeyState(VK_SHIFT) & 0x8000) && !(GetAsyncKeyState(VK_CONTROL) & 0x8000) && !(GetAsyncKeyState(VK_MENU) & 0x8000))
			{
				switch((int)wParam)
				{
				case '0':	setvtrack(-1);	break;

				case '1':	setvtrack(0);	break;
				case '2':	setvtrack(1);	break;
				case '3':	setvtrack(2);	break;
				case '4':	setvtrack(3);	break;
				case '5':	setvtrack(4);	break;
				case '6':	setvtrack(5);	break;
				case '7':	setvtrack(6);	break;
				case '8':	setvtrack(7);	break;
				case 'Q':	setvtrack(8);	break;
				case 'W':	setvtrack(9);	break;
				case 'E':	setvtrack(10);	break;
				case 'R':	setvtrack(11);	break;
				case 'T':	setvtrack(12);	break;
				case 'Y':	setvtrack(13);	break;
				case 'U':	setvtrack(14);	break;
				case 'I':	setvtrack(15);	break;
				}
				if(MainOption.PlayVisibleOnly)
					EmuMIDISetSoloCh(SendMessage(hPRollWnd, WM_PROLL, PROLLWM_GETVISIBLETRACK, 0));
			}
		}

		switch((int)wParam)
		{
		case VK_LEFT:
			if(mwv->smaf)
			{
				UINT movval = 1000;

				if(GetAsyncKeyState(VK_CONTROL) & 0x8000)
					movval = 100;

				if((mwv->smaf->start->time - mwv->smaf->start->duration) + movval <= PlayingPos)
					seek(mwv->smaf, PlayingPos - movval, mwv->emusounding || mwv->midisounding, FALSE);
				else
					seek(mwv->smaf, mwv->smaf->start->time - mwv->smaf->start->duration, mwv->emusounding || mwv->midisounding, FALSE);

				settitle(hWnd, mwv->smaf, mwv->emusounding, mwv->midisounding);
			}
			break;
		case VK_RIGHT:
			if(mwv->smaf)
			{
				UINT movval = 1000;

				if(GetAsyncKeyState(VK_CONTROL) & 0x8000)
					movval = 100;

				seek(mwv->smaf, PlayingPos + movval, mwv->emusounding || mwv->midisounding, FALSE);

				settitle(hWnd, mwv->smaf, mwv->emusounding, mwv->midisounding);
			}
			break;
		}
		break;
	case WM_KEYUP:
		MainWnd_ShiftOption(hWnd, msg, wParam, lParam);

		break;
	case WM_INITMENUPOPUP:
		{
			HMENU hm = (HMENU)wParam;
			int i;
			int max = GetMenuItemCount(hm);

			for(i = 0; i < max; i++)
			{
				MENUITEMINFO mii = {0};

				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID;
				GetMenuItemInfo(hm, i, TRUE, &mii);
				switch(mii.wID)
				{
				case ID_VTRACK1:
				case ID_VTRACK2:
				case ID_VTRACK3:
				case ID_VTRACK4:
				case ID_VTRACK5:
				case ID_VTRACK6:
				case ID_VTRACK7:
				case ID_VTRACK8:
				case ID_VTRACK9:
				case ID_VTRACK10:
				case ID_VTRACK11:
				case ID_VTRACK12:
				case ID_VTRACK13:
				case ID_VTRACK14:
				case ID_VTRACK15:
				case ID_VTRACK16:
					mii.fMask = MIIM_ID | MIIM_TYPE;
					mii.fType = MFT_OWNERDRAW;
					SetMenuItemInfo(hm, i, TRUE, &mii);
					break;
				}
			}
		}
		break;
	case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT mis = (LPMEASUREITEMSTRUCT)lParam;

			if(mis->CtlType == ODT_MENU)
			{	// これはひどいMeasureItem...
				HDC hmwdc;
				HDC hdc;
				NONCLIENTMETRICS ncm;
				HFONT hfold;
				SIZE s;
				TCHAR str[256];

				hmwdc = GetDC(hWnd);
				hdc = CreateCompatibleDC(hmwdc);

				ncm.cbSize = sizeof(ncm);
				SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);

				hfold = SelectObject(hdc, CreateFontIndirect(&ncm.lfMenuFont));

				getTrackMenuItem(mis->itemID, str, NULL, NULL);
				GetTextExtentPoint32(hdc, str, lstrlen(str), &s);
				mis->itemWidth = s.cx + 4;
				mis->itemHeight = s.cy + 4;

				DeleteObject(SelectObject(hdc, hfold));
				DeleteDC(hdc);
				ReleaseDC(hWnd, hmwdc);
			}

			if(mis->CtlType == ODT_LISTVIEW)
			{
				HDC hmwdc;
				HDC hdc;
				HFONT hfold;
				SIZE s;
				LOGFONT lf = {0};

				hmwdc = GetDC(hListWnd);
				hdc = CreateCompatibleDC(hmwdc);

				lf.lfCharSet = DEFAULT_CHARSET;
				lf.lfHeight = MainOption.EventListFontSize;
				lstrcpy(lf.lfFaceName, MainOption.EventListFontFace);
				hfold = SelectObject(hdc, CreateFontIndirect(&lf));

				GetTextExtentPoint32(hdc, "A", 1, &s);
				//mis->itemWidth = s.cx + 4;
				mis->itemHeight = s.cy;// + 4;

				DeleteObject(SelectObject(hdc, hfold));
				DeleteDC(hdc);
				ReleaseDC(hListWnd, hmwdc);
			}
		}
		break;
	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;

			if(dis->CtlType == ODT_MENU)
			{
				TCHAR str[256];
				MENUITEMINFO mii = {0};
				COLORREF bg = 0xFF000000, fg;

				getTrackMenuItem(dis->itemID, str, &fg, &bg);

				if(bg != 0xFF000000)
				{
					SIZE s;

					SaveDC(dis->hDC);
					SetBkMode(dis->hDC, TRANSPARENT);

					if(dis->itemState & ODS_SELECTED)
					{
						SetTextColor(dis->hDC, fg);
						FillRectangle(dis->hDC, dis->rcItem.left, dis->rcItem.top, dis->rcItem.right, dis->rcItem.bottom, RGB(128, 0, 128));
						FillRectangle(dis->hDC, dis->rcItem.left + 1, dis->rcItem.top + 1, dis->rcItem.right - 1, dis->rcItem.bottom - 1, bg);
					}else
						FillRectangle(dis->hDC, dis->rcItem.left, dis->rcItem.top, dis->rcItem.right, dis->rcItem.bottom, GetSysColor(COLOR_3DFACE));

					GetTextExtentPoint32(dis->hDC, str, lstrlen(str), &s);

					TextOut(dis->hDC, dis->rcItem.left + 2, dis->rcItem.top + (dis->rcItem.bottom - dis->rcItem.top - s.cy) / 2, str, lstrlen(str));

					RestoreDC(dis->hDC, -1);
				}
			}
			if(dis->hwndItem == hStatusWnd)
				if(dis->itemID == 256)	// KH...
					setStatus(mwv->smaf, mwv->emusounding, mwv->midisounding);
				else
					SendMessage(hStatusWnd, msg, wParam, lParam);
			if(dis->hwndItem == hListWnd)
			{
				HDC hdc = dis->hDC;
				RECT rc;

				SaveDC(hdc);

				ListView_GetItemRect(dis->hwndItem, dis->itemID, &rc, LVIR_BOUNDS);

				//SetBkMode(hdc, OPAQUE);
				SetBkMode(hdc, TRANSPARENT);
				// Back
				{
					EVENT* e;
					COLORREF fg, bg;

					{
						LV_ITEM lvi;

						lvi.mask = LVIF_PARAM;
						lvi.iItem = dis->itemID;
						lvi.iSubItem = 0;
						
						ListView_GetItem(hListWnd, &lvi);

						e = (EVENT*)lvi.lParam;
					}

					if((e->status & 0xF0) == 0x80
						|| (e->status & 0xF0) == 0x90
						|| (e->status & 0xF0) == 0xB0
						|| (e->status & 0xF0) == 0xC0
						|| (e->status & 0xF0) == 0xE0
						)
					{
						UINT ch = e->status & 0x0F;

						fg = MainOption.TrackTextColors[ch];
						bg = MainOption.TrackColors[ch];
					}else
					{
						fg = RGB(64, 64, 64);
						bg = RGB(240, 240, 240);
					}

					// SelectedBack
					if(dis->itemState & ODS_SELECTED)
					{
						HBRUSH hBack;

						SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
						hBack = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));//CreateHatchBrush(HS_BDIAGONAL, bg);

						FillRect(hdc, &rc, hBack);

						DeleteObject(hBack);

						SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
					}else
					{
						SetBkColor(hdc, bg);
						FillRectangle(hdc, rc.left, rc.top, rc.right, rc.bottom, bg);

						SetTextColor(hdc, fg);
					}
				}

				if(dis->itemState & ODS_FOCUS && GetFocus() == hListWnd)
				{
					RECT rc2 = rc;

					rc2.bottom--;	// above grid fix.

					DrawFocusRect(hdc, &rc2);

					// BIGGER EDGE!!
					rc2.left++;
					rc2.top++;
					rc2.right--;
					rc2.bottom--;
					DrawFocusRect(hdc, &rc2);
				}

				// Text
				{
					TCHAR str[256];
					HFONT hfold;
					LOGFONT lf = {0};

					lf.lfCharSet = DEFAULT_CHARSET;
					lf.lfHeight = MainOption.EventListFontSize;
					lstrcpy(lf.lfFaceName, MainOption.EventListFontFace);
					if(dis->itemState & ODS_SELECTED)
						lf.lfWeight = FW_BOLD;
					hfold = SelectObject(hdc, CreateFontIndirect(&lf));

					ListView_GetItemRect(dis->hwndItem, dis->itemID, &rc, LVIR_LABEL);
					ListView_GetItemText(dis->hwndItem, dis->itemID, 0, str, sizeof(str));
					DrawText(hdc, str, -1, &rc, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

					{
						UINT cols = Header_GetItemCount(ListView_GetHeader(hListWnd));
						UINT i;

						for(i = 1; i < cols; i++)
						{
							SIZE s;
							LPCTSTR p;
							int x;
							int y;
							int j;

							ListView_GetSubItemRect(hListWnd, dis->itemID, i, LVIR_BOUNDS, &rc);
							ListView_GetItemText(dis->hwndItem, dis->itemID, i, str, sizeof(str));
							rc.left += 2;

							p = str;
							x = rc.left;
							y = rc.top;

							{
								HRGN rgn;

								rgn = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);
								SelectClipRgn(hdc, rgn);

								//DrawText(hdc, str, -1, &rc, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
								for(j = 0; p < str + lstrlen(str); j++)
								{
									TextOut(hdc, x, y, p, CharNext(p) - p);
									GetTextExtentPoint32(hdc, p, CharNext(p) - p, &s);
									if(dis->itemState & ODS_SELECTED)
									{
										HFONT hfold;

										lf.lfWeight = FW_NORMAL;
										hfold = SelectObject(hdc, CreateFontIndirect(&lf));
										lf.lfWeight = FW_NORMAL;

										GetTextExtentPoint32(hdc, p, CharNext(p) - p, &s);

										DeleteObject(SelectObject(hdc, hfold));
									}
									x += s.cx;

									p = CharNext(p);
									if(rc.right < x)
										break;
								}

								SelectClipRgn(hdc, NULL);
								DeleteObject(rgn);
							}
						}
					}

					DeleteObject(SelectObject(hdc, hfold));
				}

				RestoreDC(hdc, -1);
			}
		}
		break;
	case WM_DROPFILES:
		{
			HDROP hd = (HDROP)wParam;
			TCHAR fn[MAX_PATH];

			DragQueryFile(hd, 0, fn, sizeof(fn));
			DragFinish(hd);

			OpenSMAFCommand(hWnd, &mwv->smaf, fn);
		}
		break;
	case WM_COMMAND:
		MainWnd_Command(hWnd, msg, wParam, lParam);
		break;
	case WM_NOTIFY:
		switch(((NMHDR*)lParam)->code)
		{
		case TTN_NEEDTEXT:
			/*
			{
				TCHAR mesg[256];

				if(LoadString(GetModuleHandle(NULL), ((NMHDR*)lParam)->idFrom, mesg, sizeof(mesg)))
				{
					((TOOLTIPTEXT*)lParam)->lpszText = mesg;	// ...ok?
				}
			}
			*/
			((TOOLTIPTEXT*)lParam)->hinst = GetModuleHandle(NULL);
			((TOOLTIPTEXT*)lParam)->lpszText = (LPTSTR)((NMHDR*)lParam)->idFrom;
			break;
		case LVN_ITEMCHANGED:
			if(((NM_LISTVIEW*)lParam)->uNewState & LVIS_SELECTED)
			{
				EVENT* e;
				/*
				int i;

				for(i = 0, e = mwv->smaf->events; e; i++, e = e->next)
					if(i == ((NM_LISTVIEW*)lParam)->iItem)
						break;
				*/
				if(((NM_LISTVIEW*)lParam)->iItem != -1)
				{
					LV_ITEM lvi;

					lvi.iItem = ((NM_LISTVIEW*)lParam)->iItem;
					lvi.iSubItem = 0;
					lvi.mask = LVIF_PARAM;

					ListView_GetItem(hListWnd, &lvi);

					e = (EVENT*)lvi.lParam;
				}

				if(e != mwv->selectedevent)
				{
					//OutputDebugString("LV");
					mwv->selectedevent = e;
					SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETHILIGHTEVENT, (LPARAM)e);
					// TODO: on DBLCLK
					//SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SCROLLTO, e->time);
				}
			}
			break;
		case NM_DBLCLK:
			{
				int i = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);

				if(i != -1)
				{
					EVENT* e;
					LV_ITEM lvi;

					lvi.iItem = ((NM_LISTVIEW*)lParam)->iItem;
					lvi.iSubItem = 0;
					lvi.mask = LVIF_PARAM;

					ListView_GetItem(hListWnd, &lvi);

					e = (EVENT*)lvi.lParam;

					if(e == mwv->selectedevent)
					{
						//OutputDebugString("DB");
						SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SCROLLTO, e->time);
					}
				}
			}
			break;
		case NM_RCLICK:
			{
				int i = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);

				if(i != -1)
				{
					EVENT* e;
					LV_ITEM lvi;

					lvi.iItem = ((NM_LISTVIEW*)lParam)->iItem;
					lvi.iSubItem = 0;
					lvi.mask = LVIF_PARAM;

					ListView_GetItem(hListWnd, &lvi);

					e = (EVENT*)lvi.lParam;

					if(e == mwv->selectedevent)
					{
						//OutputDebugString("RC");
						// TODO: showMenu
					}
				}
			}
			break;
			/*
		case NM_CUSTOMDRAW:
			{
				NMLVCUSTOMDRAW* lvcd = (NMLVCUSTOMDRAW*)lParam;
				switch(lvcd->nmcd.dwDrawStage)
				{
				case CDDS_PREPAINT:
					return CDRF_NOTIFYITEMDRAW;
				case CDDS_ITEMPREPAINT:
					if(lvcd->nmcd.lItemlParam)
					{
						EVENT* e = (EVENT*)lvcd->nmcd.lItemlParam;

						if((e->status & 0xF0) == 0x80
							|| (e->status & 0xF0) == 0x90
							|| (e->status & 0xF0) == 0xB0
							|| (e->status & 0xF0) == 0xC0
							|| (e->status & 0xF0) == 0xE0
							)
						{
							UINT ch = e->status & 0x0F;

							lvcd->clrText = MainOption.TrackTextColors[ch];
							lvcd->clrTextBk = MainOption.TrackColors[ch];
						}else
						{
							lvcd->clrText = RGB(64, 64, 64);
							lvcd->clrTextBk = RGB(240, 240, 240);
						}
					}
					return 0;
				}
			}

			break;
			*/
		}
		break;
	case WM_PROLLNOTIFY:
		MainWnd_PRollNotify(hWnd, msg, wParam, lParam);
		break;
	case WM_TIMER:
		{
			UINT mscale = SendMessage(hPRollWnd, WM_PROLL, PROLLWM_GETSCROLLMAG, 0);
			UINT rwidth = SendMessage(hPRollWnd, WM_PROLL, PROLLWM_GETROLLWIDTH, 0);

			if(mwv->emusounding)
			{
				if(wParam == 0)	// ID:0 = Redraw
				{
					//if(MainOption.PlayPosAlignByMagScale)
					//	PlayingPos = PlayingPos / mscale * mscale;

					if(MainOption.PlayPosAlignByMagScale)
						SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETCURRENTPOS, PlayingPos / mscale / rwidth * mscale * rwidth);
					else
						SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETCURRENTPOS, PlayingPos);

					settitle(hWnd, mwv->smaf, mwv->emusounding, mwv->midisounding);

					switch(EmuStatus())
					{
					case EMUSTATUS_PLAYING:
						break;
					case EMUSTATUS_READY:	// music stop.
						SendMessage(hWnd, WM_COMMAND, ID_STOP, 0);
						break;
					default:	// 想定の範囲外です。
						SendMessage(hWnd, WM_COMMAND, ID_STOP, 0);
						MessageBox(hWnd, "Status is NOT 4(Playing) or 3(Ready)\nSTOPPED.", "Error", MB_ICONERROR | MB_OK);
						break;
					}
				}
				if(wParam == 1)	// ID:1 = EmuProc
				{
					UINT cp;
					UINT beginpoint = 0;

					if(mwv->smaf->start)
						beginpoint = mwv->smaf->start->time - mwv->smaf->start->duration;
					if((int)(cp = EmuGetCurrentPos()) == -1)
						cp = 0;
					PlayingPos = cp + beginpoint;
				}
			}

			if(mwv->midisounding)
			{
				if(wParam == 0)	// ID:0 = Redraw
				{
					if(MainOption.PlayPosAlignByMagScale)
						SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETCURRENTPOS, PlayingPos / mscale / rwidth * mscale * rwidth);
					else
						SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETCURRENTPOS, PlayingPos);

					settitle(hWnd, mwv->smaf, mwv->emusounding, mwv->midisounding);
				}
				if(wParam == 1)	// ID:1 = MIDIProc
				{
					if(!EmuMIDIProc())
					{
						SendMessage(hWnd, WM_COMMAND, ID_STOP, 0);
						MessageBox(hWnd, "EmuMIDIProc failed!\nSTOPPED.", "Error", MB_ICONERROR | MB_OK);
						break;
					}

					{
						UINT cp = EmuGetCurrentPos();

						if((int)cp == -1)
							cp = 0;

						PlayingPos = cp;
					}

					switch(EmuStatus())
					{
					case EMUSTATUS_PLAYING:
						break;
					case EMUSTATUS_READY:	// music stop.
						SendMessage(hWnd, WM_COMMAND, ID_STOP, 0);
						break;
					default:	// 想定の範囲外です。
						SendMessage(hWnd, WM_COMMAND, ID_STOP, 0);
						MessageBox(hWnd, "Status is NOT 4(Playing) or 3(Ready)\nSTOPPED.", "Error", MB_ICONERROR | MB_OK);
						break;
					}
				}
			}

			if((mwv->emusounding || mwv->midisounding) && wParam == 0)
			{
				static DWORD lastrefresh = 0;	// static...KH
				UINT pp = PlayingPos;
				RECT rc;

				if(MainOption.PlayPosAlignByMagScale)
					pp = pp / mscale * mscale;

				GetClientRect(hPRollWnd, &rc);

				// FollowMove
				if(MainOption.FollowPlay == 1 || (MainOption.FollowPlay == 2 && GetAsyncKeyState(VK_SHIFT) & 0x8000))
				{
					SendMessage(hPRollWnd, WM_PROLL, MAKEWPARAM(PROLLWM_SCROLLTO, (short)MainOption.FollowSpace), pp);
				}
				if(MainOption.FollowPlay == 3 && (pp < PRollGetScroll(hPRollWnd) || PRollGetScroll(hPRollWnd) + (rc.right + MainOption.FollowSpace) * (UINT)SendMessage(hPRollWnd, WM_PROLL, PROLLWM_GETROLLWIDTH, 0) < pp))
				{
					SendMessage(hPRollWnd, WM_PROLL, MAKEWPARAM(PROLLWM_SCROLLTO, (short)MainOption.FollowSpace), pp);
				}
				//InvalidateRect(hWnd, NULL, FALSE);

				// refresh DetailWnd
				if(lastrefresh + MainOption.DetailRefreshRate < GetTickCount())
				{
					lastrefresh = GetTickCount();
					InvalidateRect(hDetailWnd, NULL, FALSE);
				}
			}
		}
		break;
	case WM_SIZE:
		SendMessage(hToolWnd, msg, wParam, lParam);
		SendMessage(hStatusWnd, msg, wParam, lParam);
		{
			RECT src = {0};
			GetClientRect(hStatusWnd, &src);
			{
				int pw[2];

				if(wParam == SIZE_MAXIMIZED)
				{
					pw[0] = LOWORD(lParam) - 100;
					pw[1] = LOWORD(lParam);
				}else
				{
					pw[0] = LOWORD(lParam) - 100 - src.bottom;
					pw[1] = LOWORD(lParam) - src.bottom;
				}

				SendMessage(hStatusWnd, SB_SETPARTS, sizeof(pw) / sizeof(int), (LPARAM)pw);
			}
			//if(mwv->smaf)
			{
				RECT trc = {0}, src = {0};
				UINT div = MainOption.EventListViewSize;

				if(LOWORD(lParam) < div)
					div = LOWORD(lParam) - 4;

				GetClientRect(hToolWnd, &trc);
				GetClientRect(hStatusWnd, &src);
				SetWindowPos(hListWnd, NULL, 0, trc.bottom, div, HIWORD(lParam) - trc.bottom - src.bottom, SWP_NOZORDER);
				SetWindowPos(hPRollWnd, NULL, div + 4, trc.bottom, LOWORD(lParam) - div - 4, HIWORD(lParam) - trc.bottom - src.bottom, SWP_NOZORDER);
			}//else
				InvalidateRect(hWnd, NULL, FALSE);
		}

		break;
	case WM_MENUSELECT:
		if(HIWORD(wParam) == 0xFFFF && lParam == 0)
		{	// Closed
			SendMessage(hStatusWnd, SB_SIMPLE, FALSE, 0);
		}else
		{
			if(HIWORD(wParam) & MF_HILITE)
			{
				TCHAR mesg[256];

				if(LoadString(GetModuleHandle(NULL), LOWORD(wParam), mesg, sizeof(mesg)))
				{
					SendMessage(hStatusWnd, SB_SIMPLE, TRUE, 0);
					SendMessage(hStatusWnd, SB_SETTEXT, 255 | SBT_NOBORDERS, (LPARAM)mesg);
				}
			}
			if((HIWORD(wParam) & MF_POPUP) || (HIWORD(wParam) & MF_GRAYED))
			{
				SendMessage(hStatusWnd, SB_SIMPLE, TRUE, 0);
				SendMessage(hStatusWnd, SB_SETTEXT, 255 | SBT_NOBORDERS, (LPARAM)"");
			}
		}
		break;
	case WM_CLOSE:
		if(mwv->smaf)
			SendMessage(hWnd, WM_COMMAND, ID_CLOSE, 0);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		ImageList_Destroy((HIMAGELIST)SendMessage(hToolWnd, TB_GETIMAGELIST, 0, 0));
		DestroyWindow(hDetailWnd);
		if(voices)
			hfree(voices);
		break;

	case WM_MOUSEMOVE:
		if(GetCapture() == hWnd)
		{
			int x = (short)LOWORD(lParam) - mwv->separate_offset;
			int width;
			RECT rc;

			GetClientRect(hWnd, &rc);
			width = rc.right;

			if(x < 0)
				x = 0;
			if(width - 4 < x)
				x = width - 4;

			MainOption.EventListViewSize = x;

			{
				RECT rc;

				GetClientRect(hWnd, &rc);
				SendMessage(hWnd, WM_SIZE, 0, MAKELPARAM(rc.right, rc.bottom));
			}
		}

		if(mwv->smaf)
			SetCursor(LoadCursor(NULL, IDC_SIZEWE));

		break;
	case WM_LBUTTONDOWN:
		if(mwv->smaf)
		{
			SetCapture(hWnd);
			{
				int div = MainOption.EventListViewSize;
				RECT rc;

				GetClientRect(hWnd, &rc);
				if(rc.right < div)
					div = rc.right - 4;
				mwv->separate_offset = LOWORD(lParam) - div;
			}
		}
		break;
	case WM_LBUTTONUP:
		if(mwv->smaf)
			ReleaseCapture();
		break;
	case WM_APP:
		switch(wParam)
		{
		case 0:
			if(lParam != 0)
				SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETTEMPOGRIDSHIFT, PlayingPos % lParam);
			SendMessage(hPRollWnd, WM_PROLL, PROLLWM_SETTEMPOGRIDEVERY, lParam);
			break;
		}
		break;

	case WM_PAINT:
		if(!mwv->smaf)
		{
			PAINTSTRUCT ps;
			HDC hdc;
			RECT rc, trc, src;

			hdc = BeginPaint(hWnd, &ps);

			GetClientRect(hWnd, &rc);
			GetClientRect(hToolWnd, &trc);
			GetClientRect(hStatusWnd, &src);
			rc.top = trc.bottom;
			rc.bottom -= src.bottom;

			{
				HBITMAP hbm;
				HDC hsdc;
				BITMAP bm;

				// TODO: これはひどい感じがする...
				hbm = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_LOGO), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
				GetObject(hbm, sizeof(BITMAP), &bm);
				hsdc = CreateCompatibleDC(hdc);
				SelectObject(hsdc, hbm);

				{
					DIBSECTION ds;
					RGBQUAD cl;
					GetObject(hbm, sizeof(DIBSECTION), &ds);
					GetDIBColorTable(hsdc, 1, 1, &cl);
					FillRectangle(hdc, rc.left, rc.top, rc.right, rc.bottom, RGB(cl.rgbRed, cl.rgbGreen, cl.rgbBlue));
				}

				BitBlt(hdc, rc.left + (rc.right - rc.left - bm.bmWidth) / 2, rc.top + (rc.bottom - rc.top - bm.bmHeight) / 2, bm.bmWidth, bm.bmHeight, hsdc, 0, 0, SRCCOPY);

				DrawEdge(hdc, &rc, EDGE_SUNKEN, BF_RECT);

				DeleteDC(hsdc);
				DeleteObject(hbm);
			}

			EndPaint(hWnd, &ps);
		}
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

BOOL initWnd(void)
{
	WNDCLASSEX wc = {0};
	
	wc.cbSize = sizeof(wc);
	wc.cbWndExtra = 4;
	wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_MAIN));
	wc.lpfnWndProc = MainWndProc;
	wc.lpszClassName = "MMFToolWC";
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MAIN);

	if(!RegisterClassEx(&wc))
		return FALSE;

	// I don't like WS_OVERLAPPED.
	if(!(hMainWnd = CreateWindowEx(WS_EX_ACCEPTFILES, wc.lpszClassName, APPNAME, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, wc.hInstance, NULL)))
		return FALSE;

	//wc.cbSize = sizeof(wc);
	wc.cbWndExtra = 0;
	//wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
	//wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	//wc.hInstance = GetModuleHandle(NULL);
	//wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_MAIN));
	wc.lpfnWndProc = DetailWndProc;
	wc.lpszClassName = "MMFToolDetailWC";
	wc.lpszMenuName = NULL;

	if(!RegisterClassEx(&wc))
		return FALSE;

	if(!(hDetailWnd = CreateWindowEx(WS_EX_CLIENTEDGE, wc.lpszClassName, APPNAME" - DetailView", WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_HSCROLL | WS_VSCROLL, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, wc.hInstance, NULL)))
		return FALSE;

	return TRUE;
}

void ReadConfig(void)
{
	int i;
	TCHAR inifile[MAX_PATH];

	{
		LPTSTR last;
		TCHAR lastsix[7];
		GetModuleFileName(NULL, inifile, sizeof(inifile));
		last = (LPTSTR)getLastChar(inifile, '.');
		// TODO: if module file name length lesser than 6...
		lstrcpyn(lastsix, last - 6, 7);
		if(!lstrcmpi(lastsix, "_debug"))
			last -= 6;
		lstrcpy(last, ".ini");
	}

	for(i = 0; i < sizeof(IniEntries) / sizeof(struct tagIniEntries); i++)
	{
		TCHAR ts[32];

		if(IniEntries[i].num == 1)
		{	// Value
			if(IniEntries[i].type == IEV_INTEGER)
			{
				GetPrivateProfileString(IniEntries[i].section, IniEntries[i].key, "", ts, sizeof(ts), inifile);
				if(!lstrlen(ts))
					*IniEntries[i].val = IniEntries[i].def;
				else
					*IniEntries[i].val = strtoint(ts);
			}
			if(IniEntries[i].type == IEV_STRING)
			{
				GetPrivateProfileString(IniEntries[i].section, IniEntries[i].key, (LPCTSTR)IniEntries[i].def, (LPTSTR)IniEntries[i].val, IniEntries[i].bufsize, inifile);
			}
		}else
		{	// Array
			UINT count;

			for(count = 0; count < IniEntries[i].num; count++)
			{
				TCHAR kn[64];

				lstrcpy(kn, IniEntries[i].key);
				wsprintf(ts, "[%u]", count);
				lstrcat(kn, ts);

				if(IniEntries[i].type == IEV_INTEGER)
				{
					GetPrivateProfileString(IniEntries[i].section, kn, "", ts, sizeof(ts), inifile);

					if(!lstrlen(ts))
						IniEntries[i].val[count] = ((DWORD*)IniEntries[i].def)[count];
					else
						IniEntries[i].val[count] = strtoint(ts);
				}
				if(IniEntries[i].type == IEV_STRING)
				{
					MessageBox(NULL, "IEV_STRING Array read will bug.", "WARNING", MB_ICONERROR | MB_OK);
					GetPrivateProfileString(IniEntries[i].section, kn, (LPCTSTR)IniEntries[i].def, (LPTSTR)IniEntries[i].val, IniEntries[i].bufsize, inifile);
				}
			}
		}
	}

	return;
}

void WinMainCRTStartup(void)
{
	LPCTSTR cmdline = skipspace(skipfname(GetCommandLine()));

	{
		LPTSTR c;
		TCHAR fcmd[MAX_PATH];

		lstrcpy(fcmd, cmdline);
		if(c = (LPTSTR)getNextChar(fcmd, ' '))
			*c = '\0';

		if(!lstrcmp(fcmd, "-filtering"))
		{
			TCHAR dname[MAX_PATH];

			lstrcpy(dname, "");
			if(getNextChar(cmdline, ' '))
			{
				lstrcpy(dname, skipspace(getNextChar(cmdline, ' ')));
				*(LPTSTR)skipfname(dname) = '\0';
				if(*dname == '"')
				{
					lstrcpy(dname, CharNext(dname));
					if(getNextChar(dname, '"'))
						*(LPTSTR)getNextChar(dname, '"') = '\0';
				}
			}
			FilterMode(dname);
		}

		if(!lstrcmp(fcmd, "-ctls"))
		{
			TCHAR dname[MAX_PATH];

			lstrcpy(dname, "");
			if(getNextChar(cmdline, ' '))
			{
				lstrcpy(dname, skipspace(getNextChar(cmdline, ' ')));
				*(LPTSTR)skipfname(dname) = '\0';
				if(*dname == '"')
				{
					lstrcpy(dname, CharNext(dname));
					if(getNextChar(dname, '"'))
						*(LPTSTR)getNextChar(dname, '"') = '\0';
				}
			}
			ContentTypeLSMode(dname);
		}
	}

	ReadConfig();

	if(!EmuInitialize())
		ExitProcess(0);

	EmuSetTempo(MainOption.EmuOption.Tempo);
	EmuSetKeypitch(MainOption.EmuOption.Keypitch);
	EmuSetVolume(MainOption.EmuOption.Volume);

	if(!PRollInitialize())
		ExitProcess(0);

	InitCommonControls();

	if(!initWnd())
		ExitProcess(0);

	// OpenCommandlineFile
	if(*cmdline)
	{
		MAINWNDVAL* mwv = (MAINWNDVAL*)GetWindowLong(hMainWnd, 0);
		TCHAR fn[MAX_PATH];

		LPCTSTR p;
		UINT len;

		// lstrlen is ok? strbytes?
		if(*cmdline == '"')
		{
			LPCTSTR n;

			p = CharNext(cmdline);
			if(!(n = getNextChar(CharNext(cmdline), '"')))
				n = cmdline + lstrlen(cmdline);
			len = n - CharNext(cmdline);
		}else
		{
			LPCTSTR n;

			p = cmdline;
			if(!(n = getNextChar(cmdline, ' ')))
				n = cmdline + lstrlen(cmdline);
			len = n - cmdline;
		}

		memcopy(fn, (BYTE*)p, len);

		OpenSMAFCommand(hMainWnd, &mwv->smaf, cmdline);
	}

	{
		MSG msg;
		HACCEL at;

		at = LoadAccelerators(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MAIN));

		while(GetMessage(&msg, NULL, 0, 0))
		{
			if(TranslateAccelerator(/*msg.hwnd*/hMainWnd, at, &msg))
				continue;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		EmuUninitialize();

		ExitProcess(msg.wParam);
	}
}
