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
	FF_T_UINT16 relmajorBlockEntry	= relClusterEntry % (pPart->BlkSize / nEntrySize);
	FF_T_UINT8 minorBlockNumber		= (FF_T_UINT8) (relmajorBlockEntry / (pIoman->BlkSize / nEntrySize));
	return minorBlockNumber;
}
/**
 *	@private
 **/
FF_T_UINT8 FF_getMinorBlockEntry(FF_IOMAN *pIoman, FF_T_UINT32 nEntry, FF_T_UINT16 nEntrySize) {
	FF_PARTITION *pPart				= pIoman->pPartition;
	FF_T_UINT32 relClusterEntry		= nEntry % (pIoman->BlkSize * (pPart->SectorsPerCluster * pPart->BlkFactor) / nEntrySize);
	FF_T_UINT16 relmajorBlockEntry	= relClusterEntry % (pPart->BlkSize / nEntrySize);
	FF_T_UINT8 minorBlockEntry		= (FF_T_UINT8) relmajorBlockEntry % (pIoman->BlkSize / nEntrySize);
	return minorBlockEntry;
}

