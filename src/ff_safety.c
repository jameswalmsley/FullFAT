/******************************************************************************
 *   FullFAT - Embedded FAT File-System
 *
 *   Provides a full, thread-safe, implementation of the FAT file-system
 *   suitable for low-power embedded systems.
 *
 *   Written by James Walmsley, james@worm.me.uk, www.worm.me.uk/fullfat/
 *
 *   Copyright 2009 James Walmsley
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 *   Commercial support is available for FullFAT, for more information
 *   please contact the author, james@worm.me.uk
 *
 *   Removing the above notice is illegal and will invalidate this license.
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
}

void FF_ReleaseSemaphore(void *pSemaphore) {
	// Call your OS's ReleaseSemaphore with the provided pSemaphore pointer.
	//

	//
}

void FF_DestroySemaphore(void *pSemaphore) {
	// Call your OS's DestroySemaphore with the provided pSemaphore pointer.
	//

	//
}

void FF_Yield(void) {
	// Call your OS's thread Yield function. 
	// If this doesn't work, then a deadlock will occur	
}

/**
 *	Notes on implementation.
 *
 *		
 *
 **/
 
