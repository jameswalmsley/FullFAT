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
 *	@file		ff_hash.c
 *	@author		James Walmsley
 *	@ingroup	HASH
 *
 *	@defgroup	HASH HASH Table
 *	@brief		Provides a simple HASH lookup table.
 *
 **/

#include "ff_hash.h"
#include <stdlib.h>
#include <string.h>

#ifdef FF_HASH_CACHE
struct _FF_HASH_TABLE {
	FF_T_UINT8 bitTable[FF_HASH_TABLE_SIZE];
};

/**
 *
 *
 **/
FF_HASH_TABLE FF_CreateHashTable() {
	FF_HASH_TABLE pHash = (FF_HASH_TABLE) FF_MALLOC(sizeof(struct _FF_HASH_TABLE));

	if(pHash) {
		FF_ClearHashTable(pHash);
		return pHash;
	}

	return NULL;
}

FF_ERROR FF_ClearHashTable(FF_HASH_TABLE pHash) {
	if(pHash) {
		memset(pHash->bitTable, 0, FF_HASH_TABLE_SIZE);
		return FF_ERR_NONE;
	}

	return FF_ERR_NULL_POINTER;
}

FF_ERROR FF_SetHash(FF_HASH_TABLE pHash, FF_T_UINT32 nHash) {
	FF_T_UINT32 tblIndex	= ((nHash / 8) % FF_HASH_TABLE_SIZE);
	FF_T_UINT32 tblBit		= nHash % 8;

	if(pHash) {
		pHash->bitTable[tblIndex] |= (0x80 >> tblBit);
		return FF_ERR_NONE;
	}

	return FF_ERR_NULL_POINTER;	
}

FF_ERROR FF_ClearHash(FF_HASH_TABLE pHash, FF_T_UINT32 nHash) {
	FF_T_UINT32 tblIndex	= ((nHash / 8) % FF_HASH_TABLE_SIZE);
	FF_T_UINT32 tblBit		= nHash % 8;

	if(pHash) {
		pHash->bitTable[tblIndex] &= ~(0x80 >> tblBit);
		return FF_ERR_NONE;
	}

	return FF_ERR_NULL_POINTER;
}

FF_T_BOOL FF_isHashSet(FF_HASH_TABLE pHash, FF_T_UINT32 nHash) {
	FF_T_UINT32 tblIndex	= ((nHash / 8) % FF_HASH_TABLE_SIZE);
	FF_T_UINT32 tblBit		= nHash % 8;

	if(pHash) {
		if(pHash->bitTable[tblIndex] & (0x80 >> tblBit)) {
			return FF_TRUE;
		}
	}
	return FF_FALSE;
}

FF_ERROR FF_DestroyHashTable(FF_HASH_TABLE pHash) {
	if(pHash) {
		FF_FREE(pHash);
		return FF_ERR_NONE;
	}
	return FF_ERR_NULL_POINTER;
}

#endif
