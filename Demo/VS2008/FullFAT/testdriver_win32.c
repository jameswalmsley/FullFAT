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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <windows.h>
#include <winbase.h>
#include <winioctl.h>
#include <conio.h>
#include "testdriver_win32.h"

/*
	This driver can inteface with Hard drives up to 2TB in size.
*/

/*
	FullFAT assumes that every read and write call is completed in a multiple transaction safe manor.

	That is that multiple concurrent calls to the low-level I/O layer should be threadsafe.
	These file I/O drivers require a MUTEX or Semaphore to achieve this.
*/
static HANDLE g_Sem = NULL;	// I/O Access Semaphore!

signed int fnVistaRead_512(unsigned char *buffer, unsigned long sector, unsigned short sectors, HANDLE hDev) {
	LARGE_INTEGER li;
	DWORD	ReadBytes;
	DWORD	retVal;
	li.QuadPart = (sector * 512);
	li.LowPart = SetFilePointer(hDev, li.LowPart, &li.HighPart, FILE_BEGIN);
	retVal = ReadFile(hDev, buffer, (DWORD) (sectors * 512), &ReadBytes, NULL);
	if(!retVal) {
		retVal = GetLastError();
	} 

	return (signed int) (ReadBytes / 512);	
}


signed int fnVistaWrite_512(unsigned char *buffer, unsigned long sector, unsigned short sectors, HANDLE hDev) {
	LARGE_INTEGER li;
	DWORD	WrittenBytes;
	DWORD	BytesReturned;
	DWORD	retVal;
	li.QuadPart = (sector * 512);
	li.LowPart = SetFilePointer(hDev, li.LowPart, &li.HighPart, FILE_BEGIN);
	
	retVal = DeviceIoControl(hDev, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &BytesReturned, NULL);
	retVal = DeviceIoControl(hDev, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &BytesReturned, NULL);
	
	retVal = WriteFile(hDev, buffer, (DWORD) (sectors * 512), &WrittenBytes, NULL);
	if(!retVal) {
		retVal = GetLastError();
	} 
	
	retVal = DeviceIoControl(hDev, FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, &BytesReturned, NULL);

	return (signed int) (WrittenBytes / 512);	
}

signed int fnRead_512(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam) {
	unsigned long long address;
	
	if(!g_Sem) {
		g_Sem = FF_CreateSemaphore();
	}
	
	address = (unsigned long long) sector * 512;

	FF_PendSemaphore(g_Sem);
	{
		_fseeki64(pParam, address, SEEK_SET);
		fread(buffer, 512, sectors, pParam);
	}
	FF_ReleaseSemaphore(g_Sem);
	return sectors;
}
/*
signed int fnNewRead_512(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam) {
	unsigned long long address;
	int retVal;
	address = (unsigned long long) sector * 512;
	lseek((int)pParam, address, SEEK_SET);
	retVal = read((int)pParam, buffer, 512);
	return sectors;
}

signed int fnNewWrite_512(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam) {
	unsigned long long address;
	int retVal;
	//printf("W %d %d\n", sector, sectors);
	address = (unsigned long long) sector * 512;
	lseek((int)pParam, address, SEEK_SET);
	retVal = write((int)pParam, buffer, 512);
	return sectors;
}
*/
signed int fnWrite_512(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam) {
	unsigned long long address;
	unsigned long retVal;
	//printf("W %d %d\n", sector, sectors);

	if(sector == 0) {
		printf("Write to MBR!\n");
	}
	address = (unsigned long long) sector * 512;

	if(!g_Sem) {
		g_Sem = FF_CreateSemaphore();
	}
	
	FF_PendSemaphore(g_Sem);
	{

		if(_fseeki64(pParam, address, SEEK_SET)) {
			printf("Seek error\n");
		}
		
		retVal = fwrite(buffer, 512, sectors, pParam);

	}
	FF_ReleaseSemaphore(g_Sem);

	if(retVal != sectors) {
		printf("Write Error!\n");
	}

	return retVal;
}

/*
	This driver works for devices with blocksizes of 2048.
*/
signed int test_2048(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam){
	unsigned long long address;
	address = (unsigned long long) sector * 2048;
	_fseeki64(pParam, address, SEEK_SET);
	fread(buffer, 2048, sectors, pParam);
	return sectors;
}
