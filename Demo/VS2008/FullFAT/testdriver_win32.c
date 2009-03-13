#include <stdio.h>
#include <string.h>


/*
	This driver can inteface with Hard drives up to 2TB in size.
*/
void test_512(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam) {
	unsigned long long address;
	address = (unsigned long long) sector * 512;
	_fseeki64(pParam, address, SEEK_SET);
	fread(buffer, 512, sectors, pParam);
}

/*
	This driver works for devices with blocksizes of 2048.
*/
void test_2048(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam){
	unsigned long long address;
	address = (unsigned long long) sector * 2048;
	_fseeki64(pParam, address, SEEK_SET);
	fread(buffer, 2048, sectors, pParam);
}
