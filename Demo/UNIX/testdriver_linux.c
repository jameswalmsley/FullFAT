#define _LARGEFILE64_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <sys/types.h>
#include <string.h>

#include <unistd.h> 

#define BLKSIZE 2048

/*
	Standard Linux driver, will read very large Harddrives.
	As long as drive doesn't exceed 2TB.
*/
void test(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam) {
	off64_t address;
	address = (off64_t) sector * 512;
	fseeko64(pParam, address, 0);
	fread(buffer, 512, sectors, pParam);
}


void test_ipodSectRead(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam) {
		FILE *f = pParam;
		int i;
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
