/*****************************************************************************
 *  FullFAT - High Performance, Thread-Safe Embedded FAT File-System         *
 *                                                                           *
 *  Copyright(C) 2009 James Walmsley  (james@fullfat-fs.co.uk)               *
 *	Copyright(C) 2010 Hein Tibosch    (hein_tibosch@yahoo.es)                *
 *                                                                           *
 *  See RESTRICTIONS.TXT for extra restrictions on the use of FullFAT.       *
 *                                                                           *
 *  Removing this notice is illegal and will invalidate this license.        *
 *****************************************************************************
 *  See http://www.fullfat-fs.co.uk/ for more information.                   *
 *  Or  http://fullfat.googlecode.com/ for latest releases and the wiki.     *
 *****************************************************************************
 * As of 19-July-2011 FullFAT has abandoned the GNU GPL License in favour of *
 * the more flexible Apache 2.0 license. See License.txt for full terms.     *
 *                                                                           *
 *            YOU ARE FREE TO USE FULLFAT IN COMMERCIAL PROJECTS             *
 *****************************************************************************/

#include "blkdev_win32.h"

/*
	This driver can interface with very Large Disks.
*/

/*
	FullFAT assumes that every read and write call is completed in a multiple transaction safe manor.

	That is that multiple concurrent calls to the low-level I/O layer should be threadsafe.
	These file I/O drivers require a MUTEX or Semaphore to achieve this.

	To further improve this, a special associative cache, can be implemented above this level.
*/

struct _DEV_INFO {
	HANDLE		hDev;		// Handle to a Block Device or File.
	HANDLE		AccessSem;	// Access Semaphore.
	long long	DiskSize;	// Disk Size in Bytes.
	FF_T_UINT32	BlockSize;	// Block Size.
};

/**
 *	@private
 *	@brief		If the device is a Physical Device, it returns the geometry information.
 *
 **/
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

/**
 *	@brief	Gets the BlockSize of an opened device.
 *
 **/
FF_T_UINT16 GetBlockSize(HANDLE hDevice) {
	struct _DEV_INFO *ptDevInfo = (struct _DEV_INFO *) hDevice;

	if(hDevice) {
		return (FF_T_UINT16) ptDevInfo->BlockSize;
	}

	return 0;
}

/**
 *	@brief	Opens a HANDLE to a Windows Blockdevice or File.
 *
 **/
HANDLE fnOpen(char *strDevName, int nBlockSize) {
	
	struct _DEV_INFO	*ptDevInfo;
	DISK_GEOMETRY_EX	DiskGeo;
	LARGE_INTEGER		li, address;

	BOOL				IOError;
	DWORD				BytesReturned;

	HANDLE				hDisk;
	WCHAR				pWide[MAX_PATH];

	MultiByteToWideChar(CP_ACP, 0, strDevName, -1, pWide, MAX_PATH);
	hDisk = CreateFile(pWide, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING , 0, NULL);

	IOError = GetLastError();

	if(hDisk != INVALID_HANDLE_VALUE) {

		// Dismount volume (allow Vista and Seven write access!)
		IOError = DeviceIoControl(hDisk, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &BytesReturned, NULL);

//		if(IOError) {	// Continue on error! This may be an image file!

			ptDevInfo				= (struct _DEV_INFO *) malloc(sizeof(struct _DEV_INFO));		
			if(GetDriveGeometry(&DiskGeo, hDisk)) {
				ptDevInfo->BlockSize	= DiskGeo.Geometry.BytesPerSector;
				ptDevInfo->DiskSize		= DiskGeo.DiskSize.QuadPart;
				ptDevInfo->hDev			= hDisk;
				ptDevInfo->AccessSem	= FF_CreateSemaphore();

				return (HANDLE) ptDevInfo;
			} else {
				GetFileSizeEx(hDisk, &li);
				address.QuadPart = 0;
				if(!nBlockSize) {
					ptDevInfo->BlockSize	= 512;	// Try to assume the most likely setting!
				} else {
					ptDevInfo->BlockSize	= nBlockSize;
				}
				ptDevInfo->DiskSize		= li.QuadPart;
				ptDevInfo->hDev			= hDisk;
				ptDevInfo->AccessSem	= FF_CreateSemaphore();
				return (HANDLE) ptDevInfo;
			}
		//}
	}

	return (HANDLE) NULL;
}

/**
 *	@brief	Closes The Windows Block Device.
 *
 **/
void fnClose(HANDLE hDevice) {
	struct _DEV_INFO *ptDevInfo = (struct _DEV_INFO *) hDevice;

	if(hDevice) {
		FF_DestroySemaphore(ptDevInfo->AccessSem);
		CloseHandle(ptDevInfo->hDev);
		free(ptDevInfo);
	}
}

/**
 *	@brief	Thread-Safe BlockDevice Read operation for Windows.
 *
 **/
signed int fnRead(unsigned char *buffer, unsigned long sector, unsigned short sectors, HANDLE hDevice) {
	struct _DEV_INFO	*ptDevInfo = (struct _DEV_INFO *) hDevice;
	LARGE_INTEGER		address;
	DWORD				Read;
	
	address.QuadPart = (unsigned long long) sector * ptDevInfo->BlockSize;

	FF_PendSemaphore(ptDevInfo->AccessSem);
	{
		SetFilePointerEx(ptDevInfo->hDev, address, NULL, FILE_BEGIN);
		ReadFile(ptDevInfo->hDev, buffer, ptDevInfo->BlockSize * sectors, &Read, NULL);
	}
	FF_ReleaseSemaphore(ptDevInfo->AccessSem);

	return Read / ptDevInfo->BlockSize;
}

/**
 *	@brief	Thread-Safe BlockDevice Write operation for Windows.
 *
 **/
signed int fnWrite(unsigned char *buffer, unsigned long sector, unsigned short sectors, HANDLE hDevice) {
	struct _DEV_INFO	*ptDevInfo = (struct _DEV_INFO *) hDevice;
	LARGE_INTEGER		address;
	DWORD				Written;

	address.QuadPart = (unsigned long long) sector * ptDevInfo->BlockSize;
	
	FF_PendSemaphore(ptDevInfo->AccessSem);
	{
		SetFilePointerEx(ptDevInfo->hDev, address, NULL, FILE_BEGIN);
		WriteFile(ptDevInfo->hDev, buffer, ptDevInfo->BlockSize * sectors, &Written, NULL);
	}
	FF_ReleaseSemaphore(ptDevInfo->AccessSem);

	return Written / ptDevInfo->BlockSize;
}
