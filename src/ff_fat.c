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
	
	FatSector = pIoman->pPartition->FatBeginLBA + (FatOffset / pIoman->pPartition->BlkSize);
	FatSectorEntry = FatOffset % pIoman->pPartition->BlkSize;
	
	LBAadjust = (FF_T_UINT8) (FatSectorEntry / pIoman->BlkSize);
	relClusterEntry = FatSectorEntry % pIoman->BlkSize;
	
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
				F12short[0] = FF_getChar(pBuffer->pBuffer, (pIoman->BlkSize - 1));				
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
			
			FatEntry = FF_getShort((FF_T_UINT8*)&F12short, 0);	// Guarantee correct Endianess!

			if(nCluster & 0x0001) {
				FatEntry = FatEntry >> 4;
			} 
			FatEntry &= 0x0FFF;
			return FatEntry;
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

	return FatEntry;
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
	relClusterEntry = FatSectorEntry % pIoman->BlkSize;
	
	FatSector = FF_getRealLBA(pIoman, FatSector);

#ifdef FF_FAT12_SUPPORT	
	if(pIoman->pPartition->Type == FF_T_FAT12) {
		if(relClusterEntry == (pIoman->BlkSize - 1)) {
			// Fat Entry SPANS a Sector!
			// First Buffer get the last Byte in buffer (first byte of our address)!
			pBuffer = FF_GetBuffer(pIoman, FatSector + LBAadjust, FF_MODE_READ);
			{
				if(!pBuffer) {
					return FF_ERR_DEVICE_DRIVER_FAILED;
				}
				F12short[0] = FF_getChar(pBuffer->pBuffer, (pIoman->BlkSize - 1));				
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
			
			FatEntry = FF_getShort((FF_T_UINT8*)&F12short, 0);	// Guarantee correct Endianess!

			if(nCluster & 0x0001) {
				FatEntry   &= 0x000F;
				Value		= (Value << 4);
				Value	   &= 0xFFF0;
			}  else {
				FatEntry	&= 0xF000;
				Value		&= 0x0FFF;
			}

			FF_putShort(&FatEntry, 0x0000, Value);

			pBuffer = FF_GetBuffer(pIoman, FatSector + LBAadjust, FF_MODE_WRITE);
			{
				if(!pBuffer) {
					return FF_ERR_DEVICE_DRIVER_FAILED;
				}
				FF_putChar(pBuffer->pBuffer, (pIoman->BlkSize - 1), F12short[0]);				
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

	return FatEntry;
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

	for(nCluster = 0; nCluster < pIoman->pPartition->NumClusters; nCluster++) {
		fatEntry = FF_getFatEntry(pIoman, nCluster);
		if(fatEntry == 0x00000000) {
			return nCluster;
		}
	}
	 
	return 0;
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
FF_T_SINT8 FF_ExtendClusterChain(FF_IOMAN *pIoman, FF_T_UINT32 StartCluster, FF_T_UINT16 Count) {
	
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
		FF_putFatEntry(pIoman, currentCluster, nextCluster);
	}
	return 0;
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


	return 0;
}
