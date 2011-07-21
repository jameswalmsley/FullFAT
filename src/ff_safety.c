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

/**
 *	@file		ff_safety.c
 *	@author		James Walmsley
 *	@ingroup	SAFETY
 *
 *	@defgroup	SAFETY	Process Safety for FullFAT
 *	@brief		Provides semaphores, and thread-safety for FullFAT.
 *
 *	This module aims to be as portable as possible. It is necessary to modify
 *	the functions FF_CreateSemaphore, FF_PendSemaphore, FF_ReleaseSemaphore,
 *  and FF_DestroySemaphore, as appropriate for your platform.
 *
 *	If your application has no OS and is therefore single threaded, simply
 *	have:
 *
 *	FF_CreateSemaphore() return NULL.
 *
 *	FF_PendSemaphore() should do nothing.
 *
 *	FF_ReleaseSemaphore() should do nothing.
 *
 *	FF_DestroySemaphore() should do nothing.
 *
 **/

#include "ff_safety.h"	// Íncludes ff_types.h

void *FF_CreateSemaphore(void) {
	// Call your OS's CreateSemaphore function
	//

	// return pointer to semaphore
	return NULL;	// Comment this out for your implementation.
}

void FF_PendSemaphore(void *pSemaphore) {
	// Call your OS's PendSemaphore with the provided pSemaphore pointer.
	//
	// This should block indefinitely until the Semaphore
	// becomes available. (No timeout!)
	// If your OS doesn't do it for you, you should sleep
	// this thread until the Semaphore is available.
	pSemaphore = 0;
}

void FF_ReleaseSemaphore(void *pSemaphore) {
	// Call your OS's ReleaseSemaphore with the provided pSemaphore pointer.
	//

	//
	pSemaphore = 0;
}

void FF_DestroySemaphore(void *pSemaphore) {
	// Call your OS's DestroySemaphore with the provided pSemaphore pointer.
	//

	//
	pSemaphore = 0;
}

void FF_Yield(void) {
	// Call your OS's thread Yield function.
	// If this doesn't work, then a deadlock will occur
}

void FF_Sleep(FF_T_UINT32 TimeMs) {
	// Call your OS's thread sleep function,
	// Sleep for TimeMs milliseconds
	TimeMs = 0;
}


/**
 *	Notes on implementation.
 *
 *
 *
 **/


