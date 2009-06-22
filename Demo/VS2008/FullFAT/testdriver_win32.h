
#include <Windows.h>
#include <stdio.h>
#include "../../../src/fullfat.h"			// Include everything required for FullFAT.

struct _IO_SEM {
	HANDLE	Sem;
	FILE	*pFile;
};

typedef struct _IO_SEM DEVIO;

signed int fnRead_512		(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam);
signed int fnWrite_512		(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam);


signed int fnVistaRead_512	(unsigned char *buffer, unsigned long sector, unsigned short sectors, HANDLE hDev);
signed int fnVistaWrite_512	(unsigned char *buffer, unsigned long sector, unsigned short sectors, HANDLE hDev);

signed int fnNewRead_512	(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam);
signed int fnNewWrite_512	(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam);
signed int test_2048		(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam);
