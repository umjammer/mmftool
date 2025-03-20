#include <windows.h>
#include "runtime.h"
#include "bit.h"
#include "smaf.h"
#include "emusmw5.h"
#include "exlayer.h"
#include "voice.h"

UINT LoadVoices(VOICE** voicestore, LPCTSTR fn)
{
	BYTE* vm;
	VOICE* voices = NULL;
	UINT voicesmax = 0;

	{
		HANDLE hf;
		DWORD read;

		if((hf = CreateFile(fn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
		{
			MessageBox(NULL, "DefMA3_16.vm3 couldn't open\nIt isn't required, but there are some affections.", "Error", MB_ICONERROR | MB_OK);
			return 0;
		}

		if(!(vm = halloc(GetFileSize(hf, NULL))))
		{
			CloseHandle(hf);
			MessageBox(NULL, "Couldn't allocate memory", "Error", MB_ICONERROR | MB_OK);
			return 0;
		}

		if(!ReadFile(hf, vm, GetFileSize(hf, NULL), &read, NULL))
			read = 0;

		if(read != GetFileSize(hf, NULL))
		{
			hfree(vm);
			CloseHandle(hf);
			MessageBox(NULL, "Couldn't read correctly", "Error", MB_ICONERROR | MB_OK);
			return 0;
		}

		CloseHandle(hf);
	}

	if(COMPARE4(vm, "FMM3"))
	{
		hfree(vm);
		MessageBox(NULL, "This is not FMM3", "Error", MB_ICONERROR | MB_OK);
		return 0;
	}

	{
		BITFILE* bf;
		int cnt, i;
		BYTE* p;
		DWORD size;

		p = vm + 8;
		size = SwapDword(vm + 4);

		for(cnt = 0; (DWORD)(p - vm - 8) < size/* && cnt < 128*/; cnt++)
		{
			BYTE rsize;

			UINT rec;

			rec = voicesmax;

			if(voicesmax < rec + 1)
			{
				voicesmax = rec + 1;
				voices = hrealloc(voices, (voicesmax) * sizeof(VOICE));
			}

			p += 2;	// skip record no
			//もしかして: VVal
			rsize = *p;						p += 1;

			voices[rec].chp.bm = *p;		p += 1;
			voices[rec].chp.bl = *p;		p += 1;
			voices[rec].chp.pc = *p;		p += 1;
			voices[rec].chp.na = *p;	p += 1;

			voices[rec].chp.type = *p;		p += 1;

			memcopy(voices[rec].name, p, 16);
			voices[rec].name[16] = '\0';
			p += 16;

			switch(voices[rec].chp.type)
			{
			case 0:	// FM
				voices[rec].chp.dk = *p;	p += 1;
				rsize -= 1;

				bf = bitopen(p, 55);

				voices[rec].chp.pan = bitnread(bf, 5);
				bitnread(bf, 3);

				voices[rec].chp.lfo = bitnread(bf, 2);
				voices[rec].chp.pe = bitnread(bf, 1);
				bitnread(bf, 2);
				voices[rec].chp.alg = bitnread(bf, 3);

				for(i = 0; i < 4; i++)
				{
					voices[rec].opp[i].sr = bitnread(bf, 4);
					voices[rec].opp[i].xof = bitread(bf);
					bitread(bf);
					voices[rec].opp[i].sus = bitread(bf);
					voices[rec].opp[i].ksr = bitread(bf);

					voices[rec].opp[i].rr = bitnread(bf, 4);
					voices[rec].opp[i].dr = bitnread(bf, 4);

					voices[rec].opp[i].ar = bitnread(bf, 4);
					voices[rec].opp[i].sl = bitnread(bf, 4);

					voices[rec].opp[i].tl = bitnread(bf, 6);
					voices[rec].opp[i].ksl = bitnread(bf, 2);

					voices[rec].opp[i].dam = bitnread(bf, 3);
					voices[rec].opp[i].eam = bitread(bf);
					voices[rec].opp[i].dvb = bitnread(bf, 3);
					voices[rec].opp[i].evb = bitread(bf);

					voices[rec].opp[i].multi = bitnread(bf, 4);
					bitread(bf);
					voices[rec].opp[i].dt = bitnread(bf, 3);

					voices[rec].opp[i].ws = bitnread(bf, 5);
					voices[rec].opp[i].fb = bitnread(bf, 3);
				}

				bitclose(bf);
				break;
			case 1:	// WT([AD]PCM)
				voices[rec].chp.fs = SwapWord(p);	p += 2;
				rsize -= 2;

				bf = bitopen(p, 14);

				voices[rec].chp.pan = bitnread(bf, 5);
				bitnread(bf, 2);
				voices[rec].chp.pe = bitread(bf);

				voices[rec].chp.lfo = bitnread(bf, 2);
				bitnread(bf, 6);

				voices[rec].opp[0].sr = bitnread(bf, 4);
				voices[rec].opp[0].xof = bitread(bf);
				bitread(bf);
				voices[rec].opp[0].sus = bitread(bf);
				bitread(bf);

				voices[rec].opp[0].rr = bitnread(bf, 4);
				voices[rec].opp[0].dr = bitnread(bf, 4);

				voices[rec].opp[0].ar = bitnread(bf, 4);
				voices[rec].opp[0].sl = bitnread(bf, 4);

				voices[rec].opp[0].tl = bitnread(bf, 6);
				bitnread(bf, 2);

				bitread(bf);
				voices[rec].opp[0].dam = bitnread(bf, 2);
				voices[rec].opp[0].eam = bitread(bf);
				bitread(bf);
				voices[rec].opp[0].dvb = bitnread(bf, 2);
				voices[rec].opp[0].evb = bitread(bf);

				bitnread(bf, 16);

				voices[rec].chp.lp = bitnread(bf, 16);
				voices[rec].chp.ep = bitnread(bf, 16);

				voices[rec].chp.rm = bitread(bf);
				voices[rec].chp.wavno = bitnread(bf, 7);

				bitclose(bf);

				break;
			}

			p += rsize - 5 - 16;
		}
	}

	hfree(vm);

	*voicestore = voices;

	return voicesmax;
}

BOOL setVoice(UINT rec, BOOL forcelsb, UINT lsb, BOOL forcepch, UINT pch, VOICE* voices)
{
	BYTE cmd[0x30] = {0};
	UINT cmdsize;

	CHPARAM chp = voices[rec].chp;
	OPPARAM opp[4];

	opp[0] = voices[rec].opp[0];
	opp[1] = voices[rec].opp[1];
	opp[2] = voices[rec].opp[2];
	opp[3] = voices[rec].opp[3];

	//chp.bm = 124;
	//chp.bl = banklsb;
	if(forcelsb)
		chp.bl = lsb;
	if(forcepch)
		chp.pc = pch;

	if(chp.type == VOICE_FM || chp.type == VOICE_PCM)
		cmdsize = setMA3Exclusive(cmd, &chp, opp);

	if(EmuSetMidiMsg(cmd, cmdsize))
	{
		TCHAR ts[256];

		wsprintf(ts, "PresetVoices EmuSetMidiMsg(Assign) failed at Bank%d:%d, PC%d, Note%d", chp.bm, chp.bl, chp.pc, chp.na);
		MessageBox(NULL, ts, "Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	return TRUE;
}

BOOL PresetVoices(VOICE* voices, UINT voicesmax)
{
	UINT i;

	// normal set
	for(i = 0; i < voicesmax; i++)
	{
		int rec = i;
		/*
		//設定リセット?必要?
		{
			BYTE cmd[0x0B] = {0xF0, 0x43, 0x79, 0x06, 0x7F, 0x02, voices[rec].chp.bm, voices[rec].chp.bl, voices[rec].chp.pc, voices[rec].chp.mapnote, 0xF7};

			if(EmuSetMidiMsg(cmd, 0x0B))
			{
				TCHAR ts[256];

				wsprintf(ts, "PresetVoices EmuSetMidiMsg(Erase) failed at Bank%d:%d, PC%d, Note%d", voices[rec].chp.bm, voices[rec].chp.bl, voices[rec].chp.pc, voices[rec].chp.mapnote);
				MessageBox(NULL, ts, "Error", MB_ICONERROR | MB_OK);
				return;
			}
		}
		*/

		//*
		if(voices[rec].chp.bm == 124 && voices[rec].chp.bl == 0)
		{
			int j;

			for(j = 0; j < 10; j++)
				if(!setVoice(rec, TRUE, j, FALSE, 0, voices))
					return FALSE;
		}else
		//*/
		if(voices[rec].chp.bm == 125 && voices[rec].chp.bl == 0)
		{
			int j;

			setVoice(rec, FALSE, 0, FALSE, 0, voices);
			for(j = 2; j < 10; j++)
				if(!setVoice(rec, FALSE, 0, TRUE, j, voices))
					return FALSE;
		}
		else
			if(!setVoice(rec, FALSE, 0, FALSE, 0, voices))
				return FALSE;
	}

	return TRUE;
}
