#include <stdio.h>
#include <string.h>

#define BLKSIZE 2048

/*
	This driver can read verlarge drives up to 2TB in size.
*/
void test(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam) {
	off64_t address;
	address = (off64_t) sector * 512;
	fseeko64(pParam, address, 0);
	fread(buffer, 512, sectors, pParam);
}

void test_ipodSectRead(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam) {
		FILE *f = pParam;
		unsigned long realSector = sector / (BLKSIZE / 512);
		unsigned char realSectorOffset = sector % (BLKSIZE / 512);
		unsigned long address = realSector * BLKSIZE;
		unsigned char mybuf[4096];
		if(fseek(f, address, 0) != 0){
			//return(-1);
		}
		fread(mybuf, BLKSIZE, sectors, f);
		memcpy(buffer, (mybuf + (512 * realSectorOffset)), 512);
}

/*
	This driver works for devices with blocksizes of 2048, that refuse to function as 512 sized devices.
*/
void test_ipod(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam){
	int i;
	for(i = 0; i < sectors; i++) {
		test_ipodSectRead((buffer + (512 * i)), sector + i, 1, pParam);
	}
}
