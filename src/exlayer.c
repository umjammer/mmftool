// Exclusive <=> CHPARAM/OPPARAM convert

#include <windows.h>
#include "runtime.h"
#include "smaf.h"
#include "exlayer.h"

void readExclusiveFMOp4HPS(BYTE* data, OPPARAM* opp)
{
	opp->multi	= (data[0] >> 4) & 0x0F;
	opp->ksl	= data[3] & 0x03;
	opp->tl		= (data[3] >> 2) & 0x3F;
	opp->ar		= (data[2] >> 4) & 0x0F;
	opp->dr		= data[1] & 0x0F;
	opp->sl		= data[2] & 0x0F;
	opp->rr		= (data[1] >> 4) & 0x0F;
	opp->ws		= data[4] & 0x07;
	opp->dvb	= (data[4] >> 6) & 0x03;
	opp->dam	= (data[4] >> 4) & 0x03;
	opp->evb	= (data[0] & 0x08) ? TRUE : FALSE;
	opp->xof	= 0; //(data[0] & 0x04) ? TRUE : FALSE;	// EGT
	opp->sus	= (data[0] & 0x02) ? TRUE : FALSE;
	opp->ksr	= (data[0] & 0x01) ? TRUE : FALSE;
	opp->eam	= (data[4] & 0x08) ? TRUE : FALSE;
	opp->fb		= 0;
	opp->dt		= 0;
	opp->sr		= 0;

	return;
}

BOOL readExclusiveFMAll4HPS(BYTE* data, CHPARAM* chp, OPPARAM* opp)
{
	// data pointing here->[SIZE][43][03]...

	if(data[0] != 0x12 && data[0] != 0x1C)
		return FALSE;

	if(data[1] != 0x43 || data[2] != 0x03 || data[7] != 0x01)
		return FALSE;

	if(data[4] & 0x80)	// TODO: drum don't supported yet... where is NoteAssign/DrumNote!?
		return FALSE;

	chp->lfo = (data[6] >> 6) & 0x03;
	chp->alg = data[6] & 0x07;
	chp->bm = (data[4] & 0x80) ? 0x7D : 0x7C;
	chp->bl = data[4] & 0x7F;	// TODO: FIX on Drum...
	chp->pc = data[5];
	chp->type = VOICE_FM;
	chp->pan = 16;
	chp->pe = FALSE;

	// FIX
	chp->na = 0;
	chp->dk = 0;

	// TODO: support EGT...
	readExclusiveFMOp4HPS(data + 8, &opp[0]);
	opp[0].fb		= (data[6] >> 3) & 0x07;	// irregular!!
	readExclusiveFMOp4HPS(data + 8 + 5, &opp[1]);

	if(data[0] == 0x1C)
	{
		readExclusiveFMOp4HPS(data + 8 + 5 + 5, &opp[2]);
		readExclusiveFMOp4HPS(data + 8 + 5 + 5 + 5, &opp[3]);
	}

	return TRUE;
}

// check this function
static void setExclusiveFMCh(BYTE* data, CHPARAM* chp)
{
	data[0] = 0xF0;
	data[1] = 0x43;
	data[2] = 0x79;
	//if(chp->version != 0x06)
		data[3] = 0x07;
	//else
	//	data[3] = 0x06;
	data[4] = 0x7F;
	data[5] = 0x01;
	data[6] = chp->bm;
	data[7] = chp->bl;
	data[8] = chp->pc;
	data[9] = chp->na;
	data[12] = chp->dk;
	data[11] &= ~0x30;
	data[11] |= ((chp->lfo & 0x02) ? 0x10 : 0) | ((chp->pan & 0x10) ? 0x20 : 0);
	data[13] &= ~0x78;
	data[13] |= ((chp->pan & 0x0F) << 3);
	data[14] |= ((chp->lfo & 0x01) ? 0x40 : 0) | ((chp->pe) ? 0x20 : 0) | (chp->alg);

	return;
}

