#include <Windows.h>
#include <stdio.h>
#include "../../src/fullfat.h"			// Include everything required for FullFAT.

#define BLOCK_SIZE	512

signed int fnRead	(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam);
signed int fnWrite	(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam);

