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
 *	@brief		Provides critical regions, and thread-safety for FullFAT.
 *
 *	This module aims to be as portable as possible. It is necessary to modify
 *	the functions FF_EnterCriticalRegion() and FF_ExitCriticalRegion() as
 *	appropriate for your platform.
 **/

#include "ff_safety.h"	// Íncludes ff_types.h

/**
 *	Critical Regions
 *	
 *	These 2 functions enter and exit critical regions of code. This means
 *	that the underlying OS should prevent:
 *
 *	Context Switching (via Process/Thread switching) in an OS Kernel.
 *	Interrupts should be disabled to achieve this.
 *
 *	Upon FF_ExitCriticalRegion() Context switching and Interrupts should
 *	be re-enabled.
 *
 *	Essentially critical regions should ensure that what happens between
 *	these 2 functions is completely atomic.
 *
 *	Because we can guarantee this, FullFAT can create its own semaphore
 *	system without having to worry about portability issues.
 *
 *	These functions should not fail. EnterCriticalRegions should
 *	block the program flow, until a critical region can be provided.
 *
 *	FullFAT will only enter a critical regions for atomic memory
 *	operations. Code between these regions is always 100% non-blocking
 *	and very fast. It's also limited to only 2 C statements.
 *
 *	This ensures that your OS Kernel, or system will not be compromised
 *	in terms of performance.
 **/

void FF_EnterCriticalRegion(void) {
	// Disable OS Kernel Context switching, if applicable.

	// Disable Intterupts.
}

void FF_ExitCriticalRegion(void) {
	// Enable OS Kernel Context switching, if applicable.

	// Disable Interrupts.
}

/**
 *	DUAL-CORE Systems
 *	You may have to do extra work in the above code to ensure 100% safety
 *	in a multi-core system. 
 *
 *	Just ensure that EnterCritcalRegion ensures atomic operations on your
 *	platform!
 **/