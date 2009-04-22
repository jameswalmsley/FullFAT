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
 *	@file		ff_fat.c
 *	@author		James Walmsley
 *	@ingroup	FAT
 *
 *	@defgroup	FAT Fat File-System
 *	@brief		Handles FAT access and traversal.
 *
 *	Provides file-system interfaces for the FAT file-system.
 **/

#include "ff_fat.h"
#include "ff_config.h"
#include <string.h>

static void FF_lockFAT(FF_IOMAN *pIoman) {
	FF_PendSemaphore(pIoman->pSemaphore);	// Use Semaphore to protect FAT modifications.
	{
		while(pIoman->FatLock) {
			FF_ReleaseSemaphore(pIoman->pSemaphore);
			FF_Yield();						// Keep Releasing and Yielding until we have the Fat protector.
			FF_PendSemaphore(pIoman->pSemaphore);
		}
		pIoman->FatLock = 1;
	}
	FF_ReleaseSemaphore(pIoman->pSemaphore);
}

static void FF_unlockFAT(FF_IOMAN *pIoman) {
	FF_PendSemaphore(pIoman->pSemaphore);
	{
		pIoman->FatLock = 0;
	}
	FF_ReleaseSemaphore(pIoman->pSemaphore);
}

/**
 *	@private
 **/
FF_T_UINT32 FF_getRealLBA(FF_IOMAN *pIoman, FF_T_UINT32 LBA) {
	return LBA * pIoman->pPartition->BlkFactor;
}

/**
 *	@private
 **/
FF_T_UINT32 FF_Cluster2LBA(FF_IOMAN *pIoman, FF_T_UINT32 Cluster) {
	FF_T_UINT32 lba = 0;
	FF_PARTITION *pPart;
	if(pIoman) {
		pPart = pIoman->pPartition;

		if(Cluster > 1) {
			lba = ((Cluster - 2) * pPart->SectorsPerCluster) + pPart->FirstDataSector;
		} else {
			lba = pPart->ClusterBeginLBA;
		}
	}
	return lba;
}

/**
 *	@private
 **/
FF_T_UINT32 FF_LBA2Cluster(FF_IOMAN *pIoman, FF_T_UINT32 Address) {
	FF_T_UINT32 cluster = 0;
	FF_PARTITION *pPart;
	if(pIoman) {
		pPart = pIoman->pPartition;
		if(pPart->Type == FF_T_FAT32) {
			cluster = ((Address - pPart->ClusterBeginLBA) / pPart->SectorsPerCluster) + 2;
		} else {
			cluster = ((Address - pPart->ClusterBeginLBA) / pPart->SectorsPerCluster);
		}
	}
	return cluster;
}

/**
 *	@private
 **/