// CHECK THIS FUNCTION
static void setExclusiveFMOp(BYTE* data, OPPARAM* opp)
{
	// X1 XX XX XX 23 45 67 89 AX BC DE FG

	data[0] &= ~0x0F;
	data[0] |= ((opp->sr & 0x08) ? 8 : 0) | ((opp->rr & 0x08) ? 4 : 0) | ((opp->ar & 0x08) ? 2 : 0) | ((opp->tl & 0x20) ? 1 : 0);

	data[4] = ((opp->sr & 0x07) << 4) | (opp->xof ? 8 : 0) | (opp->sus ? 2 : 0) | (opp->ksr ? 1 : 0);
	data[5] = ((opp->rr & 0x07) << 4) | (opp->dr);
	data[6] = ((opp->ar & 0x07) << 4) | (opp->sl);
	data[7] = ((opp->tl & 0x1F) << 2) | (opp->ksl);
	data[8] &= ~0x30;
	data[8] |= ((opp->multi & 0x08) ? 0x20 : 0) | ((opp->ws & 0x10) ? 0x10 : 0);
	data[9] = ((opp->dam) << 5) | ((opp->eam) ? 0x10 : 0) | ((opp->dvb) << 1) | ((opp->evb) ? 1 : 0);
	data[10] = ((opp->multi & 0x07) << 4) | (opp->dt);
	data[11] = ((opp->ws & 0x0F) << 3) | opp->fb;

	return;
}

static UINT setExclusiveFMAll(BYTE* data, CHPARAM* chp, OPPARAM* opps)
{
	UINT size;
	UINT i;

	for(i = 0; i < 0x30; i++)
		data[i] = 0;

	setExclusiveFMCh(data, chp);

	setExclusiveFMOp(data + 11, opps + 0);
	setExclusiveFMOp(data + 19, opps + 1);

	if(chp->alg <= 1)
	{
		size = 0x20;
	}else
	{
		setExclusiveFMOp(data + 27, opps + 2);
		setExclusiveFMOp(data + 35, opps + 3);
		size = 0x30;
	}

	data[size - 1] = 0xF7;

	return size;
}

static UINT setExclusivePCMAll(BYTE* data, CHPARAM* chp, OPPARAM* opp)
{
	UINT i;

	for(i = 0; i < 0x1F; i++)
		data[i] = 0;

	data[0] = 0xF0;
	data[1] = 0x43;
	data[2] = 0x79;
	data[3] = 0x07;
	data[4] = 0x7F;
	data[5] = 0x01;
	data[6] = chp->bm;
	data[7] = chp->bl;
	data[8] = chp->pc;
	data[9] = chp->na;
	data[10] = 0x01;
	data[11] |= ((chp->pan & 0x10) ? 0x10 : 0) | ((chp->fs & 0x80) ? 0x20 : 0) | ((chp->fs & 0x8000) ? 0x40 : 0);
	data[11] |= ((opp->ar & 0x08) ? 1 : 0) | ((opp->rr & 0x08) ? 2 : 0) | ((opp->sr & 0x08) ? 4 : 0) | ((chp->lfo & 0x02) ? 8 : 0);
	data[12] = (BYTE)((chp->fs >> 8) & 0x7F);
	data[13] = (BYTE)(chp->fs & 0x7F);
	data[14] = ((chp->pan & 0x0F) << 3) | ((chp->pe) ? 1 : 0);
	data[15] = (chp->lfo & 0x01) ? 0x40 : 0;

	data[16] = ((opp->sr & 0x07) << 4) | ((opp->xof) ? 8 : 0) | ((opp->sus) ? 2 : 0);
	data[17] = ((opp->rr & 0x07) << 4) | (opp->dr);
	data[18] = ((opp->ar & 0x07) << 4) | (opp->sl);
	data[19] |= ((opp->tl & 0x20) ? 0x40 : 0);
	data[19] |= ((chp->lp & 0x80) ? 2 : 0);
	data[20] = ((opp->tl) & 0x1F) << 2;
	data[21] = ((opp->dam) << 5) | ((opp->eam) ? 0x10 : 0) | ((opp->dvb) << 1) | ((opp->evb) ? 1 : 0);
	data[24] = (BYTE)((chp->lp >> 8) & 0x7F);
	data[25] = (BYTE)(chp->lp & 0x7F);
	data[26] = (BYTE)((chp->ep >> 8) & 0x7F);
	data[27] = ((chp->rm) ? 0x20 : 0) | ((chp->ep & 0x80) ? 0x40 : 0);
	data[28] = (BYTE)(chp->ep & 0x7F);
	data[29] = chp->wavno;
	data[30] = 0xF7;

	return 0x1F;
}

UINT setMA3Exclusive(BYTE* data, CHPARAM* chp, OPPARAM* opp)
{
	switch(chp->type)
	{
	case VOICE_FM:
		return setExclusiveFMAll(data, chp, opp);
	case VOICE_PCM:
		return setExclusivePCMAll(data, chp, opp);
	}

	return 0;
}

