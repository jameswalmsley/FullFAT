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


/*
	Standard Linux driver, will read very large Harddrives.
	As long as drive doesn't exceed 2TB.
*/
signed int test(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam) {
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
