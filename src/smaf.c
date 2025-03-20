#include <windows.h>
#include "runtime.h"
#include "smaf.h"
#include "bit.h"
#include "exlayer.h"
/*
#include "emusmw5.h"
*/

static SMAFCALLBACK SMAFCallBack = NULL;
static LPARAM SMAFCallBackParam = 0;
static BOOL SilentMode = FALSE;
static HWND MPWnd = NULL;	// MessageboxParentWND
static BOOL TypeCheckMode = FALSE;

void SetSMAFSilentMode(BOOL mode)
{
	SilentMode = mode;

	return;
}

void SetSMAFTypeCheckMode(BOOL mode)
{
	TypeCheckMode = mode;

	return;
}

void SetSMAFCallBack(SMAFCALLBACK scb, LPARAM param)
{
	SMAFCallBack = scb;
	SMAFCallBackParam = param;

	return;
}

void SetSMAFMPWnd(HWND hWnd)
{
	MPWnd = hWnd;

	return;
}

BYTE* findchunk4(BYTE* data, DWORD dsize, char* cname, CHUNK* ch)
{
	DWORD cur;

	for(cur = 0; cur < dsize; )
	{
		if(!memcomp(data + cur, cname, 4))
		{
			ch->data = data + cur + 8;
			ch->size = SwapDword(data + cur + 4);
			return data + cur;
		}
		cur += 8 + SwapDword(data + cur + 4);
	}

	return NULL;
}

BYTE* findchunk3(BYTE* data, DWORD dsize, char* cname, CHUNK* ch)
{
	DWORD cur;

	for(cur = 0; cur < dsize; )
	{
		if(!memcomp(data + cur, cname, 3))
		{
			ch->data = data + cur + 8;
			ch->size = SwapDword(data + cur + 4);
			return data + cur;
		}
		cur += 8 + SwapDword(data + cur + 4);
	}

	return NULL;
}

UINT SetVValAndForward(BYTE** pdata, DWORD s)
{
	UINT wbytes = 0;

	if(0x1FFFFF < s)
		*(*pdata)++ = (BYTE)(((s >> 21) & 0x7F) | 0x80), s &= 0x1FFFFF, wbytes++;
	if(0x3FFF < s)
		*(*pdata)++ = (BYTE)(((s >> 14) & 0x7F) | 0x80), s &= 0x3FFF, wbytes++;
	if(0x7F < s)
		*(*pdata)++ = (BYTE)(((s >> 7) & 0x7F) | 0x80), s &= 0x7F, wbytes++;

	*(*pdata)++ = (BYTE)(s & 0x7F), wbytes++;

	return wbytes;
}

UINT SetVVal(BYTE* pdata, DWORD s)
{
	UINT wbytes = 0;

	if(0x1FFFFF < s)
		*pdata++ = (BYTE)(((s >> 21) & 0x7F) | 0x80), s &= 0x1FFFFF, wbytes++;
	if(0x3FFF < s)
		*pdata++ = (BYTE)(((s >> 14) & 0x7F) | 0x80), s &= 0x3FFF, wbytes++;
	if(0x7F < s)
		*pdata++ = (BYTE)(((s >> 7) & 0x7F) | 0x80), s &= 0x7F, wbytes++;

	*pdata++ = (BYTE)(s & 0x7F), wbytes++;

	return wbytes;
}

UINT SetVValSize(DWORD s)
{
	UINT wbytes = 0;

	if(0x1FFFFF < s)
		s &= 0x1FFFFF, wbytes++;
	if(0x3FFF < s)
		s &= 0x3FFF, wbytes++;
	if(0x7F < s)
		s &= 0x7F, wbytes++;

	wbytes++;

	return wbytes;
}

DWORD GetVValAndForward(BYTE** pdata)
{
	DWORD val = 0;

	do
	{
		val <<= 7;
		val |= **pdata & 0x7F;
	}while(*(*pdata)++ & 0x80);

	return val;
}

DWORD GetVValSize(BYTE* pdata)
{
	BYTE* p = pdata;

	do
	{
		;
	}while(*p++ & 0x80);

	return p - pdata;
}

DWORD GetVValAndForward4HPS(BYTE** pdata)
{
	DWORD val = 0;

	if(**pdata & 0x80)
		val = (((*(*pdata)++) & 0x7F) + 1) << 7;

	val |= *(*pdata)++;

	return val;
}

DWORD GetVVal4HPS(BYTE* pdata)
{
	DWORD val = 0;

	if(*pdata & 0x80)
		val = ((*pdata++ & 0x7F) + 1) << 7;

	val |= *pdata++;

	return val;
}

DWORD GetVVal(BYTE* pdata)
{
	DWORD val = 0;

	do
	{
		val <<= 7;
		val |= *pdata & 0x7F;
	}while(*pdata++ & 0x80);

	return val;
}

UINT getGateTime(EVENT* e)
{
	if((e->status & 0xF0) == 0x80)
		return GetVVal(e->data + 1);
	if((e->status & 0xF0) == 0x90)
		return GetVVal(e->data + 2);

	return 0;
}

void ClearEvents(SMAF* smaf)
{
	EVENT* e = smaf->events;

	while(e)
	{
		EVENT* next = e->next;
		if(e->data)
			hfree(e->data);
		hfree(e);
		e = next;
	}

	smaf->events = NULL;
	smaf->start = NULL;
	smaf->stop = NULL;

	return;
}

// returns "should be smaf->start" value...
EVENT* InsertEvent(SMAF* smaf, EVENT* e)
{
	EVENT* preev = NULL;
	EVENT* ev = smaf->events;
	EVENT* enx;

	// TODO: 8n(Note w/o vel) 9n(Note w/ vel) checking.
	//  >9n 9n->if vel equal, merge with 9n 8n.

	while(ev)
	{
		// TODO: insert order=[F0 Bn Cn En (9n 8n){track order} other]
		if(e->time < ev->time)
			break;
		preev = ev;
		ev = ev->next;
	}

	if(!preev)
	{
		// insert top(smafevents="here"->ev)
		if(!smaf->events)
		{
			enx = NULL;	// NoEntry
		}else
		{
			enx = smaf->events;
			enx->duration -= e->time;
		}
		e->duration = e->time;
		smaf->events = e;

		e->next = enx;

		return e;
	}else
	{	// insert preev->"here"->ev
		enx = preev->next;
		preev->next = e;

		e->duration = e->time - preev->time;
		if(ev)
			ev->duration -= e->duration;

		e->next = enx;

		return smaf->start;
	}
}

