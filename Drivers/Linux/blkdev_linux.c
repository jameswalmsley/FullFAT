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
 *  See RESTRICTIONS.TXT for extra restrictions on the use of FullFAT.       *
 *                                                                           *
 *  Removing the above notice is illegal and will invalidate this license.   *
 *****************************************************************************
 *  See http://worm.me.uk/fullfat for more information.                      *
 *  Or  http://fullfat.googlecode.com/ for latest releases and the wiki.     *
 *****************************************************************************/

/*
	This driver can interface with very Large Disks.
*/

#define _LARGEFILE64_SOURCE

#include "blkdev_linux.h"
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h> 

struct _DEV_INFO {
	FILE		*pDevice;	// File pointer to the device.
	void 		*AccessSem;
	long long	DiskSize;
	FF_T_UINT32	BlockSize;
};

BLK_DEV_LINUX fnOpen(char *szDeviceName, int nBlockSize) {
	
	BLK_DEV_LINUX	ptDevInfo;

	FILE *pDevice;

	pDevice = fopen(szDeviceName, "rb+");

	if(pDevice) {
		ptDevInfo = (struct _DEV_INFO *) malloc(sizeof(struct _DEV_INFO));

		if(ptDevInfo) {
			ptDevInfo->BlockSize = nBlockSize;
			//ptDevInfo->DiskSize
			ptDevInfo->pDevice = pDevice;
			ptDevInfo->AccessSem = FF_CreateSemaphore();

			return (BLK_DEV_LINUX) ptDevInfo;
		}
	}

	return NULL;
}


void fnClose(BLK_DEV_LINUX pDevice) {
	fclose(pDevice->pDevice);
	FF_DestroySemaphore(pDevice->AccessSem);
	free(pDevice);
}

signed int fnRead(unsigned char *buffer, unsigned long sector, unsigned short sectors, BLK_DEV_LINUX pDevice) {
	unsigned long long address;
	unsigned long	Read;

	address = (unsigned long long) sector * pDevice->BlockSize;

	FF_PendSemaphore(pDevice->AccessSem);
	{
		fseeko64(pDevice->pDevice, address, 0);
		Read = fread(buffer, pDevice->BlockSize, sectors, pDevice->pDevice);
	}
	FF_ReleaseSemaphore(pDevice->AccessSem);

	return Read / pDevice->BlockSize;
}


signed int fnWrite(unsigned char *buffer, unsigned long sector, unsigned short sectors, BLK_DEV_LINUX pDevice) {
	unsigned long long address;
	unsigned long Written;

	address = (unsigned long long) sector * pDevice->BlockSize;

	FF_PendSemaphore(pDevice->AccessSem);
	{
		fseeko64(pDevice->pDevice, address, 0);
		Written = fwrite(buffer, pDevice->BlockSize, sectors, pDevice->pDevice);

	}
	FF_ReleaseSemaphore(pDevice->AccessSem);

	return Written / pDevice->BlockSize;
}

FF_T_UINT16 GetBlockSize(BLK_DEV_LINUX pDevice) {
	return pDevice->BlockSize;
}
