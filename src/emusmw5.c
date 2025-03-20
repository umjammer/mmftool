// Emusmw5 Wrapper
//  by Murachue

// TODO: EmuSMAFInitialize/Uninitialize, EMUMIDIInitialize...
//  like
//   SMAFInit{EmuInit(n, 2, n); DeviceControl(...);...}
//   MIDIInit{EmuInit(n, 0, n);}

#include <windows.h>
#include "runtime.h"
#include "smaf.h"
#include "emusmw5.h"
#include "exlayer.h"

static HWND MPWnd = NULL;

// status
static int EmuPlayTempo = 100;
static int EmuPlayKeypitch = 0;
static int EmuPlayVolume = 127;

// Emulation
static HANDLE hEmuDLL;
static BYTE* EmuBuf;
static BYTE* EmuP;

static BOOL EmuSMAFPlaying;
static BOOL EmuMIDIPlaying;

// MIDI Emulation
static DWORD EmuMIDIStartTick;
static EVENT* EmuMIDIEvent;
static SMAF* EmuMIDICurrentSMAF;
static NOTEOFF* EmuMIDINoteoffs = NULL;
static int EmuMIDIVelocities[16];
static int EmuMIDIBankMSB[16];
static int EmuMIDISoloCh = -1;

// MaSound Prototypes
static int (*MaSound_EmuInitialize)(DWORD, DWORD, BYTE*);
static int (*MaSound_Initialize)(int, BYTE*, int);
static int (*MaSound_DeviceControl)(int, int, int, int);
static int (*MaSound_Terminate)();
static int (*MaSound_Create)(int);
static int (*MaSound_Load)(int, BYTE*, DWORD, int, int, int);
static int (*MaSound_Control)(int, int, int, int*, int);
static int (*MaSound_Open)(int, int, int, int);
static int (*MaSound_Standby)(int, int, int);
static int (*MaSound_Start)(int, int, int, int);
static int (*MaSound_GetEmuInfo)(int);
static int (*MaSound_Stop)(int, int, int);
static int (*MaSound_Seek)(int, int, int, int, int);
static int (*MaSound_Close)(int, int, int);
static int (*MaSound_Unload)(int, int, int);
static int (*MaSound_Delete)(int);
static int (*MaSound_Terminate)();
static int (*MaSound_EmuTerminate)();
static int (*SetMidiMsg)(BYTE*, DWORD);

// Functions
void SetEmuMPWnd(HWND hWnd)
{
	MPWnd = hWnd;

	return;
}

static BOOL InitEmuFunctions(HANDLE hdll)
{
	// Only need functions...

	if(!((FARPROC)MaSound_EmuInitialize	= GetProcAddress(hdll, "MaSound_EmuInitialize")))	return FALSE;
	if(!((FARPROC)MaSound_Initialize	= GetProcAddress(hdll, "MaSound_Initialize")))		return FALSE;
	if(!((FARPROC)MaSound_DeviceControl	= GetProcAddress(hdll, "MaSound_DeviceControl")))	return FALSE;
	if(!((FARPROC)MaSound_Terminate		= GetProcAddress(hdll, "MaSound_Terminate")))		return FALSE;
	if(!((FARPROC)MaSound_Create		= GetProcAddress(hdll, "MaSound_Create")))			return FALSE;
	if(!((FARPROC)MaSound_Load			= GetProcAddress(hdll, "MaSound_Load")))			return FALSE;
	if(!((FARPROC)MaSound_Control		= GetProcAddress(hdll, "MaSound_Control")))			return FALSE;
	if(!((FARPROC)MaSound_Open			= GetProcAddress(hdll, "MaSound_Open")))			return FALSE;
	if(!((FARPROC)MaSound_Standby		= GetProcAddress(hdll, "MaSound_Standby")))			return FALSE;
	if(!((FARPROC)MaSound_Start			= GetProcAddress(hdll, "MaSound_Start")))			return FALSE;
	if(!((FARPROC)MaSound_GetEmuInfo	= GetProcAddress(hdll, "MaSound_GetEmuInfo")))		return FALSE;
	if(!((FARPROC)MaSound_Stop			= GetProcAddress(hdll, "MaSound_Stop")))			return FALSE;
	if(!((FARPROC)MaSound_Seek			= GetProcAddress(hdll, "MaSound_Seek")))			return FALSE;
	if(!((FARPROC)MaSound_Close			= GetProcAddress(hdll, "MaSound_Close")))			return FALSE;
	if(!((FARPROC)MaSound_Unload		= GetProcAddress(hdll, "MaSound_Unload")))			return FALSE;
	if(!((FARPROC)MaSound_Delete		= GetProcAddress(hdll, "MaSound_Delete")))			return FALSE;
	if(!((FARPROC)MaSound_Terminate		= GetProcAddress(hdll, "MaSound_Terminate")))		return FALSE;
	if(!((FARPROC)MaSound_EmuTerminate	= GetProcAddress(hdll, "MaSound_EmuTerminate")))	return FALSE;
	if(!((FARPROC)SetMidiMsg			= GetProcAddress(hdll, "SetMidiMsg")))				return FALSE;

	return TRUE;
}