FF_T_SINT32 FF_getFatEntry(FF_IOMAN *pIoman, FF_T_UINT32 nCluster) {

	FF_BUFFER 	*pBuffer;
	FF_T_UINT32 FatOffset;
	FF_T_UINT32 FatSector;
	FF_T_UINT32 FatSectorEntry;
	FF_T_UINT32 FatEntry;
	FF_T_UINT8	LBAadjust;
	FF_T_UINT16 relClusterEntry;
	
	FF_T_UINT8	F12short[2];		// For FAT12 FAT Table Across sector boundary traversal.
	
	if(pIoman->pPartition->Type == FF_T_FAT32) {
		FatOffset = nCluster * 4;
	} else if(pIoman->pPartition->Type == FF_T_FAT16) {
		FatOffset = nCluster * 2;
	}else {
		FatOffset = nCluster + (nCluster / 2);
	}
	
	FatSector		= pIoman->pPartition->FatBeginLBA + (FatOffset / pIoman->pPartition->BlkSize);
	FatSectorEntry	= FatOffset % pIoman->pPartition->BlkSize;
	
	LBAadjust		= (FF_T_UINT8)	(FatSectorEntry / pIoman->BlkSize);
	relClusterEntry = (FF_T_UINT16) (FatSectorEntry % pIoman->BlkSize);
	
	FatSector = FF_getRealLBA(pIoman, FatSector);
	
	if(pIoman->pPartition->Type == FF_T_FAT12) {
		if(relClusterEntry == (pIoman->BlkSize - 1)) {
			// Fat Entry SPANS a Sector!
			// First Buffer get the last Byte in buffer (first byte of our address)!
			pBuffer = FF_GetBuffer(pIoman, FatSector + LBAadjust, FF_MODE_READ);
			{
				if(!pBuffer) {
					return FF_ERR_DEVICE_DRIVER_FAILED;
				}
				F12short[0] = FF_getChar(pBuffer->pBuffer, (FF_T_UINT16)(pIoman->BlkSize - 1));				
			}
			FF_ReleaseBuffer(pIoman, pBuffer);
			// Second Buffer get the first Byte in buffer (second byte of out address)!
			pBuffer = FF_GetBuffer(pIoman, FatSector + LBAadjust + 1, FF_MODE_READ);
			{
				if(!pBuffer) {
					return FF_ERR_DEVICE_DRIVER_FAILED;
				}
				F12short[1] = FF_getChar(pBuffer->pBuffer, 0);				
			}
			FF_ReleaseBuffer(pIoman, pBuffer);
			
			FatEntry = (FF_T_UINT32) FF_getShort((FF_T_UINT8*)&F12short, 0);	// Guarantee correct Endianess!

			if(nCluster & 0x0001) {
				FatEntry = FatEntry >> 4;
			} 
			FatEntry &= 0x0FFF;
			return (FF_T_SINT32) FatEntry;
		}
	}
	pBuffer = FF_GetBuffer(pIoman, FatSector + LBAadjust, FF_MODE_READ);
	{
		if(!pBuffer) {
			return FF_ERR_DEVICE_DRIVER_FAILED;
		}
		if(pIoman->pPartition->Type == FF_T_FAT32) {
			FatEntry = FF_getLong(pBuffer->pBuffer, relClusterEntry);
			FatEntry &= 0x0fffffff;	// Clear the top 4 bits.
		} else if(pIoman->pPartition->Type == FF_T_FAT16) {
			FatEntry = (FF_T_UINT32) FF_getShort(pBuffer->pBuffer, relClusterEntry);
		} else {
			FatEntry = (FF_T_UINT32) FF_getShort(pBuffer->pBuffer, relClusterEntry);
			if(nCluster & 0x0001) {
				FatEntry = FatEntry >> 4;
			} 
			FatEntry &= 0x0FFF;
		}
	}
	FF_ReleaseBuffer(pIoman, pBuffer);

	return (FF_T_SINT32) FatEntry;
}

FF_T_SINT8 FF_ClearCluster(FF_IOMAN *pIoman, FF_T_UINT32 nCluster) {
	FF_BUFFER *pBuffer;
	FF_T_UINT16 i;
	FF_T_UINT32	BaseLBA;
	FF_T_SINT8	RetVal = 0;

	BaseLBA = FF_Cluster2LBA(pIoman, nCluster);
	BaseLBA = FF_getRealLBA(pIoman, BaseLBA);

	for(i = 0; i < pIoman->pPartition->SectorsPerCluster; i++) {
		pBuffer = FF_GetBuffer(pIoman, BaseLBA++, FF_MODE_WRITE);
		{
			if(pBuffer) {
				memset(pBuffer->pBuffer, 0x00, 512);
			} else {
				RetVal = -3;
			}
		}
		FF_ReleaseBuffer(pIoman, pBuffer);
	}

	return RetVal;
}

/**
 *	@private
 *	@brief	Returns the Cluster address of the Cluster number from the beginning of a chain.
 *	
 *	@param	pIoman		FF_IOMAN Object
 *	@param	Start		Cluster address of the first cluster in the chain.
 *	@param	Count		Number of Cluster in the chain, 
 *
 *	@return	FF_TRUE if it is an end of chain, otherwise FF_FALSE.
 *
 **/
FF_T_UINT32 FF_TraverseFAT(FF_IOMAN *pIoman, FF_T_UINT32 Start, FF_T_UINT32 Count) {
	
	FF_T_UINT32 i;
	FF_T_UINT32 fatEntry = Start, currentCluster = Start;

	for(i = 0; i < Count; i++) {
		fatEntry = FF_getFatEntry(pIoman, currentCluster);
		if(fatEntry == (FF_T_UINT32) FF_ERR_DEVICE_DRIVER_FAILED) {
			return 0;
		}

		if(FF_isEndOfChain(pIoman, fatEntry)) {
			return currentCluster;
		} else {
			currentCluster = fatEntry;
		}	
	}
	
	return fatEntry;
}

