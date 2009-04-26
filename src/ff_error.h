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
#ifndef _FF_ERROR_H_
#define _FF_ERROR_H_

/*	FullFAT defines different Error-Code spaces for each module. This ensures
	that all error codes remain unique, and their meaning can be quickly identified.
*/
// Global Error Codes
#define FF_ERR_NONE						 0	///< No Error
#define FF_ERR_NULL_POINTER				-2	///< pIoman was NULL.
#define FF_ERR_NOT_ENOUGH_MEMORY		-3	///< malloc() failed! - Could not allocate handle memory.

// IOMAN Error Codes
#define	FF_ERR_IOMAN_BAD_BLKSIZE		-11	///< The provided blocksize was not a multiple of 512.
#define FF_ERR_IOMAN_BAD_MEMSIZE		-12	///< The memory size was not a multiple of the blocksize.
#define FF_ERR_IOMAN_PARTITION_MOUNTED	-18	///< Device is in use by an actively mounted partition. Unmount the partition first.
#define FF_ERR_IOMAN_ACTIVE_HANDLES		-19 ///< The partition cannot be unmounted until all active file handles are closed.

// File Error Codes						-20 +
#define FF_ERR_FILE_ALREADY_OPEN		-20	///< File is in use.
#define FF_ERR_FILE_NOT_FOUND			-21	///< File was not found.
#define FF_ERR_FILE_OBJECT_IS_A_DIR		-22	///< Tried to FF_Open() a Directory.

// Directory Error Codes				-30 +
#define FF_ERR_DIR_OBJECT_EXISTS		-30	
#define FF_ERR_DIR_DIRECTORY_FULL		-31	///< No more items could be added to the directory.
#define FF_ERR_DIR_END_OF_DIR			-32	
#define FF_ERR_DIR_NOT_EMPTY			-33	///< Cannot delete a directory that contains files or folders.

// Fat Error Codes						-40 +

#endif

