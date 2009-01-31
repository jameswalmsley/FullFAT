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
 *	@file		ff_memory.c
 *	@author		James Walmsley
 *	@ingroup	MEMORY
 **/

#ifndef _FF_MEMORY_H_
#define _FF_MEMORY_H_

#include "ff_config.h"
#include "ff_types.h"

//---------- PROTOTYPES (in order of appearance)

// PUBLIC:

// PRIVATE:
FF_T_UINT8		FF_getChar	(FF_T_UINT8 *pBuffer, FF_T_UINT16 offset);
FF_T_UINT16		FF_getShort	(FF_T_UINT8 *pBuffer, FF_T_UINT16 offset);
FF_T_UINT32		FF_getLong	(FF_T_UINT8 *pBuffer, FF_T_UINT16 offset);

#endif