BOOL readMA3Exclusive(CHPARAM* chp, OPPARAM* opp, BYTE* data)
{
	UINT size = GetVVal(data);

	data += GetVValSize(data);

	// command check
	if(	data[0] != 0x43
		|| data[1] != 0x79
		|| data[2] != 0x06
		|| data[3] != 0x7F
		|| data[4] != 0x01
		)
	{
		return TRUE;
	}

	chp->bm = data[5];
	chp->bl = data[6];
	chp->pc = data[7];
	chp->na = data[8];
	chp->type = data[9];

	switch(chp->type)
	{
	case VOICE_FM:
		chp->dk = data[0x0B];
		chp->lfo = ((data[0x0A] & 0x10) ? 2 : 0) | ((data[0x0D] & 0x40) ? 1 : 0);
		chp->pan = ((data[0x0A] & 0x20) ? 0x10 : 0) | ((data[0x0C] >> 3) & 0x0F);
		chp->pe = (data[0x0D] & 0x20) ? TRUE : FALSE;
		chp->alg = data[0x0D] & 0x07;

		{
			int i;
			BYTE* d = data + 0x0A;

			for(i = 0; i < ((chp->alg < 2) ? 2 : 4); i++)
			{
				opp[i].ar = ((d[0] & 0x02) ? 0x08 : 0) | ((d[6] >> 4) & 0x07);
				opp[i].rr = ((d[0] & 0x04) ? 0x08 : 0) | ((d[5] >> 4) & 0x07);
				opp[i].sr = ((d[0] & 0x08) ? 0x08 : 0) | ((d[4] >> 4) & 0x07);
				opp[i].tl = ((d[0] & 0x01) ? 0x20 : 0) | ((d[7] >> 2) & 0x1F);
				opp[i].multi = ((d[8] & 0x20) ? 0x08 : 0) | ((d[0x0A] >> 4) & 0x07);
				opp[i].dt = (d[0x0A]) & 0x07;
				opp[i].dr = (d[5]) & 0x0F;
				opp[i].sl = (d[6]) & 0x0F;
				opp[i].ksl = (d[7]) & 0x03;
				opp[i].dam = (d[9] >> 5) & 0x03;
				opp[i].dvb = (d[9] >> 1) & 0x03;
				opp[i].fb = (d[0x0B]) & 0x07;
				opp[i].ws = ((d[8] & 0x10) ? 0x10 : 0) | ((d[0x0B] >> 3) & 0x0F);

				opp[i].xof = (d[4] & 0x08) ? TRUE : FALSE;
				opp[i].sus = (d[4] & 0x02) ? TRUE : FALSE;
				opp[i].ksr = (d[4] & 0x01) ? TRUE : FALSE;
				opp[i].eam = (d[9] & 0x10) ? TRUE : FALSE;
				opp[i].evb = (d[9] & 0x01) ? TRUE : FALSE;

				d += 8;
			}
		}
		return TRUE;
	case VOICE_PCM:
		chp->fs = ((data[0x0A] & 0x40) ? 0x8000 : 0) | ((data[0x0B] & 0x7F) << 8) | ((data[0x0A] & 0x20) ? 0x80 : 0) | (data[0x0C] & 0x7F);
		chp->lfo = ((data[0x0A] & 0x08) ? 2 : 0) | ((data[0x0E] & 0x40) ? 1 : 0);
		chp->pan = ((data[0x0A] & 0x10) ? 0x10 : 0) | ((data[0x0D] >> 3) & 0x0F);
		chp->pe = (data[0x0D] & 0x01) ? TRUE : FALSE;

		opp->ar = ((data[0x0A] & 0x01) ? 0x08 : 0) | ((data[0x11] >> 4) & 0x07);
		opp->dr = (data[0x10]) & 0x0F;
		opp->sr = ((data[0x0A] & 0x04) ? 0x08 : 0) | ((data[0x0F] >> 4) & 0x07);
		opp->rr = ((data[0x0A] & 0x02) ? 0x08 : 0) | ((data[0x10] >> 4) & 0x07);
		opp->sl = (data[0x11]) & 0x0F;
		opp->tl = ((data[0x12] & 0x40) ? 0x20 : 0) | ((data[0x13] >> 2) & 0x1F);
		opp->dam = (data[0x14] >> 5) & 0x03;
		opp->dvb = (data[0x14] >> 1) & 0x03;
		opp->xof = (data[0x0F] & 0x08) ? TRUE : FALSE;
		opp->sus = (data[0x0F] & 0x02) ? TRUE : FALSE;
		opp->eam = (data[0x14] & 0x10) ? TRUE : FALSE;
		opp->evb = (data[0x14] & 0x01) ? TRUE : FALSE;
		chp->lp = (data[0x17] << 8) | ((data[0x12] & 0x02) ? 0x80 : 0) | (data[0x18]);
		chp->ep = (data[0x19] << 8) | ((data[0x1A] & 0x40) ? 0x80 : 0) | (data[0x1B]);
		chp->wavno = (data[0x1C]) & 0x7F;
		chp->rm = (data[0x1A] & 0x20) ? TRUE : FALSE;
		return TRUE;
	}

	return FALSE;
}

