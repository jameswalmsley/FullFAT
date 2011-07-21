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
 *	@file		ff_blk.c
 *	@author		James Walmsley
 *	@ingroup	BLK
 *
 *	@defgroup	BLK Block Calculater
 *	@brief		Handle Block Number conversions
 *
 *	Helps calculate block numbers.
 **/

#include "ff_blk.h"
/**
 *	@private
 **/
FF_T_UINT32 FF_getClusterChainNumber(FF_IOMAN *pIoman, FF_T_UINT32 nEntry, FF_T_UINT16 nEntrySize) {
	FF_PARTITION *pPart				= pIoman->pPartition;
	FF_T_UINT32 clusterChainNumber	= nEntry / (pIoman->BlkSize * (pPart->SectorsPerCluster * pPart->BlkFactor) / nEntrySize);
	return clusterChainNumber;
}

FF_T_UINT32 FF_getClusterPosition(FF_IOMAN *pIoman, FF_T_UINT32 nEntry, FF_T_UINT16 nEntrySize) {
	return nEntry % ((pIoman->BlkSize * (pIoman->pPartition->SectorsPerCluster * pIoman->pPartition->BlkFactor)) / nEntrySize);
}
/**
 *	@private
 **/
FF_T_UINT32 FF_getMajorBlockNumber(FF_IOMAN *pIoman, FF_T_UINT32 nEntry, FF_T_UINT16 nEntrySize) {
	FF_PARTITION *pPart = pIoman->pPartition;
	FF_T_UINT32 relClusterEntry		= nEntry % (pIoman->BlkSize * (pPart->SectorsPerCluster * pPart->BlkFactor) / nEntrySize);
	FF_T_UINT32 majorBlockNumber	= relClusterEntry / (pPart->BlkSize / nEntrySize);
	return majorBlockNumber;
}
/**
 *	@private
 **/
FF_T_UINT8 FF_getMinorBlockNumber(FF_IOMAN *pIoman, FF_T_UINT32 nEntry, FF_T_UINT16 nEntrySize) {
	FF_PARTITION *pPart				= pIoman->pPartition;
	FF_T_UINT32 relClusterEntry		= nEntry % (pIoman->BlkSize * (pPart->SectorsPerCluster * pPart->BlkFactor) / nEntrySize);
	FF_T_UINT16 relmajorBlockEntry	= (FF_T_UINT16)(relClusterEntry % (pPart->BlkSize / nEntrySize));
	FF_T_UINT8 minorBlockNumber		= (FF_T_UINT8) (relmajorBlockEntry / (pIoman->BlkSize / nEntrySize));
	return minorBlockNumber;
}
/**
 *	@private
 **/
FF_T_UINT32 FF_getMinorBlockEntry(FF_IOMAN *pIoman, FF_T_UINT32 nEntry, FF_T_UINT16 nEntrySize) {
	FF_PARTITION *pPart				= pIoman->pPartition;
	FF_T_UINT32 relClusterEntry		= nEntry % (pIoman->BlkSize * (pPart->SectorsPerCluster * pPart->BlkFactor) / nEntrySize);
	FF_T_UINT32 relmajorBlockEntry	= (FF_T_UINT32)(relClusterEntry % (pPart->BlkSize / nEntrySize));
	FF_T_UINT32 minorBlockEntry		= (FF_T_UINT32)(relmajorBlockEntry % (pIoman->BlkSize / nEntrySize));
	return minorBlockEntry;
}

