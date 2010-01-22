/*****************************************************************************
 *  FullFAT - High Performance, Thread-Safe Embedded FAT File-System         *
 *  Copyright (C) 2009  James Walmsley (james@worm.me.uk)                    *
 *                                                                           *
 *  This program is free software: you can redistribute it and/or modify     *
 *  it under the terms of the GNU General Public License as published by     *
 *  the Free Software Foundation, either version 3 of the License, or        *
 *  (at your option) any later version.                                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public License        *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *                                                                           *
 *  IMPORTANT NOTICE:                                                        *
 *  =================                                                        *
 *  Alternative Licensing is available directly from the Copyright holder,   *
 *  (James Walmsley). For more information consult LICENSING.TXT to obtain   *
 *  a Commercial license.                                                    *
 *                                                                           *
 *  See EXCEPTIONS.TXT for extra restrictions on the use of FullFAT.         *
 *                                                                           *
 *  Removing the above notice is illegal and will invalidate this license.   *
 *****************************************************************************
 *  See http://worm.me.uk/fullfat for more information.                      *
 *  Or  http://fullfat.googlecode.com/ for latest releases and the wiki.     *
 *****************************************************************************/

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


typedef struct {


} 

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
