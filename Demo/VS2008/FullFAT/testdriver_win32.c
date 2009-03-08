#include <stdio.h>
#include <string.h>

#define BLKSIZE 512

void test(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam) {
//	FILE *f = fopen((char *) pParam, "rb");
	int i = 0;
	unsigned int address;
	address = sector * 512;
	fseek(pParam, address, 0);
	fread(buffer, BLKSIZE, sectors, pParam);
	//fclose(f);
}

void test_ipod(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam) {

	FILE *f = fopen((char *) pParam, "rb");

	unsigned long realSector = sector / (BLKSIZE / 512);
	unsigned char realSectorOffset = sector % (BLKSIZE / 512);
	unsigned long address = realSector * BLKSIZE;
	unsigned char mybuf[4096];

	if(fseek(f, address, 0) != 0){
		//return(-1);
	}

	fread(mybuf, BLKSIZE, sectors, f);

	fclose(f);

	memcpy(buffer, (mybuf + (512 * realSectorOffset)), 512);
	
}