EVENT* CutoutEvent(SMAF* smaf, EVENT* e)
{
	EVENT* ev = smaf->events;
	EVENT* preev = NULL;

	// TODO: 8n(Note w/o vel) 9n(Note w/ vel) checking.
	//  >9n cut->8n is default vel, need convert to 9n.

	if(ev)
		while(ev != e && ev)
		{
			preev = ev;
			ev = ev->next;
		}

	if(ev == e)
	{
		if(!preev)
			smaf->events = ev->next;
		else
			preev->next = ev->next;
		if(smaf->start == ev)
			if(!preev)
				smaf->start = ev->next;
			else
				smaf->start = preev;

		if(ev->next)
			ev->next->duration += ev->duration;
	}

	return e;
}

// for HPS: convert to MSF
BOOL ConvertEvents2(SMAF* smaf, BYTE* data, DWORD size, BOOL fsetup, DWORD start, DWORD stop, UINT chshift)
{
	BYTE* p;
	DWORD tdur = 0;
	signed int octshift[4] = {0};
	UINT scbcnt = 0;

	BOOL started = FALSE;
	BOOL stoped = FALSE;

	if(SMAFCallBack)
		SMAFCallBack(0, size, SCBSTATE_BEGIN, SMAFCallBackParam);

	for(p = data; p < data + size; )
	{
		EVENT* e;
		UINT esize = 1;	// command size
		BYTE* topp = p;

		if(SMAFCallBack && scbcnt++ == 100)
		{
			if(!SMAFCallBack(p - data, size, SCBSTATE_PROCESSING, SMAFCallBackParam))
				break;
			scbcnt = 0;
		}

		e = halloc(sizeof(EVENT));
		if(fsetup)
			e->duration = 0;
		else
			e->duration = GetVVal4HPS(p) * smaf->dbase;
		tdur += e->duration;
		e->time = tdur;
		e->status = 0;
		e->size = 0;
		e->data = NULL;

		if(p[0] == 0x00 && p[1] == 0x00 && p[2] == 0x00 && p[3] == 0x00)	// EOS
		{
			esize = 4;

#if 1
			// NOP
			e->status = 0xFF;
			e->size = 1;
			e->data = halloc(e->size);
			e->data[0] = 0x00;
			p += esize;
			InsertEvent(smaf, e);
			continue;
#else
			hfree(e);
			p += esize;
			continue;
#endif
		}

		if(!fsetup)
			GetVValAndForward4HPS(&p);

		if(!fsetup)
		{
			if(!smaf->start && (DWORD)(topp - data) == start)
				smaf->start = e;
			if(!smaf->stop && (DWORD)(topp - data) == stop)
				smaf->stop = e;
		}

		// Parse status!!
		if(p[0] == 0xFF && p[1] == 0xF0)	// Exclusive
		{
			esize = 2 + 1 + p[2];

			// need exclusive convert
#if 1
			e->status = 0xF0;

			if((p[2] == 0x12 || p[2] == 0x1C) && p[3] == 0x43 && p[4] == 0x03 && p[9] == 0x01)
			{
				BYTE cmd[0x30];

				{
					CHPARAM chp;
					OPPARAM opp[4];

					if(!readExclusiveFMAll4HPS(p + 2, &chp, opp))
					{
						// 無理だったらそのままコピー
						e->size = 1 + p[2];
						e->data = halloc(e->size);
						memcopy(e->data, p + 2, e->size);
					}else
					{
						e->size = setMA3Exclusive(cmd, &chp, opp);
						e->data = halloc(e->size);
						memcopy(e->data, cmd, e->size);
						e->data[0] = (BYTE)(e->size - 1);	// KH...
					}
				}
			}else
			{
				e->size = 1 + p[2];
				e->data = halloc(e->size);
				memcopy(e->data, p + 2, e->size);
			}
#else
			hfree(e);
			p += esize;
			continue;
#endif
		}else if(p[0] == 0xFF && p[1] == 0x00)	// NOP
		{
			esize = 2;
			e->size = 1;
			e->data = halloc(e->size);
			e->status = 0xFF;
			e->data[0] = 0x00;
		}else
		{
			if(p[0] != 0x00)	// Note
			{
				BYTE ch;
				BYTE oct;
				BYTE note;
				BYTE absnote;

				ch = (p[0] >> 6 & 0x03);
				oct = p[0] >> 4 & 0x03;
				note = p[0] & 0x0F;
				// MIDI's 69 = 440Hz(A) = Oct2,9
				absnote = 36 + (oct + octshift[ch]) * 12 + note;

				e->status = 0x90 | (ch + chshift);

				{
					BYTE* q = p + 1;
					DWORD gt;

					gt = GetVValAndForward4HPS(&q) * smaf->gbase;
					esize = q - p;
					e->size = 2 + SetVValSize(gt);
					e->data = halloc(e->size);

					e->data[0] = absnote;
					e->data[1] = 0x7F;	// velo
					SetVVal(e->data + 2, gt);
				}
			}else
			{
				BYTE ch;
				ch = (p[1] >> 6 & 0x03) + chshift;

				if((p[1] & 0x30) == 0x30)
				{	// LargeSize-control
					if((p[1] & 0x0F) == 0x00)	// Program Change
					{
						esize = 3;
						e->size = 1;
						e->data = halloc(e->size);
						e->status = 0xC0 | ch;
						e->data[0] = p[2];
					}
					if((p[1] & 0x0F) == 0x01)	// Bank Select
					{
						esize = 3;
						e->size = 2;
						e->data = halloc(e->size);
						e->status = 0xB0 | ch;
						e->data[0] = 0x00;	// bankMSB
						e->data[1] = (p[2] & 0x80) ? 0x7D : 0x7C;
						InsertEvent(smaf, e);

						{
							EVENT* ev;
							ev = halloc(sizeof(EVENT));
							ev->duration = 0;
							ev->time = e->time;

							ev->size = 2;
							ev->data = halloc(ev->size);
							ev->status = 0xB0 | ch;
							ev->data[0] = 0x20;	// bankLSB
							ev->data[1] = p[2] & 0x7F;
							e = ev;
							// 0x7C-0x00, 0x7D-0x00.
						}
					}
					if((p[1] & 0x0F) == 0x02)	// Octave Shift
					{
						UINT ch = (p[0] >> 6) & 0x03;

						esize = 3;
						if(0x01 <= p[2] && p[2] <= 0x04)
							octshift[ch] = p[2];
						if(0x81 <= p[2] && p[2] <= 0x84)
							octshift[ch] = -(p[2] - 0x80);

#if 1
						// NOP
						e->size = 1;
						e->data = halloc(e->size);
						e->status = 0xFF;
						e->data[0] = 0x00;
#else
						hfree(e);
						p += esize;

						continue;
#endif
					}
					if((p[1] & 0x0F) == 0x03)	// Modulation
					{
						esize = 3;
						e->size = 2;
						e->data = halloc(e->size);
						e->status = 0xB0 | ch;
						e->data[0] = 0x01;	// mod.
						e->data[1] = p[2];
					}
					if((p[1] & 0x0F) == 0x04)	// PitchBend
					{
						esize = 3;
						e->size = 2;
						e->data = halloc(e->size);
						e->status = 0xE0 | ch;
						e->data[0] = 0x00;
						e->data[1] = p[2];	// tenuki.
					}
					if((p[1] & 0x0F) == 0x07)	// Volume
					{
						esize = 3;
						e->size = 2;
						e->data = halloc(e->size);
						e->status = 0xB0 | ch;
						e->data[0] = 0x07;	// vol
						e->data[1] = p[2];
					}
					if((p[1] & 0x0F) == 0x0A)	// Pan
					{
						esize = 3;
						e->size = 2;
						e->data = halloc(e->size);
						e->status = 0xB0 | ch;
						e->data[0] = 0x0A;	// pan
						e->data[1] = p[2];
					}
					if((p[1] & 0x0F) == 0x0B)	// Expression
					{
						esize = 3;
						e->size = 2;
						e->data = halloc(e->size);
						e->status = 0xB0 | ch;
						e->data[0] = 0x0B;	// exp.
						e->data[1] = p[2];
					}
				}else
				{	// SmallSize-control
					if((p[1] & 0x30) == 0x00)	// Expression
					{
						// TODO: there are more nice codes... e.g.BitShift
						const BYTE exptable[] = {0x00/*resv*/, 0x00, 0x1F, 0x27, 0x2F, 0x37, 0x3F, 0x47, 0x4F, 0x57, 0x5F, 0x67, 0x6F, 0x77, 0x7F, 0x00/*resv*/};

						esize = 2;
						e->size = 2;
						e->data = halloc(e->size);
						e->status = 0xB0 | ch;
						e->data[0] = 0x0B;	// exp.
						e->data[1] = exptable[(p[1] & 0x0F)];
					}
					if((p[1] & 0x30) == 0x10)	// PitchBend
					{
						const BYTE pbtable[] = {0x00/*resv*/, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x00/*resv*/};

						esize = 2;
						e->size = 2;
						e->data = halloc(e->size);
						e->status = 0xE0 | ch;
						e->data[0] = 0x00;
						e->data[1] = pbtable[(p[1] & 0xF0)];// << 3;	// chou tenuki
					}
					if((p[1] & 0x30) == 0x20)	// Modulation
					{
						const BYTE modtable[] = {0x00/*resv*/, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x48, 0x50, 0x60, 0x68, 0x70, 0x7F, 0x00/*resv*/};

						esize = 2;
						e->size = 2;
						e->data = halloc(e->size);
						e->status = 0xB0 | ch;
						e->data[0] = 0x01;	// mod.
						e->data[1] = modtable[(p[1] & 0xF0)];// << 3;
					}
				}
			}
		}

		if(esize == 1)
		{
			if(!SilentMode)
			{
				TCHAR ts[256];

				wsprintf(ts, "ConvertEvents2(HPS-import): What's this? %02X", p[0]);
				MessageBox(MPWnd, ts, "Error", MB_ICONERROR | MB_OK);
			}

			if(!fsetup && !smaf->start)
				smaf->start = smaf->events;

			if(SMAFCallBack)
				SMAFCallBack(0, size, SCBSTATE_FINISH, SMAFCallBackParam);

			return FALSE;
		}

		p += esize;

		InsertEvent(smaf, e);
	}

	if(!fsetup && !smaf->start)
		smaf->start = smaf->events;

	if(SMAFCallBack)
		SMAFCallBack(0, size, SCBSTATE_FINISH, SMAFCallBackParam);

	return TRUE;
}

