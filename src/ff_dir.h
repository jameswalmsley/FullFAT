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
 *  See EXCEPTIONS.TXT for extra restrictions on the use of FullFAT.         *
 *                                                                           *
 *  Removing the above notice is illegal and will invalidate this license.   *
 *****************************************************************************
 *  See http://worm.me.uk/fullfat for more information.                      *
 *  Or  http://fullfat.googlecode.com/ for latest releases and the wiki.     *
 *****************************************************************************/

/**
 *	@file		ff_dir.h
 *	@author		James Walmsley
 *	@ingroup	DIR
 **/
#ifndef _FF_DIR_H_
#define _FF_DIR_H_

#include "ff_types.h"
#include "ff_config.h"
#include "ff_error.h"
#include "ff_ioman.h"
#include "ff_blk.h"
#include "ff_fat.h"
#include "fat.h"
#include <string.h>

typedef struct {
	
#ifdef FF_LFN_SUPPORT
	FF_T_INT8	FileName[FF_MAX_FILENAME];
#else
	FF_T_INT8	FileName[FF_MAX_FILENAME];
#endif
	FF_T_UINT8	Attrib;
	FF_T_UINT32 Filesize;
	FF_T_UINT32	ObjectCluster;

	//---- Book Keeping for FF_Find Functions
	FF_T_UINT32	CurrentItem;	
	FF_T_UINT32	DirCluster;
	FF_T_UINT32	CurrentCluster;
	FF_T_BOOL	ProcessedLFN;
} FF_DIRENT;

FF_T_SINT8	FF_GetEntry	(FF_IOMAN *pIoman, FF_T_UINT32 nEntry, FF_T_UINT32 DirCluster, FF_DIRENT *pDirent, FF_T_BOOL Deleted);
FF_T_UINT32 FF_FindEntry(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_T_INT8 *name, FF_T_UINT8 pa_Attrib, FF_DIRENT *pDirent);

#endif
