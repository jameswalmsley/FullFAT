#include <stdio.h>
#include <string.h>

#define BLKSIZE 2048

/*
	This driver can read very large drives up to 2TB in size.
*/
signed int test_512(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam) {
	off64_t address;
	address = (off64_t) sector * 512;
	fseeko64(pParam, address, 0);
	fread(buffer, 512, sectors, pParam);
	return sectors;
}


/*
	This driver works for devices with blocksizes of 2048.
*/
signed int test_2048(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam){
	off64_t address;
	address = (off64_t) sector * 2048;
	fseeko64(pParam, address, 0);
	fread(buffer, 2048, sectors, pParam);
	return sectors;
}
