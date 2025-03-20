typedef struct
{
	BYTE* data;
	DWORD size;
}CHUNK;

typedef struct _tag_EVENTLIST
{
	DWORD duration;
	DWORD time;

	BYTE status;
	UINT param1;	// note/control no.
	UINT param2;	// velo/gtime/control val.
	UINT param3;	// gtime

	BYTE* data;		// for exclusive
	DWORD size;

	struct _tag_EVENTLIST* next;
}EVENT;

#define SMAFF_NONE	0
#define SMAFF_MA1	1
#define SMAFF_MA2	2
#define SMAFF_MA3	3
#define SMAFF_MA5	4
#define SMAFF_MA7	5
#define SMAFF_UTA2	6
#define SMAFF_UTA3	7
typedef struct
{
	BOOL readonly;

	BYTE* data;
	DWORD size;

	EVENT* start;
	EVENT* stop;

	EVENT* events;
	DWORD dbase;
	DWORD gbase;

	UINT format;

	TCHAR path[MAX_PATH];
	TCHAR name[MAX_PATH];
	// タイトルとかメタ情報が欲しいところ。
}SMAF;

typedef BOOL (*SMAFCALLBACK)(DWORD curpos, DWORD total, UINT state, LPARAM param);
#define SCBSTATE_BEGIN		0
#define SCBSTATE_PROCESSING	1
#define SCBSTATE_FINISH		2

void SetSMAFSilentMode(BOOL mode);
void SetSMAFTypeCheckMode(BOOL mode);
void SetSMAFCallBack(SMAFCALLBACK scb, LPARAM param);
void SetSMAFMPWnd(HWND hWnd);
//findchunkは統合した方がいいのだろうか
BYTE* findchunk4(BYTE* data, DWORD dsize, char* cname, CHUNK* ch);
BYTE* findchunk3(BYTE* data, DWORD dsize, char* cname, CHUNK* ch);
UINT SetVValAndForward(BYTE** pdata, DWORD s);
UINT SetVVal(BYTE* pdata, DWORD s);
UINT SetVValSize(DWORD s);
DWORD GetVValAndForward(BYTE** pdata);
DWORD GetVValSize(BYTE* pdata);
DWORD GetVValAndForward4HPS(BYTE** pdata);
DWORD GetVVal4HPS(BYTE* pdata);
DWORD GetVVal(BYTE* pdata);
UINT getGateTime(EVENT* e);
void ClearEvents(SMAF* smaf);
EVENT* CutoutEvent(SMAF* smaf, EVENT* e);
EVENT* InsertEvent(SMAF* smaf, EVENT* e);
BOOL ConvertEvents2(SMAF* smaf, BYTE* data, DWORD size, BOOL fsetup, DWORD start, DWORD stop, UINT chshift);
BOOL ConvertEvents(SMAF* smaf, BYTE* data, DWORD size, BOOL fsetup, DWORD start, DWORD stop, UINT starttime, UINT stoptime);
SMAF* FreeSmaf(SMAF* smaf);
BOOL ReloadSmaf(SMAF* smaf);
SMAF* CreateSmaf(UINT dbase, UINT gbase);
SMAF* LoadSmaf(LPCTSTR fname);
WORD CalcCrc(BYTE* buf, UINT size);
BYTE* CompileSmaf(SMAF* smaf, DWORD* datasize);
