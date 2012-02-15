#include <Windows.h>
#include <stdio.h>
#include "../../src/fullfat.h"			// Include everything required for FullFAT.

#define BLOCK_SIZE	512

FF_T_UINT16 GetBlockSize(HANDLE hDevice);

HANDLE fnOpen(char *strDevName, int nBlockSize);
void fnClose(HANDLE hDevice);
signed int fnRead	(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam);
signed int fnWrite	(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam);

