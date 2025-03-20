typedef struct
{
	BYTE* data;
	DWORD size;
	DWORD byteofs;
	BYTE byte;
	DWORD bitofs;
}BITFILE;

BITFILE* bitopen(BYTE* data, DWORD size);
int bitread(BITFILE* bf);
int bitnread(BITFILE* bf, int n);
void bitclose(BITFILE* bf);

BOOL huffmanDecode(BYTE* dst, BYTE* src, DWORD dstsize, DWORD srcsize);
