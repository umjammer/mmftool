#include <windows.h>
#include "runtime.h"
#include "bit.h"

BITFILE* bitopen(BYTE* data, DWORD size)
{
	BITFILE* bf;

	bf = halloc(sizeof(BITFILE));

	bf->data = data;
	bf->size = size;
	bf->byteofs = -1;
	bf->bitofs = 8;

	return bf;
}

int bitread(BITFILE* bf)
{
	int bit;

	if(bf->bitofs == 8)
	{
		if(bf->byteofs + 1 >= bf->size)
		{
			MessageBox(NULL, "bitread: Over size...It may be bug.\nPlease contact to manufacturer.", "Fatal Error", MB_OK);
			return -1;
		}
		bf->byteofs++;
		bf->bitofs = 0;
		bf->byte = bf->data[bf->byteofs];
	}

	bit = (bf->byte >> (7 - bf->bitofs)) & 0x01;
	bf->bitofs++;

	return bit;
}

int bitnread(BITFILE* bf, int n)
{
	int bits = 0;
	int i;

	for(i = 0; i < n; i++)
	{
		int bit;

		if((bit = bitread(bf)) == -1)
			return 0;
		bits = (bits << 1) | bit;
	}

	return bits;
}

void bitclose(BITFILE* bf)
{
	hfree(bf);
}

#define N 256	// chars
static int left[2*N-1], right[2*N-1];

int readtree(BITFILE* bf, BOOL init)
{
	int i;
	static int avail;
	int b;

	if(init)
		avail = N;

	b = bitread(bf);
	if(b == -1)
		return -1;

	if(b)	// 1=fushi
	{
		if((i = avail++) >= 2*N-1)
		{
			TCHAR ts[64];

			wsprintf(ts, "Incorrect tree\nByte %u, Bit %u", bf->byteofs, bf->bitofs);

			MessageBox(NULL, ts, "Error", MB_ICONERROR | MB_OK);
			return -1;
		}
		if((left[i] = readtree(bf, FALSE)) == -1)
			return -1;
		if((right[i] = readtree(bf, FALSE)) == -1)
			return -1;
		return i;
	}else
		return bitnread(bf, 8);
}

BOOL huffmanDecode(BYTE* dst, BYTE* src, DWORD dstsize, DWORD srcsize)
{
	BITFILE* bf;
	int root, j;
	DWORD k;

	bf = bitopen(src, srcsize);

	// read tree
	if((root = readtree(bf, TRUE)) == -1)
	{
		bitclose(bf);
		return FALSE;
	}

	// decode
	for(k = 0; k < dstsize; k++)
	{
		j = root;
		while(j >= N)
		{
			int b = bitread(bf);
			if(b == -1)
			{
				bitclose(bf);
				return FALSE;
			}
			if(b)
				j = right[j];
			else
				j = left[j];
		}

		*dst++ = j;
	}

	bitclose(bf);

	return TRUE;
}