FF_T_UINT32 FF_FindEndOfChain(FF_IOMAN *pIoman, FF_T_UINT32 Start) {

	FF_T_UINT32 fatEntry = Start, currentCluster = Start;

	while(!FF_isEndOfChain(pIoman, fatEntry)) {
		fatEntry = FF_getFatEntry(pIoman, currentCluster);
		if(fatEntry == (FF_T_UINT32) FF_ERR_DEVICE_DRIVER_FAILED) {
			return 0;
		}

		if(FF_isEndOfChain(pIoman, fatEntry)) {
			return currentCluster;
		} else {
			currentCluster = fatEntry;
		}	
	}
	
	return fatEntry;
}


/**
 *	@private
 *	@brief	Tests if the fatEntry is an End of Chain Marker.
 *	
 *	@param	pIoman		FF_IOMAN Object
 *	@param	fatEntry	The fat entry from the FAT table to be checked.
 *
 *	@return	FF_TRUE if it is an end of chain, otherwise FF_FALSE.
 *
 **/
FF_T_BOOL FF_isEndOfChain(FF_IOMAN *pIoman, FF_T_UINT32 fatEntry) {
	FF_T_BOOL result = FF_FALSE;
	if(pIoman->pPartition->Type == FF_T_FAT32) {
		if((fatEntry & 0x0fffffff) >= 0x0ffffff8) {
			result = FF_TRUE;
		}
	} else if(pIoman->pPartition->Type == FF_T_FAT16) {
		if(fatEntry >= 0x0000fff8) {
			result = FF_TRUE;
		}
	} else {
		if(fatEntry >= 0x00000ff8) {
			result = FF_TRUE;
		}	
	}
	if(fatEntry == 0x00000000) {
		result = FF_TRUE;	//Perhaps trying to read a deleted file!
	}
	return result;
}






/**
 *	@private
 *	@brief	Writes a new Entry to the FAT Tables.
 *	
 *	@param	pIoman		IOMAN object.
 *	@param	nCluster	Cluster Number to be modified.
 *	@param	Value		The Value to store.
 **/
