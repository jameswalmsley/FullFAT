/*****************************************************************************
 *  FullFAT - High Performance, Thread-Safe Embedded FAT File-System         *
 *                                                                           *
 *  Copyright(C) 2009 James Walmsley  (james@fullfat-fs.co.uk)               *
 *	Copyright(C) 2010 Hein Tibosch    (hein_tibosch@yahoo.es)                *
 *                                                                           *
 *  See RESTRICTIONS.TXT for extra restrictions on the use of FullFAT.       *
 *                                                                           *
 *  Removing this notice is illegal and will invalidate this license.        *
 *****************************************************************************
 *  See http://www.fullfat-fs.co.uk/ for more information.                   *
 *  Or  http://fullfat.googlecode.com/ for latest releases and the wiki.     *
 *****************************************************************************
 * As of 19-July-2011 FullFAT has abandoned the GNU GPL License in favour of *
 * the more flexible Apache 2.0 license. See License.txt for full terms.     *
 *                                                                           *
 *            YOU ARE FREE TO USE FULLFAT IN COMMERCIAL PROJECTS             *
 *****************************************************************************/

/**
 *	@file		ff_hash.h
 *	@author		James Walmsley
 *	@ingroup	HASH
 *
 **/

#ifndef _FF_HASH_H_
#define _FF_HASH_H_

#include "ff_config.h"
#include "ff_types.h"
#include "ff_error.h"

struct			_FF_HASH_TABLE;
typedef struct	_FF_HASH_TABLE *FF_HASH_TABLE;

FF_HASH_TABLE	FF_CreateHashTable	();
FF_ERROR		FF_DestroyHashTable	(FF_HASH_TABLE pHash);
FF_ERROR		FF_ClearHashTable	(FF_HASH_TABLE pHash);
FF_ERROR		FF_ClearHash		(FF_HASH_TABLE pHash, FF_T_UINT32 nHash);
FF_ERROR		FF_SetHash			(FF_HASH_TABLE pHash, FF_T_UINT32 nHash);
FF_T_BOOL		FF_isHashSet		(FF_HASH_TABLE pHash, FF_T_UINT32 nHash);

#endif