// for MSF
BOOL ConvertEvents(SMAF* smaf, BYTE* data, DWORD size, BOOL fsetup, DWORD start, DWORD stop, UINT starttime, UINT stoptime)
{
	BYTE* p;
	DWORD tdur = 0;
	UINT scbcnt = 0;

	BOOL started = FALSE;
	BOOL stoped = FALSE;

	BYTE bankmsb[16] = {0x7C};

	bankmsb[9] = 0x7D;

	if(SMAFCallBack)
		SMAFCallBack(0, size, SCBSTATE_BEGIN, SMAFCallBackParam);

#ifdef _DEBUG
	if(fsetup)
	{
		OutputDebugString("*************************\n");
		OutputDebugString("***** NEW SMAF ktkr ***** >");
		OutputDebugString(smaf->name);
		OutputDebugString("\n");
		OutputDebugString("*************************\n");
	}
	OutputDebugString("=========================\n");
	OutputDebugString("===== CONVERT BEGIN =====\n");
	if(fsetup)
		OutputDebugString("=====--   setup   --=====\n");
	else
		OutputDebugString("=====--   music   --=====\n");
	OutputDebugString("=========================\n");
#endif

	for(p = data; p < data + size; )
	{
		EVENT* e;
		BYTE* topp = p;
		int esize = 0;

		if(SMAFCallBack && scbcnt++ == 100)
		{
			if(!SMAFCallBack(p - data, size, SCBSTATE_PROCESSING, SMAFCallBackParam))
				break;
			scbcnt = 0;
		}

		e = halloc(sizeof(EVENT));
		if(fsetup)
			e->duration = 0;
		else
			e->duration = GetVValAndForward(&p) * smaf->dbase;
		tdur += e->duration;
		e->time = tdur;
		e->status = *p++;
		e->size = 0;
		e->data = NULL;

		if(!fsetup)
		{
			if(!smaf->start && start && (DWORD)(topp - data) == start)
				smaf->start = e;
			if(!smaf->stop && stop && (DWORD)(topp - data) == stop)
				smaf->stop = e;
			if(!smaf->start && starttime && starttime <= (e->time - e->duration))
				smaf->start = e;
			if(!smaf->stop && stoptime && stoptime <= (e->time - e->duration))
				smaf->stop = e;
		}

		// Parse status!!
		if((e->status & 0xF0) == 0x80)	// Note w/o verocity: convert gate time
		{
			DWORD gt = GetVVal(p + 1) * smaf->gbase;
			e->size = 1 + SetVValSize(gt);
			e->data = halloc(e->size);
			e->data[0] = p[0];
			SetVVal(e->data + 1, gt);

			esize = -(1 + (int)GetVValSize(p + 1));
		}
		if((e->status & 0xF0) == 0x90)	// Note w/ verocity: convert gate time
		{
			DWORD gt = GetVVal(p + 2) * smaf->gbase;
			e->size = 2 + SetVValSize(gt);
			e->data = halloc(e->size);
			e->data[0] = p[0];
			e->data[1] = p[1];
			SetVVal(e->data + 2, gt);

			esize = -(2 + (int)GetVValSize(p + 2));
		}
		if((e->status & 0xF0) == 0xA0)	// Resv 3byte
		{
			esize = 3;
		}
		if((e->status & 0xF0) == 0xB0)	// Control Change
		{
			esize = 2;
		}
		if((e->status & 0xF0) == 0xC0)	// Program Change
		{
			esize = 1;
		}
		if((e->status & 0xF0) == 0xD0)	// Resv 2byte
		{
			esize = 2;
		}
		if((e->status & 0xF0) == 0xE0)	// PitchBend
		{
			esize = 2;
		}
		if(e->status == 0xF0)	// Exclusive
		{
			BYTE* q = p;
			//
			UINT pktsize = GetVVal(p);
			BYTE* datab = p + GetVValSize(p);

			esize = GetVValAndForward(&q);
			esize += (q - p);	// VVal size

#ifdef _DEBUG
			if(	0
				// MA-3 SetVoiceFM
				|| ((pktsize == 0x1F || pktsize == 0x2F) && datab[0] == 0x43 && datab[1] == 0x79 && datab[2] == 0x06 && datab[3] == 0x7F && datab[4] == 0x01)
				// MA-3 SetVoiceWT
				|| (pktsize == 0x1E && datab[0] == 0x43 && datab[1] == 0x79 && datab[2] == 0x06 && datab[3] == 0x7F && datab[4] == 0x01)
				)
			{
				BYTE cmd[0x30];
				CHPARAM chp;
				OPPARAM opp[4];

				UINT s = 0x30;

				readMA3Exclusive(&chp, opp, p);
				if(chp.type == VOICE_FM)
				{
					s = setMA3Exclusive(cmd, &chp, opp);
					cmd[0] = s - 1;
					cmd[3] = 0x06;
					// ssdでも立ってる...
					cmd[0x0D] |= 1;
				}
				if(chp.type == VOICE_PCM)
				{
					s = setMA3Exclusive(cmd, &chp, opp);
					cmd[0] = s - 1;
					cmd[3] = 0x06;
					//cmd[0x10] |= 1;
				}

				if(memcomp(cmd, p, s))
				{
					TCHAR ts[1024];
					TCHAR dump[512];
					UINT i;

					lstrcpy(ts, "readMA3Exclusive Validation failed!\n");

					lstrcpy(dump, "");
					for(i = 0; i < pktsize + 1; i++)
					{
						TCHAR hex[4];
						wsprintf(hex, "%02X ", *(p + i));
						lstrcat(dump, hex);
					}
					lstrcat(ts, "ORG: ");
					lstrcat(ts, dump);
					lstrcat(ts, "\n");

					lstrcpy(dump, "");
					for(i = 0; i < s; i++)
					{
						TCHAR hex[4];
						wsprintf(hex, "%02X ", cmd[i]);
						lstrcat(dump, hex);
					}
					lstrcat(ts, "NEW: ");
					lstrcat(ts, dump);
					lstrcat(ts, "\n");

					lstrcat(ts, "--->");
					for(i = 0; i < s; i++)
					{
						if(*(p + i) != cmd[i])
						{
							TCHAR hex[4];
							wsprintf(hex, " %02X", i - 1);
							lstrcat(ts, hex);
						}else
							lstrcat(ts, "   ");
					}
					lstrcat(ts, "\n");

					//MessageBox(MPWnd, ts, "Error-debug", MB_ICONERROR | MB_OK);
					OutputDebugString(ts);
				}else
				{
					TCHAR ts[1024];
					wsprintf(ts, "readMA3Exclusive Validation success: Size=%02X BM=%02X BL=%02X PC=%02X NA=%02X", pktsize, chp.bm, chp.bl, chp.pc, chp.na);
					if(chp.type == VOICE_FM)
						wsprintf(ts, "%s DK=%02X", ts, chp.dk);
					lstrcat(ts, "\n");
					//MessageBox(MPWnd, "readMA3Exclusive Validation success.", "Error-debug", MB_ICONINFORMATION | MB_OK);
					OutputDebugString(ts);
				}
			}
#endif

			//*
			// TODO: it is not todo, but HEAVY change.
			// convert MA5 to MA3.
			if(	0
				// MA-5 SetVoiceFM
				|| ((pktsize == 0x1C || pktsize == 0x2A) && datab[0] == 0x43 && datab[1] == 0x79 && datab[2] == 0x07 && datab[3] == 0x7F && datab[4] == 0x01)
				// MA-5 SetVoiceWT
				|| (pktsize == 0x1B && datab[0] == 0x43 && datab[1] == 0x79 && datab[2] == 0x07 && datab[3] == 0x7F && datab[4] == 0x01)
				)
			{
				CHPARAM chp;
				OPPARAM opp[4];

				if(!readMA5Exclusive(&chp, opp, p))
				{
					MessageBox(MPWnd, "ConvertEvents: readMA5Exclusive failed(invalid format VoiceSetExclusive.)", "Error", MB_ICONERROR | MB_OK);
					continue;
				}

				// TODO: なんかへちょい音になってるので調査解明ダイバスター。

				if(chp.type == VOICE_FM)
				{
					BYTE rcmd[0x30];
					UINT size;

					size = setMA3Exclusive(rcmd, &chp, opp);
					e->size = size;
					e->data = halloc(size);
					e->data[0] = size - 1;
					memcopy(e->data + 1, rcmd + 1, size - 1);
					e->data[3] = 0x06;
				}
				if(chp.type == VOICE_PCM)
				{
					BYTE rcmd[0x1F];
					UINT size;

					size = setMA3Exclusive(rcmd, &chp, opp);
					e->size = size;
					e->data = halloc(size);
					e->data[0] = size - 1;
					memcopy(e->data + 1, rcmd + 1, size - 1);
					e->data[3] = 0x06;
				}
				if(chp.type != VOICE_FM && chp.type != VOICE_PCM)
					esize = 0;
				else
					esize = -esize;
			}

			if(	0
				// Reset
				|| (pktsize == 0x06 && datab[0] == 0x43 && datab[1] == 0x79 && datab[3] == 0x7F && datab[4] == 0x7F)
				// Volume
				|| (pktsize == 0x07 && datab[0] == 0x43 && datab[1] == 0x79 && datab[3] == 0x7F && datab[4] == 0x00)
				// ??
				|| (pktsize == 0x07 && datab[0] == 0x43 && datab[1] == 0x79 && datab[3] == 0x7F && datab[4] == 0x07)
				// MA-3,5 SetWave
				|| (datab[0] == 0x43 && datab[1] == 0x79 && datab[3] == 0x7F && datab[4] == 0x03)
				)
			{
				e->size = GetVValSize(p) + pktsize;
				e->data = halloc(e->size);
				memcopy(e->data, p, e->size);
				e->data[GetVValSize(p) + 2] = 0x06;
				esize = -esize;
			}
			//*/

			if(	0
				// MA-5 SetWave
				|| (datab[0] == 0x43 && datab[1] == 0x79 && datab[2] == 0x07 && datab[3] == 0x7F && datab[4] == 0x03)
				)
			{
				// MA-5=>MA-3
				//data copied...convert!
				UINT size = GetVVal(e->data) - 7 - 1;
				BYTE* data = e->data + GetVValSize(e->data) + 7;
				BYTE* p = data;

				UINT newdatasize = 7 + (size * 8 / 7 + ((size % 7) ? 1 : 0)) + 1;
				UINT newsize = SetVValSize(newdatasize) + newdatasize;
				BYTE* newdata;

				BYTE* q;
				BYTE* dstart;
				int left = size;

				// TODO: newsize calculate is Instability?
				q = newdata = halloc(newsize);

				q += SetVVal(q, newdatasize);
				dstart = q;
				*q++ = 0x43;
				*q++ = 0x79;
				*q++ = 0x06;
				*q++ = 0x7F;
				*q++ = 0x03;
				*q++ = datab[5];
				*q++ = datab[6];

				while(left)
				{
					int i;
					BYTE msbflag = 0;

					for(i = 0; i < 7; i++)
					{
						msbflag <<= 1;
						if(i < left)
							msbflag |= ((*(p + i)) & 0x80) ? 1 : 0;
					}

					*q++ = msbflag;

					for(i = 0; i < 7; i++)
					{
						if(left <= i)
							break;
						*q++ = (*p++) & 0x7F;
					}

					left -= i;
				}

				*q++ = 0xF7;

				if(newdatasize != (UINT)(q - dstart))
				{
					MessageBox(MPWnd, "Warning: size calcuration bug: may be data destroied.", "Warning - Bug", MB_ICONWARNING | MB_OK);
					SetVVal(newdata, q - dstart);
				}

				hfree(e->data);
				e->data = newdata;
				e->size = newsize;
			}
#if 0
			// MA-3 => MA-5
			{
				//data copied...convert!
				UINT size = GetVVal(e->data) - 7 - 1;
				BYTE* data = e->data + GetVValSize(e->data) + 7;
				BYTE* p = data;

				UINT newsize = 7 + (size * 7 / 8) + 1;
				BYTE* newdata;

				int count = 0;
				BYTE msbflag;
				BYTE* q;

				newsize += SetVValSize(newsize);

				q = newdata = halloc(newsize);

				q += SetVVal(q, size * 7 / 8);
				*q++ = 0x43;
				*q++ = 0x79;
				*q++ = 0x07;
				*q++ = 0x7F;
				*q++ = 0x03;
				*q++ = datab[5];
				*q++ = datab[6];

				while(p < data + size)
				{
					if(--count < 0)
					{
						msbflag = *p++;
						count = 7;
					}
					*q++ = *p++ | ((msbflag & 0x40) << 1);
				}

				*q = 0xF7;

				hfree(e->data);
				e->data = newdata;
				e->size = newsize;
			}
#endif
		}
		if(e->status == 0xFF)	// NOP or EOS
		{
			if(*p == 0x00)
				esize = 1;
			if(*p == 0x2F)
				esize = 2;
		}

		// copy.
		if(esize < 0)
		{
			p += -esize;
		}else if(esize > 0)
		{
			e->size = esize;
			e->data = halloc(esize);
			memcopy(e->data, p, esize);
			p += esize;
		}else
		{
			if(!SilentMode)
			{
				TCHAR ts[256];

				wsprintf(ts, "ConvertEvents(MSF-import): Program's bug or Illegal SMAF file\nstat=%02X, %02X %02X %02X %02X, stat-offset=%08X", e->status, *p, *(p+1), *(p+2), *(p+3), p - data);
				MessageBox(MPWnd, ts, "What the hell...?", MB_OK);
			}

			// MspIが無かったときの。
			if(!fsetup && !smaf->start)
				smaf->start = smaf->events;

			if(SMAFCallBack)
				SMAFCallBack(0, size, SCBSTATE_FINISH, SMAFCallBackParam);

			return FALSE;
		}

		InsertEvent(smaf, e);

		if(TypeCheckMode)
		{
			if((e->status & 0xF0) == 0xB0 && e->data[0] == 0x00)
				bankmsb[e->status & 0x0F] = e->data[1];
			if((e->status & 0xF0) == 0x80 || (e->status & 0xF0) == 0x90)
				if(bankmsb[e->status & 0x0F] != 0x7D || ((12 < e->data[0] && e->data[0] < 92) || (110 < e->data[0])))
					break;	// Normal NoteEvent...it is not EseUtaMA-3.
		}

		// EOS->quit
		if(e->status == 0xFF && e->size == 2 && e->data[0] == 0x2F && e->data[1] == 0x00)
			break;
	}

	// MspIが無かったときの。
	if(!fsetup && !smaf->start)
		smaf->start = smaf->events;

	if(SMAFCallBack)
		SMAFCallBack(0, size, SCBSTATE_FINISH, SMAFCallBackParam);

#ifdef _DEBUG
	OutputDebugString("=========================\n");
	OutputDebugString("===== CONVERT  END  =====\n");
	OutputDebugString("=========================\n");
#endif

	return TRUE;
}

