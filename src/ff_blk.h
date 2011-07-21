/*****************************************************************************
 *  FullFAT - High Performance, Thread-Safe Embedded FAT File-System         *
 *                                                                           *
 *  Copyright(C) 2009 James Walmsley  (james@fullfat-fs.co.uk)               *
 *  Copyright(C) 2010 Hein Tibosch    (hein_tibosch@yahoo.es)                *
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
 *	@file		ff_blk.h
 *	@author		James Walmsley
 *	@ingroup	BLK
 *
 *	Helps calculate block numbers.
 **/

#ifndef _FF_BLK_H_
#define _FF_BLK_H_

#include "ff_ioman.h"

// PROTOTYPES:
FF_T_UINT32 FF_getClusterPosition	(FF_IOMAN *pIoman, FF_T_UINT32 nEntry, FF_T_UINT16 nEntrySize);
FF_T_UINT32 FF_getClusterChainNumber(FF_IOMAN *pIoman, FF_T_UINT32 nEntry, FF_T_UINT16 nEntrySize);
FF_T_UINT32 FF_getMajorBlockNumber	(FF_IOMAN *pIoman, FF_T_UINT32 nEntry, FF_T_UINT16 nEntrySize);
FF_T_UINT8	FF_getMinorBlockNumber	(FF_IOMAN *pIoman, FF_T_UINT32 nEntry, FF_T_UINT16 nEntrySize);
FF_T_UINT32 FF_getMinorBlockEntry	(FF_IOMAN *pIoman, FF_T_UINT32 nEntry, FF_T_UINT16 nEntrySize);

#endif

