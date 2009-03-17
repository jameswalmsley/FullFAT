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

/**
 *	@file		ff_memory.c
 *	@author		James Walmsley
 *	@ingroup	MEMORY
 *
 *	@defgroup	MEMORY	FullFAT Memory Access Routines
 *	@brief		Handles memory access in a portable way.
 *
 *	Provides simple, fast, and portable access to memory routines.
 *	These are only used to read data from buffers. That are LITTLE ENDIAN
 *	due to the FAT specification.
 *
 *	These routines may need to be modified to your platform.
 *
 **/

#include "ff_memory.h"
#include <string.h>

#ifdef FF_LITTLE_ENDIAN

/**
 *	@public
 *	@brief	8 bit memory access routines. 
 **/
FF_T_UINT8 FF_getChar(FF_T_UINT8 *pBuffer, FF_T_UINT16 offset) {
	return (FF_T_UINT8) (pBuffer[offset]);
}

FF_T_UINT16 FF_getShort(FF_T_UINT8 *pBuffer, FF_T_UINT16 offset) {
	return (FF_T_UINT16) (pBuffer[offset] & 0x00FF) | ((FF_T_UINT16) (pBuffer[offset+1] << 8) & 0xFF00);
}

FF_T_UINT32 FF_getLong(FF_T_UINT8 *pBuffer, FF_T_UINT16 offset) {
	return (FF_T_UINT32) (pBuffer[offset] & 0x000000FF) | ((FF_T_UINT32) (pBuffer[offset+1] << 8) & 0x0000FF00) | ((FF_T_UINT32) (pBuffer[offset+2] << 16) & 0x00FF0000) | ((FF_T_UINT32) (pBuffer[offset+3] << 24) & 0xFF000000);
}

void FF_putChar(FF_T_UINT8 *pBuffer, FF_T_UINT16 offset, FF_T_UINT8 Value) {
	pBuffer[offset] = Value;
}

void FF_putShort(FF_T_UINT8 *pBuffer, FF_T_UINT16 offset, FF_T_UINT16 Value) {
	FF_T_UINT8 *Val		= (FF_T_UINT8 *) &Value;
	pBuffer[offset]		= Val[0];
	pBuffer[offset + 1] = Val[1];
}

void FF_putLong(FF_T_UINT8 *pBuffer, FF_T_UINT16 offset, FF_T_UINT32 Value) {
	FF_T_UINT8 *Val		= (FF_T_UINT8 *) &Value;
	pBuffer[offset]		= Val[0];
	pBuffer[offset + 1] = Val[1];
	pBuffer[offset + 2] = Val[2];
	pBuffer[offset + 3] = Val[3];
}

#endif

#ifdef FF_BIG_ENDIAN
/*
	These haven't been tested or checked. They should work in theory :)
	Please contact james@worm.me.uk if they don't work, and also any fix.
*/
FF_T_UINT8 FF_getChar(FF_T_UINT8 *pBuffer, FF_T_UINT16 offset) {
	return (FF_T_UINT8) (pBuffer[offset]);
}

FF_T_UINT16 FF_getShort(FF_T_UINT8 *pBuffer, FF_T_UINT16 offset) {
	return (FF_T_UINT16) ((pBuffer[offset] & 0xFF00)  << 8) | ((FF_T_UINT16) (pBuffer[offset+1]) & 0x00FF);
}

FF_T_UINT32 FF_getLong(FF_T_UINT8 *pBuffer, FF_T_UINT16 offset) {
	return (FF_T_UINT32) ((pBuffer[offset] << 24) & 0xFF0000) | ((FF_T_UINT32) (pBuffer[offset+1] << 16) & 0x00FF0000) | ((FF_T_UINT32) (pBuffer[offset+2] << 8) & 0x0000FF00) | ((FF_T_UINT32) (pBuffer[offset+3]) & 0x000000FF);
}

#endif

/*
 *	These will eventually be moved into a platform independent string
 *	library. Which will be optional. (To allow the use of system specific versions).
 */

//strtok() is not threadsafe, here's one that is! (but it works differently!!)
// returns the iteration value that should passed into the iteration arg
// 0 means its finished!
// Token is a buffer, of the maxlength of a token!

/**
 *	@private
 **/
void FF_tolower(FF_T_INT8 *string, FF_T_UINT32 strLen) {
	FF_T_UINT32 i;
	for(i = 0; i < strLen; i++) {
		if(string[i] >= 'A' && string[i] <= 'Z')
			string[i] += 32;
		if(string[i] == '\0') 
			break;
	}
}


FF_T_INT8 *FF_strtok(FF_T_INT8 *string, FF_T_INT8 *token, FF_T_UINT16 *tokenNumber, FF_T_BOOL *last, FF_T_UINT16 Length) {
	FF_T_UINT16 strLen = Length;
	FF_T_UINT16 i,y, tokenStart, tokenEnd = 0;

	i = 0;
	y = 0;

	if(string[i] == '\\' || string[i] == '/') {
		i++;
	}

	tokenStart = i;

	while(i < strLen) {
		if(string[i] == '\\' || string[i] == '/') {
			y++;
			if(y == *tokenNumber) {
				tokenStart = (FF_T_UINT16) (i + 1);
			}
			if(y == (*tokenNumber + 1)) {
				tokenEnd = i;
				break;
			}
		}
		i++;
	}

	if(!tokenEnd) {
		if(*last == FF_TRUE) {
			return NULL;
		} else {
			*last = FF_TRUE;
		}
		tokenEnd = i;
	}
	
	memcpy(token, (string + tokenStart), (FF_T_UINT32)(tokenEnd - tokenStart));
	token[tokenEnd - tokenStart] = '\0';
	*tokenNumber += 1;

	return token;	
}