SMAF* FreeSmaf(SMAF* smaf)
{
	if(!smaf)
	{
		MessageBox(MPWnd, "already smaf == NULL", "BUG?", MB_ICONERROR | MB_OK);
		return NULL;
	}

	if(smaf->data)
		hfree(smaf->data);

	ClearEvents(smaf);

	hfree(smaf);

	return NULL;
}

BOOL ReloadSmaf(SMAF* smaf)
{
	ClearEvents(smaf);

	{
		CHUNK mmmd;
		CHUNK mtr;
		DWORD chkplus;
		DWORD chshift;
		UINT smafstart = 0, smafstop = 0;

		if(!findchunk4(smaf->data, smaf->size, "MMMD", &mmmd))
		{
			if(!SilentMode)
				MessageBox(MPWnd, "This is not SMAF", "Error", MB_ICONERROR | MB_OK);
			return FALSE;
		}

		mmmd.size -= 2;	// MMMD's CRC

		// TODO: Check copy/edit-guard etc. flag.

		{
			CHUNK opda;

			if(findchunk4(mmmd.data, mmmd.size, "OPDA", &opda))
			{
				CHUNK pro;
				if(findchunk3(opda.data, opda.size, "Pro", &pro))
				{
					smafstart = SwapDword(pro.data + 4);
					smafstop = SwapDword(pro.data + 8);
				}
			}
		}

		// Uta-MA2 check
		if(findchunk3(mmmd.data, mmmd.size, "ATR", &mtr) && !findchunk3(mmmd.data, mmmd.size, "MTR", &mtr))
		{
			smaf->format = SMAFF_UTA2;
			if(TypeCheckMode)
				return TRUE;
		}

		// 複数MTRへの対応
		for(chkplus = 0, chshift = 0; chkplus < mmmd.size; chshift += 4)
		{
			BYTE* cstart;

			if(!(cstart = findchunk3(mmmd.data + chkplus, mmmd.size - chkplus, "MTR", &mtr)))
				break;

			// easy-version-check... umm, is it correct...? I don't have confidence.
			if(smaf->format == SMAFF_NONE)
			{
				if(cstart[3] == 0x00)
				{
					smaf->format = SMAFF_MA1;
					if(TypeCheckMode)
						return TRUE;
				}
				if(0x01 <= cstart[3] && cstart[3] <= 0x04)
				{
					smaf->format = SMAFF_MA2;
					if(TypeCheckMode)
						return TRUE;
				}
				if(cstart[3] == 0x05)
					smaf->format = SMAFF_MA3;
				if(cstart[3] == 0x06)
					smaf->format = SMAFF_MA5;
				if(cstart[3] == 0x07)
					smaf->format = SMAFF_MA7;
			}

			chkplus = (mtr.data - mmmd.data) + mtr.size;

			{
				CHUNK dummy;

				switch(mtr.data[2])
				{
				case 0x00: smaf->dbase = 1; break;
				case 0x01: smaf->dbase = 2; break;
				case 0x02: smaf->dbase = 4; break;
				case 0x03: smaf->dbase = 5; break;
				case 0x10: smaf->dbase = 10; break;
				case 0x11: smaf->dbase = 20; break;
				case 0x12: smaf->dbase = 40; break;
				case 0x13: smaf->dbase = 50; break;
				default: if(!SilentMode)MessageBox(MPWnd, "Unknown duration timebase", "Error", MB_ICONERROR | MB_OK); return FALSE;
				}

				switch(mtr.data[3])
				{
				case 0x00: smaf->gbase = 1; break;
				case 0x01: smaf->gbase = 2; break;
				case 0x02: smaf->gbase = 4; break;
				case 0x03: smaf->gbase = 5; break;
				case 0x10: smaf->gbase = 10; break;
				case 0x11: smaf->gbase = 20; break;
				case 0x12: smaf->gbase = 40; break;
				case 0x13: smaf->gbase = 50; break;
				default: if(!SilentMode)MessageBox(MPWnd, "Unknown gatetime timebase", "Error", MB_ICONERROR | MB_OK); return FALSE;
				}

				if(mtr.data[0] == 0x00)	// HPS
				{
					DWORD start, stop;

					// TODO: Read channel stat.
					mtr.data += 4 + 2;	// skip ChStatus
					mtr.size -= 4 + 2;

					// init start,stop......need "findchunk4(Mtsq)"?
					if(findchunk4(mtr.data, mtr.size, "Mtsq", &dummy))
					{
						start = 0;
						stop = dummy.size;
					}

					if(findchunk4(mtr.data, mtr.size, "Mtsu", &dummy))
						if(!ConvertEvents2(smaf, dummy.data, dummy.size, TRUE, 0, 0, chshift))
							if(!SilentMode)
							{
								if(MessageBox(MPWnd, "ConvertEvents2(Mtsu) returns FALSE.\nContinue?", "Warning", MB_ICONWARNING | MB_YESNO) == IDNO)
									return FALSE;
							}else
								return FALSE;

					if(findchunk4(mtr.data, mtr.size, "MspI", &dummy))
					{
						BYTE* p = dummy.data;

						while(p < dummy.data + dummy.size)
						{
							if(*p == 's' && *(p + 1) == 't')
							{
								p += 3;
								start = SwapDword(p);
								p += 4;
							}
							if(*p == 's' && *(p + 1) == 'p')
							{
								p += 3;
								stop = SwapDword(p);
								p += 4;
							}
							if(*p != ',')
								break;
							p++;
						}
					}

					if(findchunk4(mtr.data, mtr.size, "Mtsq", &dummy))
						if(!ConvertEvents2(smaf, dummy.data, dummy.size, FALSE, start, stop, chshift))
							if(!SilentMode)
							{
								if(MessageBox(MPWnd, "ConvertEvents2(Mtsq) returns FALSE.\nContinue?", "Warning", MB_ICONWARNING | MB_YESNO) == IDNO)
									return FALSE;
							}else
								return FALSE;
				}

				if(mtr.data[0] == 0x01 || mtr.data[0] == 0x02)	// MSF
				{
					DWORD start, stop;
					BYTE mtrtype = mtr.data[0];

					// TODO: Read channel stat.
					mtr.data += 4 + 16;
					mtr.size -= 4 + 16;

					// init start,stop.
					if(findchunk4(mtr.data, mtr.size, "Mtsq", &dummy))
					{
						start = 0;
						if(mtrtype == 0x01)	// if compressed?
							stop = SwapDword(dummy.data);
						else
							stop = dummy.size;
					}

					if(findchunk4(mtr.data, mtr.size, "Mtsu", &dummy))
						if(!ConvertEvents(smaf, dummy.data, dummy.size, TRUE, 0, 0, 0, 0))
							if(!SilentMode)
							{
								if(MessageBox(MPWnd, "ConvertEvents(Mtsu) returns FALSE.\nContinue?", "Warning", MB_ICONWARNING | MB_YESNO) == IDNO)
									return FALSE;
							}else
								return FALSE;

					if(findchunk4(mtr.data, mtr.size, "MspI", &dummy))
					{
						BYTE* p = dummy.data;

						while(p < dummy.data + dummy.size)
						{
							if(*p == 's' && *(p + 1) == 't')
							{
								p += 3;
								start = SwapDword(p);
								p += 4;
							}
							if(*p == 's' && *(p + 1) == 'p')
							{
								p += 3;
								stop = SwapDword(p);
								p += 4;
							}
							if(*p != ',')
								break;
							p++;
						}
					}

					if(findchunk4(mtr.data, mtr.size, "Mtsq", &dummy))
					{
						if(mtrtype == 0x01)
						{
							BYTE* data;

							data = halloc(SwapDword(dummy.data));
							if(!huffmanDecode(data, dummy.data + 4, SwapDword(dummy.data), dummy.size - 4))
							{
								hfree(data);
								if(!SilentMode)
									MessageBox(MPWnd, "SMAF: LoadSmaf: huffmanDecode failed", "Error", MB_ICONERROR | MB_OK);
								return FALSE;
							}
							if(!ConvertEvents(smaf, data, SwapDword(dummy.data), FALSE, start, stop, smafstart * smaf->dbase, smafstop * smaf->dbase))
								if(!SilentMode)
								{
									if(MessageBox(MPWnd, "ConvertEvents(Mtsq-compressed) returns FALSE.\nContinue?", "Warning", MB_ICONWARNING | MB_YESNO) == IDNO)
										return FALSE;
								}else
									return FALSE;
							hfree(data);
						}
						if(mtrtype == 0x02)
							if(!ConvertEvents(smaf, dummy.data, dummy.size, FALSE, start, stop, smafstart * smaf->dbase, smafstop * smaf->dbase))
								if(!SilentMode)
								{
									if(MessageBox(MPWnd, "ConvertEvents(Mtsq-raw) returns FALSE.\nContinue?", "Warning", MB_ICONWARNING | MB_YESNO) == IDNO)
										return FALSE;
								}else
									return FALSE;
						// check chakuta?
					}
				}
			}
		}

		// Uta-MA3,5 check
		if(!findchunk3(mmmd.data, mmmd.size, "ATR", &mtr) && findchunk3(mmmd.data, mmmd.size, "MTR", &mtr))
		{
			EVENT* ev;
			BYTE bm[16] = {0x7C};
			BOOL existdrum = FALSE;
			bm[9] = 0x7D;

			for(ev = smaf->events; ev; ev = ev->next)
			{
				if((ev->status & 0xF0) == 0xB0 && ev->data[0] == 0x00)
					bm[ev->status & 0x0F] = ev->data[1];

				if((ev->status & 0xF0) == 0x80 || (ev->status & 0xF0) == 0x90)
					if(bm[ev->status & 0x0F] != 0x7D || ((12 < ev->data[0] && ev->data[0] < 92) || (110 < ev->data[0])))
						break;
					else
						existdrum = TRUE;
			}

			if(!ev && existdrum)
				smaf->format = SMAFF_UTA3;
		}
	}

	return TRUE;
}

