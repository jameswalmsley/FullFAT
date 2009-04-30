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
 *	@ingroup	DIR
 *
 *	@defgroup	DIR Handles Directory Traversal
 *	@brief		Handles DIR access and traversal.
 *
 *	Provides FindFirst() and FindNext() Interfaces
 **/
#include "ff_config.h"
#include "ff_types.h"

#ifdef FF_DEBUG
const struct _FFERRTAB
{
    const FF_T_INT8 * const strErrorString;
    const FF_T_SINT32 iErrorCode;

} gcpFullFATErrorTable[] =
{
	"Error Code Not Found",					-1000,
    "FF_ERR_NONE",                          0,
    "FF_ERR_NULL_POINTER",                  -2,
    "FF_ERR_NOT_ENOUGH_MEMORY",             -3,
    "FF_ERR_DEVICE_DRIVER_FAILED",          -4,
    "FF_ERR_IOMAN_BAD_BLKSIZE",             -11,
    "FF_ERR_IOMAN_BAD_MEMSIZE",             -12,
    "FF_ERR_IOMAN_DEV_ALREADY_REGD",        -11,
    "FF_ERR_IOMAN_NO_MOUNTABLE_PARTITION",  -12,
    "FF_ERR_IOMAN_INVALID_FORMAT",          -13,
    "FF_ERR_IOMAN_INVALID_PARTITION_NUM",   -14,
    "FF_ERR_IOMAN_NOT_FAT_FORMATTED",       -15,
    "FF_ERR_IOMAN_DEV_INVALID_BLKSIZE",     -16,
    "FF_ERR_IOMAN_PARTITION_MOUNTED",       -18,
    "FF_ERR_IOMAN_ACTIVE_HANDLES",          -19,
    "FF_ERR_FILE_ALREADY_OPEN",             -30,
    "FF_ERR_FILE_NOT_FOUND",                -31,
    "FF_ERR_FILE_OBJECT_IS_A_DIR",          -32,
    "FF_ERR_FILE_IS_READ_ONLY",             -33,
    "FF_ERR_FILE_INVALID_PATH",             -34,
    "FF_ERR_DIR_OBJECT_EXISTS",             -50,
    "FF_ERR_DIR_DIRECTORY_FULL",            -51,
    "FF_ERR_DIR_END_OF_DIR",                -52,
    "FF_ERR_DIR_NOT_EMPTY",                 -53
};

/**********************************************************************************
Function Name: cmdGetFullFatErrorString
Description:   Function to get an error string for a fullFAT error code
Parameters:    IN  iErrorCode - The error code
Return value:  Pointer to the string
**********************************************************************************/
const FF_T_INT8 *FF_GetErrMessage( FF_T_SINT32 iErrorCode)
{
    //static const char * const pszNotFount = "Error not found";
    FF_T_UINT32 stCount = sizeof (gcpFullFATErrorTable) / sizeof ( struct _FFERRTAB);
    while (stCount--)
    {
        if (gcpFullFATErrorTable[stCount].iErrorCode == iErrorCode)
        {
            return gcpFullFATErrorTable[stCount].strErrorString;
        }
    }
	return gcpFullFATErrorTable[0].strErrorString;
}
#endif
