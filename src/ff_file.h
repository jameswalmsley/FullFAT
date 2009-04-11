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
 *	@file		ff_file.h
 *	@author		James Walmsley
 *	@ingroup	FILEIO
 **/
#ifndef _FF_FILE_H_
#define _FF_FILE_H_

#include "ff_config.h"
#include "ff_types.h"
#include "ff_ioman.h"
#include "ff_dir.h"

#define FF_SEEK_SET	1
#define FF_SEEK_CUR	2
#define FF_SEEK_END	3

typedef struct _FF_FILE {
	FF_IOMAN	*pIoman;		///< Ioman Pointer!
	FF_T_UINT32 Filesize;		///< File's Size.
	FF_T_UINT32 ObjectCluster;	///< File's Start Cluster.
	FF_T_UINT32 FilePointer;	///< Current Position Pointer.
	FF_T_UINT8	Mode;			///< Mode that File Was opened in.
	FF_T_UINT32	CurrentCluster;	///< Prevents FAT Thrashing
	FF_T_UINT32 AddrCurrentCluster;
	FF_T_UINT32 DirCluster;		///< Cluster Number that the Dirent is in.
	FF_T_UINT32 DirEntry;		///< Dirent Entry Number describing this file.
	struct _FF_FILE *Next;
} FF_FILE,
*PFF_FILE;

//---------- PROTOTYPES
// PUBLIC (Interfaces):
FF_T_SINT8	 FF_FindFirst	(FF_IOMAN *pIoman, FF_DIRENT *pDirent, FF_T_INT8 *path);
FF_T_SINT8	 FF_FindNext	(FF_IOMAN *pIoman, FF_DIRENT *pDirent);
FF_T_UINT32	 FF_FindDir		(FF_IOMAN *pIoman, FF_T_INT8 *path, FF_T_UINT16 pathLen);

FF_FILE		*FF_Open	(FF_IOMAN *pIoman, FF_T_INT8 *path, FF_T_UINT8 Mode, FF_T_SINT8 *pError);
FF_T_SINT8	 FF_Close	(FF_FILE *pFile);
FF_T_INT32	 FF_GetC	(FF_FILE *pFile);
FF_T_UINT32	 FF_Read	(FF_FILE *pFile, FF_T_UINT32 ElementSize, FF_T_UINT32 Count, FF_T_UINT8 *buffer);
FF_T_BOOL	 FF_isEOF	(FF_FILE *pFile);
FF_T_SINT8	 FF_Seek	(FF_FILE *pFile, FF_T_SINT32 Offset, FF_T_INT8 Origin);
FF_T_SINT8	 FF_PutC    (FF_FILE *pFile, FF_T_UINT8 Value);

// Private :

#endif