FF_T_SINT8 FF_putFatEntry(FF_IOMAN *pIoman, FF_T_UINT32 nCluster, FF_T_UINT32 Value) {

	FF_BUFFER 	*pBuffer;
	FF_T_UINT32 FatOffset;
	FF_T_UINT32 FatSector;
	FF_T_UINT32 FatSectorEntry;
	FF_T_UINT32 FatEntry;
	FF_T_UINT8	LBAadjust;
	FF_T_UINT16 relClusterEntry;
	
	FF_T_UINT8	F12short[2];		// For FAT12 FAT Table Across sector boundary traversal.
	
	if(pIoman->pPartition->Type == FF_T_FAT32) {
		FatOffset = nCluster * 4;
	} else if(pIoman->pPartition->Type == FF_T_FAT16) {
		FatOffset = nCluster * 2;
	}else {
		FatOffset = nCluster + (nCluster / 2);
	}
	
	FatSector = pIoman->pPartition->FatBeginLBA + (FatOffset / pIoman->pPartition->BlkSize);
	FatSectorEntry = FatOffset % pIoman->pPartition->BlkSize;
	
	LBAadjust = (FF_T_UINT8) (FatSectorEntry / pIoman->BlkSize);
	relClusterEntry = (FF_T_UINT16)(FatSectorEntry % pIoman->BlkSize);
	
	FatSector = FF_getRealLBA(pIoman, FatSector);

#ifdef FF_FAT12_SUPPORT	
	if(pIoman->pPartition->Type == FF_T_FAT12) {
		if(relClusterEntry == (FF_T_UINT16) (pIoman->BlkSize - 1)) {
			// Fat Entry SPANS a Sector!
			// First Buffer get the last Byte in buffer (first byte of our address)!
			pBuffer = FF_GetBuffer(pIoman, FatSector + LBAadjust, FF_MODE_READ);
			{
				if(!pBuffer) {
					return FF_ERR_DEVICE_DRIVER_FAILED;
				}
				F12short[0] = FF_getChar(pBuffer->pBuffer, (FF_T_UINT16)(pIoman->BlkSize - 1));				
			}
			FF_ReleaseBuffer(pIoman, pBuffer);
			// Second Buffer get the first Byte in buffer (second byte of out address)!
			pBuffer = FF_GetBuffer(pIoman, FatSector + LBAadjust + 1, FF_MODE_READ);
			{
				if(!pBuffer) {
					return FF_ERR_DEVICE_DRIVER_FAILED;
				}
				F12short[1] = FF_getChar(pBuffer->pBuffer, (FF_T_UINT16) 0x0000);				
			}
			FF_ReleaseBuffer(pIoman, pBuffer);
			
			FatEntry = FF_getShort((FF_T_UINT8*)&F12short, (FF_T_UINT16) 0x0000);	// Guarantee correct Endianess!

			if(nCluster & 0x0001) {
				FatEntry   &= 0x000F;
				Value		= (Value << 4);
				Value	   &= 0xFFF0;
			}  else {
				FatEntry	&= 0xF000;
				Value		&= 0x0FFF;
			}

			FF_putShort((FF_T_UINT8 *)&FatEntry, 0x0000, (FF_T_UINT16)Value);

			pBuffer = FF_GetBuffer(pIoman, FatSector + LBAadjust, FF_MODE_WRITE);
			{
				if(!pBuffer) {
					return FF_ERR_DEVICE_DRIVER_FAILED;
				}
				FF_putChar(pBuffer->pBuffer, (FF_T_UINT16)(pIoman->BlkSize - 1), F12short[0]);				
			}
			FF_ReleaseBuffer(pIoman, pBuffer);
			// Second Buffer get the first Byte in buffer (second byte of out address)!
			pBuffer = FF_GetBuffer(pIoman, FatSector + LBAadjust + 1, FF_MODE_READ);
			{
				if(!pBuffer) {
					return FF_ERR_DEVICE_DRIVER_FAILED;
				}
				FF_putChar(pBuffer->pBuffer, 0x0000, F12short[1]);				
			}
			FF_ReleaseBuffer(pIoman, pBuffer);
		}
	}
#endif
	
	pBuffer = FF_GetBuffer(pIoman, FatSector + LBAadjust, FF_MODE_WRITE);
	{
		if(!pBuffer) {
			return FF_ERR_DEVICE_DRIVER_FAILED;
		}
		if(pIoman->pPartition->Type == FF_T_FAT32) {
			Value &= 0x0fffffff;	// Clear the top 4 bits.
			FF_putLong(pBuffer->pBuffer, relClusterEntry, Value);
		} else if(pIoman->pPartition->Type == FF_T_FAT16) {
			FF_putShort(pBuffer->pBuffer, relClusterEntry, (FF_T_UINT16) Value);
		} else {
			FatEntry	= (FF_T_UINT32) FF_getShort(pBuffer->pBuffer, relClusterEntry);
			if(nCluster & 0x0001) {
				FatEntry   &= 0x000F;
				Value		= (Value << 4);
				Value	   &= 0xFFF0;
			}  else {
				FatEntry	&= 0xF000;
				Value		&= 0x0FFF;
			}
			
			FF_putShort(pBuffer->pBuffer, relClusterEntry, (FF_T_UINT16) (FatEntry | Value));
		}
	}
	FF_ReleaseBuffer(pIoman, pBuffer);

	return 0;
}



/**
 *	@private
 *	@brief	Finds a Free Cluster and returns its number.
 *
 *	@param	pIoman	IOMAN Object.
 *
 *	@return	The number of the cluster found to be free.
 *	@return 0 on error.
 **/
FF_T_UINT32 FF_FindFreeCluster(FF_IOMAN *pIoman) {
	FF_T_UINT32 nCluster;
	FF_T_UINT32 fatEntry;

	for(nCluster = pIoman->pPartition->LastFreeCluster; nCluster < pIoman->pPartition->NumClusters; nCluster++) {
		fatEntry = FF_getFatEntry(pIoman, nCluster);
		if(fatEntry == 0x00000000) {
			pIoman->pPartition->LastFreeCluster = nCluster;
			return nCluster;
		}
	}
	 
	return 0;
}

