#define EMUSTATUS_ERROR		-1
#define EMUSTATUS_CREATE	0
#define EMUSTATUS_LOAD		1
#define EMUSTATUS_OPEN		2
#define EMUSTATUS_READY		3
#define EMUSTATUS_PLAYING	4

void SetEmuMPWnd(HWND hWnd);

BOOL Emu526829(void);
BOOL Emu5282(void);
BOOL EmuMIDIInitialize(void);
void EmuMIDIUninitialize(void);
BOOL EmuInitialize(void);
void EmuUninitialize(void);
BOOL EmuSMAFInitialize(void);
void EmuSMAFUninitialize(void);
BOOL EmuSMAFPlay(SMAF* smaf, UINT startfrom);
void EmuSMAFStop(void);
int EmuStatus(void);
int EmuInfo(void);
int EmuGetCurrentPos(void);
BOOL EmuSeek(UINT pos);
int EmuSetMidiMsg(BYTE* data, DWORD size);
BOOL EmuSetVolume(UINT vol);
BOOL EmuSetTempo(UINT tempo);
BOOL EmuSetKeypitch(int key);
UINT EmuGetVolume(void);
UINT EmuGetTempo(void);
int EmuGetKeypitch(void);

BOOL EmuMIDISetVolume(void);
BOOL DoEvent(EVENT* ev, BOOL issound);
BOOL EmuMIDIPlay(SMAF* smaf, UINT startfrom);
BOOL EmuMIDIStop(void);
BOOL EmuMIDIProc(void);
void EmuMIDISetSoloCh(int ch);

typedef struct _tag_NOTEOFF
{
	int ch;
	int sound;
	UINT time;
	struct _tag_NOTEOFF* next;
}NOTEOFF;
