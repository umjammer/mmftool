typedef struct
{
	TCHAR name[17];	// 16 + NULL

	CHPARAM chp;
	OPPARAM opp[4];
}VOICE;

UINT LoadVoices(VOICE** voicestore, LPCTSTR fn);
BOOL setVoice(UINT rec, BOOL forcelsb, UINT lsb, BOOL forcepch, UINT pch, VOICE* voices);
BOOL PresetVoices(VOICE* voices, UINT voicesmax);
