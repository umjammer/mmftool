typedef struct{
	WORD	major;
	WORD	minor;
	WORD	patch;
	WORD	build;
}VERSION;

BOOL getFileVersion(VERSION* v, LPCTSTR modname);
BOOL getVersion(VERSION* v);
