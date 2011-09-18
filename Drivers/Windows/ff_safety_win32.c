/*****************************************************************************
 *  FullFAT - High Performance, Thread-Safe Embedded FAT File-System         *
 *                                                                           *
 *  Copyright(C) 2009  James Walmsley  (james@fullfat-fs.co.uk)              *
 *  Many Thanks to     Hein Tibosch    (hein_tibosch@yahoo.es)               *
 *                                                                           *
 *  See RESTRICTIONS.TXT for extra restrictions on the use of FullFAT.       *
 *                                                                           *
 *                FULLFAT IS NOT FREE FOR COMMERCIAL USE                     *
 *                                                                           *
 *  Removing this notice is illegal and will invalidate this license.        *
 *****************************************************************************
 *  See http://www.fullfat-fs.co.uk/ for more information.                   *
 *  Or  http://fullfat.googlecode.com/ for latest releases and the wiki.     *
 *****************************************************************************
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
 *****************************************************************************/

#include "../../src/ff_safety.h"
#include <windows.h>

void *FF_CreateSemaphore(void) {
	HANDLE hSem = CreateSemaphore(NULL, 1, 1, NULL);
	return (void *) hSem;
}

void FF_PendSemaphore(void *pSemaphore) {
	// Call your OS's PendSemaphore with the provided pSemaphore pointer.
	
	HANDLE hSem = (HANDLE) pSemaphore;
	WaitForSingleObject(hSem, INFINITE);	// Take the semaphore, reducing its count to 0.
}

void FF_ReleaseSemaphore(void *pSemaphore) {
	// Call your OS's ReleaseSemaphore with the provided pSemaphore pointer.
	//
	HANDLE hSem = (HANDLE) pSemaphore;
	ReleaseSemaphore(hSem, 1, NULL);	// Reduce Sem count by 1.
}

void FF_DestroySemaphore(void *pSemaphore) {
	// Call your OS's DestroySemaphore with the provided pSemaphore pointer.
	//

	HANDLE hSem = (HANDLE) pSemaphore;
	CloseHandle(hSem);
}

void FF_Yield(void) {
	// Call your OS's thread Yield function.
	// If this doesn't work, then a deadlock will occur
	SwitchToThread();
}

void FF_Sleep(FF_T_UINT32 TimeMs) {
	// Call your OS's thread sleep function,
	// Sleep for TimeMs milliseconds
	Sleep(TimeMs);
}