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

#include "blkdev_win32.h"

/*
	This driver can inteface with Hard drives up to 2TB in size.
*/

/*
	FullFAT assumes that every read and write call is completed in a multiple transaction safe manor.

	That is that multiple concurrent calls to the low-level I/O layer should be threadsafe.
	These file I/O drivers require a MUTEX or Semaphore to achieve this.
*/
static HANDLE g_Sem = NULL;	// I/O Access Semaphore!

signed int fnRead(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam) {
	unsigned long long address;
	unsigned long retVal;
	
	if(!g_Sem) {
		g_Sem = FF_CreateSemaphore();
	}
	
	address = (unsigned long long) sector * 512;

	FF_PendSemaphore(g_Sem);
	{
		_fseeki64(pParam, address, SEEK_SET);
		retVal = fread(buffer, 512, sectors, pParam);
	}
	FF_ReleaseSemaphore(g_Sem);
	return retVal;
}

signed int fnWrite(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam) {
	unsigned long long address;
	unsigned long retVal;

	if(!g_Sem) {
		g_Sem = FF_CreateSemaphore();
	}

	address = (unsigned long long) sector * BLOCK_SIZE;
	
	FF_PendSemaphore(g_Sem);
	{
		_fseeki64(pParam, address, SEEK_SET);
		retVal = fwrite(buffer, BLOCK_SIZE, sectors, pParam);
	}
	FF_ReleaseSemaphore(g_Sem);
	return retVal;
}

