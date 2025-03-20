#define VOICE_FM	0
#define VOICE_PCM	1

typedef struct
{
	BYTE bm;
	BYTE bl;
	BYTE pc;

	// member for drum: normal must set 0 to these
	BYTE na;	// NoteAssign
	BYTE dk;	// DrumKey

	BYTE lfo;
	BYTE pan;
	BOOL pe;
	BYTE alg;	// only FM

	// extended member
	UINT type;
	//BYTE version;

	// member for PCM
	DWORD fs;
	BOOL rm;
	BYTE wavno;
	DWORD lp;
	DWORD ep;
}CHPARAM;

typedef struct
{
				// max val
	BYTE multi;	//15	; only FM
	BYTE dt;	//7		; only FM
	BYTE ar;	//15
	BYTE dr;	//15
	BYTE sr;	//15
	BYTE rr;	//15
	BYTE sl;	//15
	BYTE tl;	//63
	BYTE ksl;	//3		; only FM
	BYTE dam;	//3
	BYTE dvb;	//3
	BYTE fb;	//7		; only FM
	BYTE ws;	//31	; only FM

	BOOL xof;
	BOOL sus;
	BOOL ksr;	//		; only FM
	BOOL eam;
	BOOL evb;
}OPPARAM;

void readExclusiveFMOp4HPS(BYTE* data, OPPARAM* opp);
BOOL readExclusiveFMAll4HPS(BYTE* data, CHPARAM* chp, OPPARAM* opp);

//void setExclusiveFMCh(BYTE* data, CHPARAM* chp);
//void setExclusiveFMOp(BYTE* data, OPPARAM* opp);
//UINT setExclusiveFMAll(BYTE* data, CHPARAM* chp, OPPARAM* opps);
UINT setMA3Exclusive(BYTE* data, CHPARAM* chp, OPPARAM* opp);

BOOL readMA3Exclusive(CHPARAM* chp, OPPARAM* opp, BYTE* data);

UINT setExclusivePCMAll(BYTE* data, CHPARAM* chp, OPPARAM* opp);
BOOL readMA5Exclusive(CHPARAM* chp, OPPARAM* opp, BYTE* exdata);
