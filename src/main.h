#define MIDIMSG2(stat, data1) (DWORD)((stat) | ((data1) << 8))
#define MIDIMSG3(stat, data1, data2) (DWORD)((stat) | ((data1) << 8) | ((data2) << 16))

#ifdef _DEBUG
#define APPNAME "MMFTool(DebugVersion)"
#else
#define APPNAME "MMFTool"
#endif

typedef struct
{
	// public
	UINT DrawFunc;
	COLORREF TrackColors[16];
	COLORREF TrackTextColors[16];

	//private
	UINT FollowPlay;
	int FollowSpace;
	BOOL PlayVisibleOnly;
	BOOL DontPreview;
	UINT EventListViewSize;
	BOOL PlayPosAlignByMagScale;

	UINT DefaultDurationBase;
	UINT DefaultGateBase;

	UINT RefreshRate;
	UINT DetailRefreshRate;

	TCHAR	EventListFontFace[LF_FACESIZE];
	UINT	EventListFontSize;

	struct{
		UINT Tempo;
		int Keypitch;
		UINT Volume;
	}EmuOption;

	struct{
		UINT RollHeight;
		UINT RollWidth;
		BOOL NiagaraTouch;
		UINT DefaultGateTime;
		UINT ScrollMag;
		BOOL ShowOctave;
		BOOL ReadOnly;

		COLORREF	BackColor;
		COLORREF	NoteEdgeColor;
		COLORREF	NoteHilightEdgeColor;
		COLORREF	NoteGridColor;
		COLORREF	NoteHilightGridColor;
		COLORREF	NoteGridTextColor;
		COLORREF	TimeGridColor;
		COLORREF	TimeGridHilightColor;
		COLORREF	DurationGridColor;
		COLORREF	GateGridColor;
		COLORREF	TempoGridColor;
		COLORREF	DurationLineColor;
		COLORREF	GateLineColor;

		COLORREF	CurrentPosColor;
		COLORREF	StartPosColor;
		COLORREF	EndPosColor;

		TCHAR	SystemTextFontFace[LF_FACESIZE];
		int		SystemTextFontSize;
		int		SystemTextFontWeight;

		TCHAR	PosbarTextFontFace[LF_FACESIZE];
		int		PosbarTextFontSize;
		int		PosbarTextFontWeight;

		TCHAR	NoteGridTextFontFace[LF_FACESIZE];
		int		NoteGridTextFontSize;
		int		NoteGridTextFontWeight;
		int		NoteGridTextYOffset;
	}PRollOption;
}MAINOPTION;
