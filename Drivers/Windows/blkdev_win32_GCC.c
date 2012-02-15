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

#include "blkdev_win32_GCC.h"

/*
	This driver can inteface with Hard drives up to 2TB in size.
*/

/*
	FullFAT assumes that every read and write call is completed in a multiple transaction safe manor.

	That is that multiple concurrent calls to the low-level I/O layer should be threadsafe.
	These file I/O drivers require a MUTEX or Semaphore to achieve this.
*/


struct _DEV_INFO {
	HANDLE		hDev;
	HANDLE		AccessSem;
	long long	DiskSize;
	FF_T_UINT32	BlockSize;
};

static BOOL GetDriveGeometry(DISK_GEOMETRY_EX *pdg, HANDLE hDevice) {
  BOOL bResult;                 // results flag
  DWORD junk;                   // discard results

  if (hDevice == INVALID_HANDLE_VALUE) // cannot open the drive
  {
    return (FALSE);
  }

  bResult = DeviceIoControl(hDevice,  // device to be queried
      IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,  // operation to perform
                             NULL, 0, // no input buffer
                            pdg, sizeof(*pdg),     // output buffer
                            &junk,                 // # bytes returned
                            (LPOVERLAPPED) NULL);  // synchronous I/O

  return (bResult);
}


FF_T_UINT16 GetBlockSize(HANDLE hDevice) {
	struct _DEV_INFO *ptDevInfo = (struct _DEV_INFO *) hDevice;

	if(hDevice) {
		return (FF_T_UINT16) ptDevInfo->BlockSize;
	}

	return 0;
}


HANDLE fnOpen(char *strDevName, int nBlockSize) {
	
	struct _DEV_INFO *ptDevInfo;
	DISK_GEOMETRY_EX DiskGeo;
	LARGE_INTEGER	li, address;

	HANDLE hDisk;

	hDisk = CreateFile(strDevName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING , 0, NULL);

	if(hDisk) {
		ptDevInfo				= (struct _DEV_INFO *) malloc(sizeof(struct _DEV_INFO));		
		if(GetDriveGeometry(&DiskGeo, hDisk)) {
			ptDevInfo->BlockSize	= DiskGeo.Geometry.BytesPerSector;
			ptDevInfo->DiskSize		= DiskGeo.DiskSize.QuadPart;
			ptDevInfo->hDev			= hDisk;
			ptDevInfo->AccessSem	= FF_CreateSemaphore();

			return (HANDLE) ptDevInfo;
		} else {
			//GetFileSizeEx(hDisk, &li);
			address.QuadPart = 0;
			if(!nBlockSize) {
				ptDevInfo->BlockSize	= 512;
			} else {
				ptDevInfo->BlockSize	= nBlockSize;
			}
			ptDevInfo->DiskSize		= li.QuadPart;
			ptDevInfo->hDev			= hDisk;
			ptDevInfo->AccessSem	= FF_CreateSemaphore();
			return (HANDLE) ptDevInfo;
		}
	}

	return (HANDLE) NULL;
}

void fnClose(HANDLE hDevice) {
	struct _DEV_INFO *ptDevInfo = (struct _DEV_INFO *) hDevice;

	if(hDevice) {
		FF_DestroySemaphore(ptDevInfo->AccessSem);
		CloseHandle(ptDevInfo->hDev);
		free(ptDevInfo);
	}
}

signed int fnRead(unsigned char *buffer, unsigned long sector, unsigned short sectors, HANDLE hDevice) {
	struct _DEV_INFO *ptDevInfo = (struct _DEV_INFO *) hDevice;
	LARGE_INTEGER address;
	DWORD	Read;
	//unsigned long long address;
	//unsigned long retVal;
	
	address.QuadPart = (unsigned long long) sector * ptDevInfo->BlockSize;

	FF_PendSemaphore(ptDevInfo->AccessSem);
	{
		//_fseeki64(pParam, address, SEEK_SET);
		SetFilePointerEx(ptDevInfo->hDev, address, NULL, FILE_BEGIN);
		//retVal = fread(buffer, 512, sectors, pParam);
		ReadFile(ptDevInfo->hDev, buffer, ptDevInfo->BlockSize * sectors, &Read, NULL);
	}
	FF_ReleaseSemaphore(ptDevInfo->AccessSem);
	return Read;
}

signed int fnWrite(unsigned char *buffer, unsigned long sector, unsigned short sectors, HANDLE hDevice) {
	struct _DEV_INFO *ptDevInfo = (struct _DEV_INFO *) hDevice;
	LARGE_INTEGER address;
	DWORD	Written;
	//unsigned long retVal;

	address.QuadPart = (unsigned long long) sector * ptDevInfo->BlockSize;
	
	FF_PendSemaphore(ptDevInfo->AccessSem);
	{
		//_fseeki64(pParam, address, SEEK_SET);
		SetFilePointerEx(ptDevInfo->hDev, address, NULL, FILE_BEGIN);
		//retVal = fwrite(buffer, BLOCK_SIZE, sectors, pParam);
		WriteFile(ptDevInfo->hDev, buffer, ptDevInfo->BlockSize * sectors, &Written, NULL);
	}
	FF_ReleaseSemaphore(ptDevInfo->AccessSem);
	return Written;
}

