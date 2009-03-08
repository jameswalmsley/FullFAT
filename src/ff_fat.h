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
 *	@file		ff_fat.h
 *	@author		James Walmsley
 *	@ingroup	FAT
 **/

#ifndef _FF_FAT_H_
#define _FF_FAT_H_

#include "fat.h"
#include "ff_ioman.h"

typedef struct {
	FF_IOMAN	*pIoman;		///< Ioman Pointer!
	FF_T_UINT32 Filesize;		///< File's Size.
	FF_T_UINT32 ObjectCluster;	///< File's Start Cluster.
	FF_T_UINT32 FilePointer;	///< Current Position Pointer.
	FF_T_UINT8	Mode;			///< Mode that File Was opened in.
	FF_T_UINT32	CurrentCluster;	///< Prevents FAT Thrashing
	FF_T_UINT32 AddrCurrentCluster;
} FF_FILE;


typedef struct {
	
#ifdef FF_LFN_SUPPORT
	FF_T_INT8	FileName[256];
#else
	FF_T_INT8	FileName[13];
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


//---------- ERROR CODES

#define FF_ERR_FAT_NULL_POINTER	-2

//---------- PROTOTYPES
// PUBLIC (Interfaces):
FF_T_SINT8	 FF_FindFirst	(FF_IOMAN *pIoman, FF_DIRENT *pDirent, FF_T_INT8 *path);
FF_T_SINT8	 FF_FindNext	(FF_IOMAN *pIoman, FF_DIRENT *pDirent);

FF_FILE		*FF_Open	(FF_IOMAN *pIoman, FF_T_INT8 *path, FF_T_INT8 *filename, FF_T_UINT8 Mode);
FF_T_SINT8	 FF_Close	(FF_FILE *pFile);
FF_T_UINT8	 FF_GetC	(FF_FILE *pFile);
FF_T_UINT32	 FF_Read	(FF_FILE *pFile, FF_T_UINT32 ElementSize, FF_T_UINT32 Count, FF_T_UINT8 *buffer);
FF_T_BOOL	 FF_isEOF	(FF_FILE *pFile);
#endif