BOOL Emu526829(void)
{
	if(MaSound_DeviceControl(0x05, 2, 0, 0))
	{
		MessageBox(MPWnd, "DeviceControl(0x05, 2, 0, 0) failed!", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}
	if(MaSound_DeviceControl(0x06, 0, 0, 0))
	{
		MessageBox(MPWnd, "DeviceControl(0x06, 0, 0, 0) failed!", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}
	if(MaSound_DeviceControl(0x08, 2, 0, 0))
	{
		MessageBox(MPWnd, "DeviceControl(0x08, 2, 0, 0) failed!", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}
	if(MaSound_DeviceControl(0x09, 0, 0, 0))
	{
		MessageBox(MPWnd, "DeviceControl(0x09, 0, 0, 0) failed!", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	return TRUE;
}

BOOL Emu5282(void)
{
	if(MaSound_DeviceControl(0x05, 2, 0, 0))
	{
		MessageBox(MPWnd, "DeviceControl(0x05, 2, 0, 0) failed!", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}
	if(MaSound_DeviceControl(0x08, 2, 0, 0))
	{
		MessageBox(MPWnd, "DeviceControl(0x08, 2, 0, 0) failed!", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	return TRUE;
}

BOOL EmuMIDIInitialize(void)
{
	if(MaSound_EmuInitialize(48000, 0, EmuP))
	{
		MessageBox(MPWnd, "EmuInitialize(for MIDI) failed!\nM5_EmuHw.dll found?", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	if(!Emu526829())
		return FALSE;

	EmuMIDIPlaying = FALSE;

	return TRUE;
}

void EmuMIDIUninitialize(void)
{
	MaSound_EmuTerminate();

	EmuMIDIPlaying = FALSE;

	return;
}

BOOL EmuInitialize(void)
{
	if(!(hEmuDLL = LoadLibrary("M5_EmuSmw5.dll")))
	{
		MessageBox(MPWnd, "M5_EmuSmw5.dll not found.\nIt requires to run this application.", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	if(!InitEmuFunctions(hEmuDLL))
		return FALSE;

	// TODO: このバッファの意味を調べよう
	EmuBuf = halloc(1024);	// とりあえず確保
	EmuP = EmuBuf;
	while(((DWORD)EmuP & 0xFF) != 0x81)	// MaSound_EmuInitializeのParam3はアドレス下1バイトが0x81以下?てかJust0x81でなくてはならないらしい。Align?
		EmuP++;

	if(!EmuMIDIInitialize())
		return FALSE;

	// Init. FLAGS
	EmuSMAFPlaying = FALSE;
	EmuMIDIPlaying = FALSE;

	return TRUE;
}

void EmuUninitialize(void)
{
	EmuMIDIUninitialize();

	hfree(EmuBuf);

	FreeLibrary(hEmuDLL);
}

BOOL EmuSMAFInitialize(void)
{
	if(MaSound_EmuInitialize(48000, 2, EmuP))
	{
		MessageBox(MPWnd, "EmuInitialize(for PlaySMAF) failed!\nM5_EmuHw.dll found?", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}
	if(MaSound_Initialize(0, EmuBuf/* + 0x2F*/, 0))	// これは大丈夫?
	{
		MessageBox(MPWnd, "Initialize failed!", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}
	if(MaSound_DeviceControl(0x0D, 0, 0, 0))	// これが無いとPCMな音がならないようで。
	{
		MessageBox(MPWnd, "DeviceControl(0x0D, 0, 0, 0) failed!", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}
	if(!Emu526829())
	{
		return FALSE;
	}

	EmuSMAFPlaying = FALSE;

	return TRUE;
}

void EmuSMAFUninitialize(void)
{
	MaSound_Terminate();
	MaSound_EmuTerminate();

	EmuSMAFPlaying = FALSE;

	return;
}

BOOL EmuSMAFPlay(SMAF* smaf, UINT startfrom)
{
	if(!smaf)
	{
		MessageBox(MPWnd, "EmuSMAFPlay: smaf == NULL, it is probably a program bug!", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}
	if(!smaf->data)
	{
		MessageBox(MPWnd, "EmuSMAFPlay: No data. Please compile before EmuPlay.", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	EmuMIDIUninitialize();	// Uninitializeエラー無視...(これはvoidだが)

	// Init
	if(!EmuSMAFInitialize())
		return FALSE;

	// open
	if(MaSound_Create(1) != 1)
	{
		MessageBox(MPWnd, "Create failed!", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}
	if(MaSound_Load(1, smaf->data, smaf->size, 1, (int)NULL, 0) != 1)	// なんかATS-MA5みるとコールバックっぽいけど、ATS-MA5では一度も呼ばれてない。NULL指定してもOKだったので採用。
	{
		MessageBox(MPWnd, "Load failed!\nMay be incorrect SMAF specified.(CRC etc.)", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}
	if(MaSound_Open(1, 1, 0, 0))
	{
		MessageBox(MPWnd, "Open failed!", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}
	if(MaSound_Standby(1, 1, 0))
	{
		MessageBox(MPWnd, "Standby failed!", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	// Play
	if(MaSound_Control(1, 1, 0, &EmuPlayVolume, 0))	// 音量 between 0 to 127
	{
		MessageBox(MPWnd, "Control volume failed!", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}
	if(MaSound_Control(1, 1, 2, &EmuPlayKeypitch, 0))	// キーピッチ between -12 to 12
	{
		MessageBox(MPWnd, "Control keypitch failed!", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}
	if(MaSound_Control(1, 1, 1, &EmuPlayTempo, 0))	// テンポ(%) between 70 to 130.
	{
		MessageBox(MPWnd, "Control tempo failed!", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	if(MaSound_Seek(1, 1, startfrom, 0, 0))
	{
		MessageBox(MPWnd, "Seek failed", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	if(MaSound_Start(1, 1, 1, 0))
	{
		MessageBox(MPWnd, "Start failed!", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	EmuSMAFPlaying = TRUE;

	return TRUE;
}
void EmuSMAFStop(void)
{
	MaSound_Stop(1, 1, 0);

	MaSound_Close(1, 1, 0);
	MaSound_Unload(1, 1, 0);
	MaSound_Delete(1);

	EmuSMAFUninitialize();

	EmuSMAFPlaying = FALSE;

	if(!EmuMIDIInitialize())
		return;

	return;
}

//SMAF/MIDI共用
int EmuStatus(void)
{
	if(EmuSMAFPlaying)
		return MaSound_Control(1, 1, 6, 0, 0);
	if(EmuMIDIPlaying)
	{
		if(EmuMIDIEvent)
			return EMUSTATUS_PLAYING;
		else
			return EMUSTATUS_READY;
	}

	return -1;
}

int EmuInfo(void)
{
	return MaSound_GetEmuInfo(4);
}

int EmuSetMidiMsg(BYTE* data, DWORD size)
{
	return SetMidiMsg(data, size);
}

BOOL EmuSetVolume(UINT vol)
{
	EmuPlayVolume = vol;

	if(EmuSMAFPlaying && MaSound_Control(1, 1, 0, &vol, 0))
		return TRUE;
	if(EmuMIDIPlaying)
	{
		BYTE cmd[] = {0xF0, 0x43, 0x79, 0x05, 0x7E, 0x09, EmuPlayVolume, 0xF7};
		if(SetMidiMsg(cmd, sizeof(cmd)))
			return TRUE;
	}

	return FALSE;
}
BOOL EmuSetTempo(UINT tempo)
{
	EmuPlayTempo = tempo;

	if(EmuSMAFPlaying && MaSound_Control(1, 1, 1, &tempo, 0))
		return TRUE;

	return FALSE;
}
BOOL EmuSetKeypitch(int key)
{
	EmuPlayKeypitch = key;

	if(EmuSMAFPlaying && MaSound_Control(1, 1, 2, &key, 0))
		return TRUE;

	return FALSE;
}

UINT EmuGetVolume(void)
{
	return EmuPlayVolume;
}
UINT EmuGetTempo(void)
{
	return EmuPlayTempo;
}
int EmuGetKeypitch(void)
{
	return EmuPlayKeypitch;
}

BOOL EmuMIDISetVolume(void)
{
	BYTE cmd[] = {0xF0, 0x43, 0x79, 0x05, 0x7E, 0x09, EmuPlayVolume, 0xF7};
	if(EmuSetMidiMsg(cmd, sizeof(cmd)))
	{
		MessageBox(MPWnd, "EmuMIDISetVolume failed", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	return TRUE;
}

BOOL DoEvent(EVENT* ev, BOOL issound)
{
	// sounding
	if((ev->status & 0xF0) == 0x80 || (ev->status & 0xF0) == 0x90)
	{
		int ch = ev->status & 0x0F;

		if((ev->status & 0xF0) == 0x90)
			EmuMIDIVelocities[ch] = ev->data[1];

		if(issound && (EmuMIDISoloCh == -1 || EmuMIDISoloCh == (ev->status & 0x0F)))
		{
			BYTE cmd[3];

			cmd[0] = 0x90 | ch;
			cmd[1] = ev->data[0];
			if(EmuMIDIBankMSB[ch] != 125)	// 古いやつだと0x00と0x80とか使ってるから死ねる。
			{
				//if(EmuMIDICurrentSMAF->format == SMAFF_MA5)
				//	cmd[1] -= 12;
				cmd[1] += EmuPlayKeypitch;
			}

			cmd[2] = EmuMIDIVelocities[ch];

			//if(EmuMIDIBankMSB[ch] == 125)	// TODO: KH code...is it right?
			//	cmd[2] /= 2;

			if(SetMidiMsg(cmd, 3))
			{
				MessageBox(MPWnd, "EmuMIDI: DoEvent NoteON Failed", "Error", MB_ICONERROR | MB_OK);
				return FALSE;
			}

			{
				NOTEOFF* no = EmuMIDINoteoffs;

				if(EmuMIDINoteoffs)
				{
					while(no->next)
						no = no->next;
					no->next = halloc(sizeof(NOTEOFF));
					no = no->next;
				}else
				{
					EmuMIDINoteoffs = halloc(sizeof(NOTEOFF));
					no = EmuMIDINoteoffs;
				}

				no->ch = ch;
				no->sound = ev->data[0];
				if(EmuMIDIBankMSB[ch] != 125)
				{
					//if(EmuMIDICurrentSMAF->format == SMAFF_MA5)
					//	no->sound -= 12;
					no->sound += EmuPlayKeypitch;
				}
				no->time = ev->time + GetVVal(ev->data + 1 + (((ev->status & 0xF0) == 0x90) ? 1 : 0))/* * EmuMIDICurrentSMAF->gbase*/;
				no->next = NULL;
			}
		}
	}
	if((ev->status & 0xF0) == 0xB0)
	{
		BYTE cmd[3];
		int ch = ev->status & 0x0F;

		cmd[0] = ev->status;
		cmd[1] = ev->data[0];
		cmd[2] = ev->data[1];
		if(cmd[1] == 0x00 && (cmd[2] != 0x7C && cmd[2] != 0x7D))
			if(ch != 9)
				cmd[2] = 0x7C;
			else
				cmd[2] = 0x7D;

		if(SetMidiMsg(cmd, 3))
		{
			MessageBox(MPWnd, "EmuMIDI: DoEvent CC Failed", "Error", MB_ICONERROR | MB_OK);
			return FALSE;
		}

		if(ev->data[0] == 0x00)
			EmuMIDIBankMSB[ch] = ev->data[1];
	}
	// 0xEn = PitchBend(SMAF only, not GM.)
	if((ev->status & 0xF0) == 0xE0)
	{
		BYTE cmd[3];
		int ch = ev->status & 0x0F;

		cmd[0] = ev->status;
		cmd[1] = ev->data[0];
		cmd[2] = ev->data[1];

		if(SetMidiMsg(cmd, 3))
		{
			MessageBox(MPWnd, "EmuMIDI: DoEvent PB Failed", "Error", MB_ICONERROR | MB_OK);
			return FALSE;
		}
	}
	if((ev->status & 0xF0) == 0xC0)
	{
		BYTE cmd[2];
		int ch = ev->status & 0x0F;

		cmd[0] = ev->status;
		cmd[1] = ev->data[0];

		if(SetMidiMsg(cmd, 2))
		{
			MessageBox(MPWnd, "EmuMIDI: DoEvent PC Failed", "Error", MB_ICONERROR | MB_OK);
			return FALSE;
		}
	}
	if(ev->status == 0xF0)
	{
		BYTE* cmd;
		UINT size = 0;
		UINT pktsize = GetVVal(ev->data);	// Faster, faster...
		BYTE* datab = ev->data + GetVValSize(ev->data);

		if(	0
			// Reset
			|| (pktsize == 0x06 && datab[0] == 0x43 && datab[1] == 0x79/* && datab[2] == 0x06*/ && datab[3] == 0x7F && datab[4] == 0x7F)
			// Volume: no need.
			//|| (pktsize == 0x07 && datab[0] == 0x43 && datab[1] == 0x79/* && datab[2] == 0x06*/ && datab[3] == 0x7F && datab[4] == 0x00)
			// ??
			|| (pktsize == 0x07 && datab[0] == 0x43 && datab[1] == 0x79/* && datab[2] == 0x06*/ && datab[3] == 0x7F && datab[4] == 0x07)
			// MA-2 SetVoiceFM
			|| ((pktsize == 0x12 || pktsize == 0x1C) && datab[0] == 0x43 && datab[1] == 0x03 && datab[6] == 0x01)
			// MA-3 SetVoiceFM
			|| ((pktsize == 0x1F || pktsize == 0x2F) && datab[0] == 0x43 && datab[1] == 0x79 && datab[2] == 0x06 && datab[3] == 0x7F && datab[4] == 0x01)
			// MA-3 SetVoiceWT
			|| (pktsize == 0x1E && datab[0] == 0x43 && datab[1] == 0x79 && datab[2] == 0x06 && datab[3] == 0x7F && datab[4] == 0x01)
			// MA-3,5 SetWave
			|| (datab[0] == 0x43 && datab[1] == 0x79/* && datab[2] == 0x06*/ && datab[3] == 0x7F && datab[4] == 0x03)
			)
		{
			size = 1 + pktsize;
			cmd = halloc(size);
			cmd[0] = 0xF0;
			memcopy(cmd + 1, datab, size - 1);
			cmd[3] = 0x06;
		}

		if(	0
			// MA-5 SetVoiceFM
			|| ((pktsize == 0x1C || pktsize == 0x2A) && datab[0] == 0x43 && datab[1] == 0x79 && datab[2] == 0x07 && datab[3] == 0x7F && datab[4] == 0x01)
			// MA-5 SetVoiceWT
			|| (pktsize == 0x1B && datab[0] == 0x43 && datab[1] == 0x79 && datab[2] == 0x07 && datab[3] == 0x7F && datab[4] == 0x01)
			)
		{
			CHPARAM chp;
			OPPARAM opp[4];

			if(!readMA5Exclusive(&chp, opp, ev->data))
			{
				MessageBox(MPWnd, "EmuMIDI: DoEvent readMA5Exclusive failed(invalid format VoiceSetExclusive.)", "Error", MB_ICONERROR | MB_OK);
				return FALSE;
			}

			if(chp.type == VOICE_FM || chp.type == VOICE_PCM)
			{
				BYTE rcmd[0x30];

				size = setMA3Exclusive(rcmd, &chp, opp);
				cmd = halloc(size);
				memcopy(cmd, rcmd, size);
			}
		}

		// Need this...?
		if(	0
			// MA-3 SetVoiceFM
			|| ((pktsize == 0x1F || pktsize == 0x2F) && datab[0] == 0x43 && datab[1] == 0x79 && datab[2] == 0x06 && datab[3] == 0x7F && datab[4] == 0x01)
			// MA-3 SetVoiceWT
			|| (pktsize == 0x1E && datab[0] == 0x43 && datab[1] == 0x79 && datab[2] == 0x06 && datab[3] == 0x7F && datab[4] == 0x01)
			)
		{
			BYTE cmd[0x0B] = {0xF0, 0x43, 0x79, 0x06, 0x7F, 0x02, ev->data[6], ev->data[7], ev->data[8], ev->data[9], 0xF7};

			if(EmuSetMidiMsg(cmd, 0x0B))
			{
				MessageBox(MPWnd, "EmuMIDI: DoEvent EmuSetMidiMsg(Erase?Select?) failed", "Error", MB_ICONERROR | MB_OK);
				return FALSE;
			}
		}

		// MA-3,5 SetWave
		if(datab[0] == 0x43 && datab[1] == 0x79/* && datab[2] == 0x06*/ && datab[3] == 0x7F && datab[4] == 0x03)
		{
			if(datab[0x19] & 0x80)
			{
				BYTE rcmd[0x08] = {0xF0, 0x43, 0x79, 0x06, 0x7F, 0x04, datab[5], 0xF7};

				if(EmuSetMidiMsg(rcmd, 0x08))
				{
					MessageBox(MPWnd, "EmuMIDI: DoEvent EmuSetMidiMsg(DetachWave?) failed", "Error", MB_ICONERROR | MB_OK);
					return FALSE;
				}
			}

			cmd[3] = 0x06;
		}

		if(size > 0)
		{
			int ret = SetMidiMsg(cmd, size);

			hfree(cmd);

			if(ret)
			{
				TCHAR ts[256];
				static BOOL shown = FALSE;

				wsprintf(ts, "EmuMIDI: DoEvent Exclusive failed(send-size=%d)", size);
				if(!shown)	// TODO: Fix this KH fix code
				{
					shown = TRUE;
					MessageBox(MPWnd, ts, "Error", MB_ICONERROR | MB_OK);
					shown = FALSE;
				}
				return FALSE;
			}
		}

		if(	0
			// reset
			|| (pktsize == 0x06 && datab[0] == 0x43 && datab[1] == 0x79/* && datab[2] == 0x06*/ && datab[3] == 0x7F && datab[4] == 0x7F)
			)
			EmuMIDISetVolume();
	}

	return TRUE;
}
static BOOL DoMIDIEvent(BOOL issound)
{
	BOOL doeresult;

	doeresult = DoEvent(EmuMIDIEvent, issound);

	EmuMIDIEvent = EmuMIDIEvent->next;

	return doeresult;
}
BOOL EmuMIDIPlay(SMAF* smaf, UINT startfrom)
{
	EmuMIDICurrentSMAF = smaf;
	EmuMIDIEvent = smaf->events;

	if(!smaf)
	{
		MessageBox(MPWnd, "EmuMIDIPlay: smaf == NULL, it is probably a program bug!", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}
	if(!smaf->events)
	{
		MessageBox(MPWnd, "EmuMIDIPlay: No events. Please put one or more notes onto pianoroll.", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}
	if(!smaf->start)
	{
		MessageBox(MPWnd, "EmuMIDIPlay: smaf->events != NULL but smaf->start == NULL, it is probably a program bug!", "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	// Velocity,Bank,PC set Default
	{
		int i;

		for(i = 0; i < 16; i++)
		{
			EmuMIDIVelocities[i] = 64;
			EmuMIDIBankMSB[i] = 124;
			{
				BYTE cmd[3] = {0xB0 | i, 0x00, 124};
				EmuSetMidiMsg(cmd, sizeof(cmd));
			}
			{
				BYTE cmd[3] = {0xB0 | i, 0x20, 0};
				EmuSetMidiMsg(cmd, sizeof(cmd));
			}
			{
				BYTE cmd[2] = {0xC0 | i, 0x00};
				EmuSetMidiMsg(cmd, sizeof(cmd));
			}
		}

		EmuMIDIBankMSB[9] = 125;	// Tr10=Drum default.
		{
			BYTE cmd[3] = {0xB0 | i, 0x00, 125};
			EmuSetMidiMsg(cmd, sizeof(cmd));
		}
	}

	// Reset
	{
		BYTE cmd[] = {0xF0, 0x43, 0x79, 0x06, 0x7F, 0x7F, 0xF7};
		if(EmuSetMidiMsg(cmd, sizeof(cmd)))
		{
			MessageBox(MPWnd, "SetMidiMsg ini failed", "Error", MB_ICONERROR | MB_OK);
			return FALSE;
		}
	}

	{
		int i;

		for(i = 0; i < 128; i++)
		{
			BYTE cmd[0x08] = {0xF0, 0x43, 0x79, 0x06, 0x7F, 0x04, i, 0xF7};

			if(EmuSetMidiMsg(cmd, 0x08))
			{
				MessageBox(MPWnd, "EmuMIDI: EmuMIDIPlay EmuSetMidiMsg(DetachWave) failed", "Error", MB_ICONERROR | MB_OK);
				return FALSE;
			}
		}
	}

	// forwarding event
	while(EmuMIDIEvent && EmuMIDIEvent->time < startfrom + (smaf->start->time - smaf->start->duration))
	{
		if(!DoMIDIEvent(FALSE))
		{
			MessageBox(MPWnd, "EmuMIDI: EmuMIDIPlay DoEvent Failed", "Error", MB_ICONERROR | MB_OK);
			return FALSE;
		}
	}

	EmuMIDINoteoffs = NULL;

	// set volume
	EmuMIDISetVolume();

	// start!
	EmuMIDIStartTick = GetTickCount() - ((smaf->start->time - smaf->start->duration) + startfrom);
	EmuMIDIPlaying = TRUE;

	return TRUE;
}

static BOOL EmuDeleteAllNoteoff(void)
{
	NOTEOFF* no = EmuMIDINoteoffs;

	while(no)
	{
		NOTEOFF* nx = no->next;

		{
			BYTE cmd[3];
			cmd[0] = 0x90 | no->ch;
			cmd[1] = no->sound;
			cmd[2] = 0;

			if(SetMidiMsg(cmd, 3))
			{
				MessageBox(MPWnd, "EmuMIDI: EmuDeleteAllNoteoff NoteOFF Failed", "Error", MB_ICONERROR | MB_OK);
				return FALSE;
			}
		}

		hfree(no);
		no = nx;
	}

	EmuMIDINoteoffs = NULL;

	return TRUE;
}

BOOL EmuMIDIStop(void)
{
	EmuMIDIPlaying = FALSE;

	if(!EmuDeleteAllNoteoff())
		return FALSE;

	return TRUE;
}

//SMAF/MIDI共用
BOOL EmuSeek(UINT pos)
{
	if(EmuSMAFPlaying)
	{
		MaSound_Stop(1, 1, 0);
		if(MaSound_Seek(1, 1, pos, 0, 0))
			return FALSE;
		MaSound_Start(1, 1, 1, 0);

		return TRUE;
	}
	if(EmuMIDIPlaying)
	{
		SMAF* smaf = EmuMIDICurrentSMAF;

		EmuMIDIStop();

		return EmuMIDIPlay(smaf, pos);
	}

	return FALSE;
}

//SMAF/MIDI共用
int EmuGetCurrentPos(void)
{
	if(EmuSMAFPlaying)
		return MaSound_Control(1, 1, 4, 0, 0);
	if(EmuMIDIPlaying)
		return GetTickCount() - EmuMIDIStartTick;

	return -1;
}

BOOL EmuMIDIProc(void)
{
	DWORD curpos = GetTickCount() - EmuMIDIStartTick;

	{
		NOTEOFF* no = EmuMIDINoteoffs;
		NOTEOFF* pre = NULL;

		while(no)
		{
			NOTEOFF* nx = no->next;
			if(no->time < curpos)
			{
				BYTE cmd[3];
				cmd[0] = 0x90 | no->ch;
				cmd[1] = no->sound;
				cmd[2] = 0;

				if(SetMidiMsg(cmd, 3))
				{
					MessageBox(MPWnd, "EmuMIDI: EmuMIDIProc NoteOFF Failed", "Error", MB_ICONERROR | MB_OK);
					return FALSE;
				}

				hfree(no);

				if(!pre)
					EmuMIDINoteoffs = nx;
				else
					pre->next = nx;
			}else
			{
				pre = no;
			}
			no = nx;
		}
	}

	while(EmuMIDIEvent && EmuMIDIEvent->time < curpos)
	{
		if(!DoMIDIEvent(TRUE))
		{
			MessageBox(MPWnd, "EmuMIDI: EmuMIDIEvent DoEvent Failed", "Error", MB_ICONERROR | MB_OK);
			return FALSE;
		}
	}

	return TRUE;
}

void EmuMIDISetSoloCh(int ch)
{
	EmuMIDISoloCh = ch;

	return;
}
