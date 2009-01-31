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
 *
 *	@defgroup	MEMORY	FullFAT Memory Access Routines
 *	@brief		Handles memory access in a portable way.
 *
 *	Provides simple, fast, and portable access to memory routines.
 **/

#include "ff_memory.h"

#ifdef _FF_LITTLE_ENDIAN_
/* This exists because some systems don't have byte access, in this case make this return the byte of interest */
FF_T_UINT8 FF_getChar(FF_T_UINT8 *pBuffer, FF_T_UINT16 offset) {
	return (FF_T_UINT8) (pBuffer[offset]);
}

FF_T_UINT16 FF_getShort(FF_T_UINT8 *pBuffer, FF_T_UINT16 offset) {
	return (FF_T_UINT16) (pBuffer[offset] & 0x00FF) | ((FF_T_UINT16) (pBuffer[offset+1] << 8) & 0xFF00);
}

FF_T_UINT32 FF_getLong(FF_T_UINT8 *pBuffer, FF_T_UINT16 offset) {
	return (FF_T_UINT32) (pBuffer[offset] & 0x000000FF) | ((FF_T_UINT32) (pBuffer[offset+1] << 8) & 0x0000FF00) | ((FF_T_UINT32) (pBuffer[offset+2] << 16) & 0x00FF0000) | ((FF_T_UINT32) (pBuffer[offset+3] << 24) & 0xFF000000);
}

#endif

#ifdef _FF_BIG_ENDIAN_

FF_T_UINT8 FF_getChar(FF_T_UINT8 *pBuffer, FF_T_UINT16 offset) {
	return (FF_T_UINT8) (pBuffer[offset]);
}

FF_T_UINT16 FF_getShort(FF_T_UINT8 *pBuffer, FF_T_UINT16 offset) {
	return (FF_T_UINT16) (pBuffer[offset] & 0x00FF) | ((FF_T_UINT16) (pBuffer[offset+1] << 8) & 0xFF00);
}

FF_T_UINT32 FF_getLong(FF_T_UINT8 *pBuffer, FF_T_UINT16 offset) {
	return (FF_T_UINT32) (pBuffer[offset] & 0x000000FF) | ((FF_T_UINT32) (pBuffer[offset+1] << 8) & 0x0000FF00) | ((FF_T_UINT32) (pBuffer[offset+2] << 16) & 0x00FF0000) | ((FF_T_UINT32) (pBuffer[offset+3] << 24) & 0xFF000000);
}

#endif