SMAF* CreateSmaf(UINT dbase, UINT gbase)
{
	SMAF* smaf;

	if(!(smaf = halloc(sizeof(SMAF))))
		return NULL;

	smaf->readonly = FALSE;

	smaf->data = NULL;
	smaf->events = NULL;

	smaf->start = NULL;
	smaf->stop = NULL;

	if(	dbase != 1
		&& dbase != 2
		&& dbase != 4
		&& dbase != 5
		&& dbase != 10
		&& dbase != 20
		&& dbase != 40
		&& dbase != 50
		)
		dbase = 4;
	smaf->dbase = dbase;
	if(	gbase != 1
		&& gbase != 2
		&& gbase != 4
		&& gbase != 5
		&& gbase != 10
		&& gbase != 20
		&& gbase != 40
		&& gbase != 50
		)
		gbase = 4;
	smaf->gbase = gbase;

	smaf->format = SMAFF_NONE;

	lstrcpy(smaf->name, "");
	lstrcpy(smaf->path, "");

	return smaf;
}

SMAF* LoadSmaf(LPCTSTR fname)
{
	SMAF* smaf = NULL;
	HANDLE hf = INVALID_HANDLE_VALUE;
	DWORD read;

	if(!(smaf = CreateSmaf(0, 0)))	// 後で変換するので0でOK。
		goto Error;

	lstrcpy(smaf->path, fname);
	lstrcpy(smaf->name, basename(fname));

	if((hf = CreateFile(fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
	{
		if(!SilentMode)
		{
			TCHAR ts[MAX_PATH + 80];

			wsprintf(ts, "Can't open %s", fname);
			MessageBox(MPWnd, ts, "Error", MB_ICONERROR | MB_OK);
		}

		goto Error;
	}

	smaf->size = GetFileSize(hf, NULL);

	if(!(smaf->data = halloc(smaf->size)))
	{
		if(!SilentMode)
			MessageBox(MPWnd, "Can't alloc memory", "Error", MB_ICONERROR | MB_OK);
		goto Error;
	}
	if(!ReadFile(hf, smaf->data, smaf->size, &read, NULL))
	{
		read = 0;
	}
	if(read < smaf->size)
	{
		if(!SilentMode)
			MessageBox(MPWnd, "Couldn't read file.", "Error", MB_ICONERROR | MB_OK);
		goto Error;
	}

	CloseHandle(hf), hf = INVALID_HANDLE_VALUE;

	if(!ReloadSmaf(smaf))
		goto Error;

	return smaf;
Error:
	if(smaf)
		FreeSmaf(smaf);
	if(hf != INVALID_HANDLE_VALUE)
		CloseHandle(hf);

	return NULL;
}

WORD CalcCrc(BYTE* buf, UINT size)
{
	WORD crc = 0xFFFF;
	UINT i;
	int j;

	for (i = 0; i < size; i++)
	{
		crc ^= (buf[i] << 8);
		for (j = 0; j < 8; j++)
			if (crc & 0x8000U)
				 crc = (crc << 1) ^ /*CRCPoly*/0x1021U;
			else
				crc <<= 1;
	}

	return ~crc;
}

BYTE* CompileSmaf(SMAF* smaf, DWORD* datasize)
{
	BYTE* data;
	BYTE* p;

	p = data = halloc(1048576);	//めんどい

	{
		BYTE* cs = p;
		memcopy(p, "MMMD", 4);	p += 4;
		// とりあえずskip
		p += 4;

		{
			BYTE* cs = p;
			memcopy(p, "CNTI", 4);	p += 4;
			p += 4;

			*p++ = 0x00;	// CClass: YAMAHA
			*p++ = 0x32;	// CT: RingMelo: よくわからん。
				// MA-3=32,33
				// MA-5=34~38
				// MA-7=39...other?
			*p++ = 0x00;	// CCT: SJIS
			*p++ = 0xF8;	// CS: Edit Save Trans OK! TODO: set flag.
			*p++ = 0x00;	// CCnt: 0

			StoreBEDword(cs + 4, p - cs - 8);
		}

		// TODO:OPDA...

		// -MA3=MspI,MA5-=Pro? ...is it right?

		{
			BYTE* cs = p;
			// TODO: Select MA-3/MA-5.
			// 05はMA-3、06はMA-5ってこと?
			memcopy(p, "MTR\x05", 4);	p += 4;
			p += 4;

			*p++ = 0x02;	// MobileStandardFormat(NoCompress)
			*p++ = 0x00;	// StreamSequence
			switch(smaf->dbase)	// TB-D
			{
			case 1:		*p++ = 0x00;	break;
			case 2:		*p++ = 0x01;	break;
			case 4:		*p++ = 0x02;	break;
			case 5:		*p++ = 0x03;	break;
			case 10:	*p++ = 0x10;	break;
			case 20:	*p++ = 0x11;	break;
			case 40:	*p++ = 0x12;	break;
			case 50:	*p++ = 0x13;	break;
			}
			switch(smaf->gbase)	// TB-G
			{
			case 1:		*p++ = 0x00;	break;
			case 2:		*p++ = 0x01;	break;
			case 4:		*p++ = 0x02;	break;
			case 5:		*p++ = 0x03;	break;
			case 10:	*p++ = 0x10;	break;
			case 20:	*p++ = 0x11;	break;
			case 40:	*p++ = 0x12;	break;
			case 50:	*p++ = 0x13;	break;
			}

			{
				int i;

				for(i = 0; i < 16; i++)
					*p++ = 0x00;	// chstatus... TODO: set ch.status.
			}

			{
				EVENT* e = smaf->events;

				{
					BYTE* cs = p;
					memcopy(p, "Mtsu", 4);	p += 4;
					p += 4;

					while(e && e->status == 0xF0 && e->time == 0)
					{
						// MasterVolume must in Mtsq.
						if(!memcomp(e->data, "\x07\x43\x79\x06\x7F\x00", 6))	// 0x06 ver
							break;
						if(!memcomp(e->data, "\x07\x43\x79\x07\x7F\x00", 6))	// 0x07 ver
							break;

						*p++ = 0xF0;
						memcopy(p, e->data, e->size);
						p += e->size;
						e = e->next;
					}

					if(p - cs - 8 == 0)
						p = cs;		// Mtsu not need
					else
						StoreBEDword(cs + 4, p - cs - 8);
				}

				{
					BYTE* cs = p;
					memcopy(p, "Mtsq", 4);	p += 4;
					p += 4;

					{
						UINT pret = 0;
						while(e)
						{
							SetVValAndForward(&p, (e->time - pret) / smaf->dbase);

							pret = e->time;
							*p++ = e->status;
							switch(e->status & 0xF0)
							{
							case 0x80:
								{
									DWORD val;
									*p++ = e->data[0];
									val = GetVVal(e->data + 1);
									SetVValAndForward(&p, val / smaf->gbase);
								}
								break;
							case 0x90:
								{
									DWORD val;
									*p++ = e->data[0];
									*p++ = e->data[1];
									val = GetVVal(e->data + 2);
									SetVValAndForward(&p, val / smaf->gbase);
								}
								break;
							default:
								memcopy(p, e->data, e->size);
								p += e->size;
								break;
							}
							e = e->next;
						}
					}

					StoreBEDword(cs + 4, p - cs - 8);
				}
			}

			StoreBEDword(cs + 4, p - cs - 8);
		}

		StoreBEDword(cs + 4, p - cs - 8 + 2);	// +2 = CRC

		StoreBEWord(p, CalcCrc(cs, p - cs));
	}

	if(datasize)
		*datasize = p + 2 - data;

	return data;
}