static int readMA5FMParam(OPPARAM* opp, BYTE* data)
{
	opp->sr = (data[0] >> 4) & 0x0F;
	opp->xof = (data[0] & 0x08) ? TRUE : FALSE;
	opp->sus = (data[0] & 0x02) ? TRUE : FALSE;
	opp->ksr = (data[0] & 0x01) ? TRUE : FALSE;

	opp->rr = (data[1] >> 4) & 0x0F;
	opp->dr = data[1] & 0x0F;

	opp->ar = (data[2] >> 4) & 0x0F;
	opp->sl = data[2] & 0x0F;

	opp->tl = (data[3] >> 2) & 0x3F;
	opp->ksl = data[3] & 0x03;

	opp->dam = (data[4] >> 5) & 0x03;
	opp->eam = (data[4] & 0x10) ? TRUE : FALSE;
	opp->dvb = (data[4] >> 1) & 0x03;
	opp->evb = (data[4] & 0x01) ? TRUE : FALSE;

	opp->multi = (data[5] >> 4) & 0x0F;
	opp->dt = data[5] & 0x07;

	opp->ws = (data[6] >> 3) & 0x1F;
	opp->fb = data[6] & 0x07;

	return 7;
}

static int readMA5PCMParam(OPPARAM* opp, BYTE* data)
{
	opp->sr = (data[0] >> 4) & 0x0F;
	opp->xof = (data[0] & 0x08) ? TRUE : FALSE;
	opp->sus = (data[0] & 0x02) ? TRUE : FALSE;

	opp->rr = (data[1] >> 4) & 0x0F;
	opp->dr = data[1] & 0x0F;

	opp->ar = (data[2] >> 4) & 0x0F;
	opp->sl = data[2] & 0x0F;

	opp->tl = (data[3] >> 2) & 0x3F;

	opp->dam = (data[4] >> 5) & 0x03;
	opp->eam = (data[4] & 0x10) ? TRUE : FALSE;
	opp->dvb = (data[4] >> 1) & 0x03;
	opp->evb = (data[4] & 0x01) ? TRUE : FALSE;

	return 5;
}

BOOL readMA5Exclusive(CHPARAM* chp, OPPARAM* opp, BYTE* exdata)
{
	BYTE* data = exdata + GetVValSize(exdata);
	UINT size = GetVVal(exdata);

	chp->bm = data[5];
	chp->bl = data[6];
	chp->pc = data[7];
	chp->na = data[8];

	if(data[9] == 0x00)	// FM
	{
		chp->alg = data[0x0C] & 0x07;
		chp->lfo = (data[0x0C] >> 6) & 0x03;
		chp->pan = (data[0x0B] >> 2) & 0x3F;
		chp->pe = (data[0x0C] & 0x20) ? TRUE : FALSE;
		chp->dk = data[10];

		chp->type = VOICE_FM;

		readMA5FMParam(&opp[0], data + 13);
		readMA5FMParam(&opp[1], data + 13 + 7);
		if(size == 0x2A)
		{
			readMA5FMParam(&opp[2], data + 13 + 7 + 7);
			readMA5FMParam(&opp[3], data + 13 + 7 + 7 + 7);
		}

		return TRUE;
	}
	if(data[9] == 0x01)	// PCM
	{	// TODO: There are problems?
		chp->lfo = (data[0x0D] >> 6) & 0x03;
		chp->pan = (data[0x0C] >> 3) & 0x1F;
		chp->pe = (data[0x0C] & 0x01) ? TRUE : FALSE;

		chp->fs = SwapWord(data + 0x0A);
		chp->rm = (data[0x19] & 0x80) ? TRUE : FALSE;
		chp->wavno = data[0x19] & 0x7F;
		chp->lp = SwapWord(data + 0x15);
		chp->ep = SwapWord(data + 0x17);

		chp->type = VOICE_PCM;

		readMA5PCMParam(opp, data + 0x0E);

		return TRUE;
	}

	return FALSE;
}