/**
 * @private
 * @brief	Create's a Cluster Chain
 **/
FF_T_UINT32 FF_CreateClusterChain(FF_IOMAN *pIoman) {
	FF_T_UINT32	iStartCluster;
	iStartCluster = FF_FindFreeCluster(pIoman);
	FF_putFatEntry(pIoman, iStartCluster, 0xFFFFFFFF); // Mark the cluster as EOC
	return iStartCluster;
}

FF_T_UINT32 FF_GetChainLength(FF_IOMAN *pIoman, FF_T_UINT32 pa_nStartCluster) {
	FF_T_UINT32 iLength = 0;
	
	FF_lockFAT(pIoman);
	{
		while(!FF_isEndOfChain(pIoman, pa_nStartCluster)) {
			pa_nStartCluster = FF_getFatEntry(pIoman, pa_nStartCluster);
			iLength++;
		}
	}
	FF_unlockFAT(pIoman);

	return iLength;
}

/**
 *	@private
 *	@brief Extend a Cluster chain by Count number of Clusters
 *	
 *	@param	pIoman			IOMAN object.
 *	@param	StartCluster	Cluster Number that starts the chain.
 *	@param	Count			Number of clusters to extend the chain with.
 *
 **/
FF_T_UINT32 FF_ExtendClusterChain(FF_IOMAN *pIoman, FF_T_UINT32 StartCluster, FF_T_UINT16 Count) {
	
	FF_T_UINT32 fatEntry = StartCluster;
	FF_T_UINT32 currentCluster, nextCluster;
	FF_T_UINT32 clusEndOfChain;
	FF_T_UINT16 i;

	do {
		currentCluster = fatEntry;
		fatEntry = FF_getFatEntry(pIoman, currentCluster);
	}while(!FF_isEndOfChain(pIoman, fatEntry));

	clusEndOfChain = currentCluster;
	nextCluster = FF_FindFreeCluster(pIoman);	// Find Free clusters!

	FF_putFatEntry(pIoman, clusEndOfChain, nextCluster);

	for(i = 0; i < Count; i++) {
		currentCluster = nextCluster;
		if(i + 1 == Count) {
			FF_putFatEntry(pIoman, currentCluster, 0xFFFFFFFF);
			break;
		}

		nextCluster = FF_FindFreeCluster(pIoman);
		FF_putFatEntry(pIoman, currentCluster, ++nextCluster);
	}
	return currentCluster;
}


/**
 *	@private
 *	@brief Free's Disk space by freeing unused links on Cluster Chains
 *
 *	@param	pIoman,			IOMAN object.
 *	@param	StartCluster	Cluster Number that starts the chain.
 *	@param	Count			Number of Clusters from the end of the chain to unlink.
 *	@param	Count			0 Means Free the entire chain (delete file).
 *
 *	@return 0 On Success.
 *	@return	-1 If the device driver failed to provide access.
 *
 **/
FF_T_SINT8 FF_UnlinkClusterChain(FF_IOMAN *pIoman, FF_T_UINT32 StartCluster, FF_T_UINT16 Count) {
	
	FF_T_UINT32 fatEntry;
	FF_T_UINT32 currentCluster, chainLength = 0;

	fatEntry = StartCluster;

	if(Count == 0) {
		// Free all clusters in the chain!
		currentCluster = StartCluster;
		fatEntry = currentCluster;
        do {
			fatEntry = FF_getFatEntry(pIoman, fatEntry);
			FF_putFatEntry(pIoman, currentCluster, 0x00000000);
			currentCluster = fatEntry;
		}while(!FF_isEndOfChain(pIoman, fatEntry));
	} else {
		// Truncation - This is quite hard, because we can only do it backwards.
		do {
			fatEntry = FF_getFatEntry(pIoman, fatEntry);
			chainLength++;
		}while(!FF_isEndOfChain(pIoman, fatEntry));
	}

	return 0;
}

