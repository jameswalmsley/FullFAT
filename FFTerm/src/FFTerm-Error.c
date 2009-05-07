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
 *	@file		ff_error.c
 *	@author		James Walmsley
 *	@ingroup	ERROR
 *
 *	@defgroup	ERR Handles Error Codes
 *	@brief		Used to return pretty strings for FullFAT error codes.
 *
 **/
#include "FFTerm-Types.h"
#include "FFTerm-Error.h"

const struct _FFTERRTAB
{
    const FF_T_INT8 * const strErrorString;
    const FF_T_SINT32 iErrorCode;

} gcpFFTermErrorTable[] =
{
	"Unknown or Generic Error! - Please contact FFTerm DEV - james@worm.me.uk",	-1000,
    "No Error.",															FFT_ERR_NONE,
	"Generic Error - Unknown Cause.",										FFT_ERR_GENERIC,
    "Null Pointer provided, (probably for IOMAN).",							FFT_ERR_NULL_POINTER,
    "Not enough memory (malloc() returned NULL).",							FFT_ERR_NOT_ENOUGH_MEMORY,
	"The length of the provided command name exceeds the allowed limit.",	FFT_ERR_CMD_NAME_TOO_LONG,
	"Command Not Found!",													FFT_ERR_CMD_NOT_FOUND,
	"Command already exists!",												FFT_ERR_CMD_ALREADY_EXISTS
};

/**
 *	@public
 *	@brief	Returns a pointer to a string relating to a FullFAT error code.
 *	
 *	@param	iErrorCode	The error code.
 *
 *	@return	Pointer to a string describing the error.
 *
 **/
const FF_T_INT8 *FFTerm_GetErrMessage( FF_T_SINT32 iErrorCode) {
    FF_T_UINT32 stCount = sizeof (gcpFFTermErrorTable) / sizeof ( struct _FFTERRTAB);
    while (stCount--){
        if (gcpFFTermErrorTable[stCount].iErrorCode == iErrorCode) {
            return gcpFFTermErrorTable[stCount].strErrorString;
        }
    }
	return gcpFFTermErrorTable[0].strErrorString;
}


