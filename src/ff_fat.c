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
 *	@file		ff_fat.c
 *	@author		James Walmsley
 *	@ingroup	FAT
 *
 *	@defgroup	FAT
 *	@brief		Handles FAT access and traversal.
 *
 *	Provides file-system interfaces for the FAT file-system.
 **/

#include "ff_fat.h"
#include "ff_config.h"
#include <string.h>

FF_T_UINT32 FF_getRealLBA(FF_IOMAN *pIoman, FF_T_UINT32 LBA) {
	return LBA * pIoman->pPartition->BlkFactor;
}

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


// Release all buffers before calling this!
FF_T_UINT32 FF_getFatEntry(FF_IOMAN *pIoman, FF_T_UINT32 nCluster) {

	FF_BUFFER 	*pBuffer;
	FF_T_UINT32 FatOffset;
	FF_T_UINT32 FatSector;
	FF_T_UINT32 FatSectorEntry;
	FF_T_UINT32 FatEntry;
	FF_T_UINT8	LBAadjust;
	FF_T_UINT32 relClusterEntry;
	FF_T_UINT8	char1, char2;
	/*
		This code needs to be modified to work with big sectors > 512 in size!
	*/
	
	if(pIoman->pPartition->Type == FF_T_FAT32) {
		FatOffset = nCluster * 4;
	} else if(pIoman->pPartition->Type == FF_T_FAT16) {
		FatOffset = nCluster * 2;
	}else {
		FatOffset = nCluster + (nCluster / 2);
	}
	
	FatSector = pIoman->pPartition->FatBeginLBA + (FatOffset / pIoman->pPartition->BlkSize);
	FatSectorEntry = FatOffset % pIoman->pPartition->BlkSize;
	
	LBAadjust = (FatSectorEntry / 512);
	relClusterEntry = FatSectorEntry % 512;
	

	
	FatSector = FF_getRealLBA(pIoman, FatSector);
	
	if(pIoman->pPartition->Type == FF_T_FAT12) {
		if(relClusterEntry == (512 - 1)) {
			// Fat Entry SPANS a Sector!
			// First Buffer get the last Byte!
			pBuffer = FF_GetBuffer(pIoman, FatSector + LBAadjust, FF_MODE_READ);
			{
				char1 = FF_getChar(pBuffer->pBuffer, 511);				
			}
			FF_ReleaseBuffer(pIoman, pBuffer);
			
			pBuffer = FF_GetBuffer(pIoman, FatSector + LBAadjust + 1, FF_MODE_READ);
			{
				char2 = FF_getChar(pBuffer->pBuffer, 0);				
			}
			FF_ReleaseBuffer(pIoman, pBuffer);
			
			FatEntry = (FF_T_UINT16) (char1) | (FF_T_UINT16)(char2  << 8);	// ENDIANESS NEEDS GATING HERE!
			FatEntry = FatEntry >> 4;
			FatEntry &= 0x0FFF;
			
			return FatEntry;
			
		}
	}
	
	pBuffer = FF_GetBuffer(pIoman, FatSector + LBAadjust, FF_MODE_READ);
	{
		if(pIoman->pPartition->Type == FF_T_FAT32) {
			FatEntry = FF_getLong(pBuffer->pBuffer, relClusterEntry);
			FatEntry &= 0x0fffffff;	// Clear the top 4 bits.
		} else if(pIoman->pPartition->Type == FF_T_FAT16) {
			FatEntry = (FF_T_UINT32) FF_getShort(pBuffer->pBuffer, relClusterEntry);
		} else {
			FatEntry = (FF_T_UINT32) FF_getShort(pBuffer->pBuffer, relClusterEntry);
			if(nCluster & 0x0001) {
				FatEntry = FatEntry >> 4;
				FatEntry &= 0x0FFF;
			} else {
				FatEntry &= 0x0FFF;
			}
		}
	}
	FF_ReleaseBuffer(pIoman, pBuffer);

	return FatEntry;
}

/**
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

/*
	FindEntry Now Wraps GetEntry, Removing the previous functions.
	Saving massive code density.
*/
FF_T_UINT32 FF_FindEntry(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_T_INT8 *name, FF_T_UINT8 pa_Attrib, FF_DIRENT *pDirent) {
	
	FF_T_INT32		retVal;
	FF_DIRENT		MyDir;
#ifdef FF_LFN_SUPPORT
	FF_T_INT8		Filename[260];
#else
	FF_T_INT8		Filename[12];
#endif
	FF_T_UINT16		fnameLen;
	FF_T_UINT16		compareLength;
	FF_T_UINT16		nameLen = strlen(name);
	
	MyDir.CurrentItem = 0;		// Starting at the first Dir Entry
	MyDir.CurrentCluster = 0;	// Set to Zero so that traversing across dir's > than 1 cluster in size works.

	while(1) {	
		do {
			retVal = FF_GetEntry(pIoman, MyDir.CurrentItem, DirCluster, &MyDir);
		}while(retVal == -1);

		if(retVal == -2) {
			break;
		}
		if((MyDir.Attrib & pa_Attrib) == pa_Attrib){
			strcpy(Filename, MyDir.FileName);
			fnameLen = strlen(Filename);
			FF_tolower(Filename, fnameLen);
			FF_tolower(name, nameLen);
			if(nameLen > fnameLen) {
				compareLength = nameLen;
			} else {
				compareLength = fnameLen;
			}
			if(strncmp(name, Filename, compareLength) == 0) {
				// Object found!!
				if(pDirent) {
					memcpy(pDirent, &MyDir, sizeof(FF_DIRENT));
				}
				return MyDir.ObjectCluster;	// Return the cluster number
			}
		}
		
	}

	return 0;

}

// returns the Cluster of the specified directory
FF_T_UINT32 FF_FindDir(FF_IOMAN *pIoman, FF_T_INT8 *path) {

	FF_T_UINT32 dirCluster = pIoman->pPartition->RootDirCluster;
	FF_T_INT8 mypath[260];
	FF_T_INT8 *token;
	FF_T_UINT16	it = 0, mod = 0;		// Re-entrancy Variables for FF_strtok()
	FF_T_UINT32 pathLength = strlen(path);
	FF_DIRENT MyDir;

	MyDir.CurrentCluster = 0;

	if(pathLength == 1) {	// Must be the root dir! (/ or \)
		return pIoman->pPartition->RootDirCluster;
	}
	strcpy(mypath, path);

	token = FF_strtok(mypath, &it, &mod); // Tokenise Path, thread-safely

	while(token != NULL) {
		dirCluster = FF_FindEntry(pIoman, dirCluster, token, FF_FAT_ATTR_DIR, &MyDir);
		if(dirCluster == 0 && MyDir.CurrentItem == 2) {	// .. Dir Entry pointing to root dir.
			dirCluster = pIoman->pPartition->RootDirCluster;
		}
		token = FF_strtok(mypath, &it, &mod);
	}

	return dirCluster;
}


#ifdef FF_LFN_SUPPORT
FF_T_SINT8 FF_getLFN(FF_IOMAN *pIoman, FF_BUFFER *pBuffer, FF_DIRENT *pDirent, FF_T_INT8 *filename) {

	FF_T_UINT8	 	numLFNs;
	FF_T_UINT8	 	retLFNs = 0;
	FF_T_UINT16		lenlfn = 0;
	FF_T_UINT8		tester;
	FF_T_UINT16		i,y;
	FF_T_UINT32		OriginalSector = pBuffer->Sector, Sector = pBuffer->Sector;
	FF_T_UINT32		CurrentCluster;
	FF_T_UINT32		fatEntry;

	FF_T_UINT8		*buffer = pBuffer->pBuffer;

	FF_T_UINT32		Entry		= FF_getMinorBlockEntry(pIoman, pDirent->CurrentItem, 32);

	tester = FF_getChar(pBuffer->pBuffer, (Entry * 32));
	numLFNs = tester & ~0x40;

	while(numLFNs > 0) {
		if(FF_getClusterChainNumber(pIoman, pDirent->CurrentItem, 32) > pDirent->CurrentCluster) {
			FF_ReleaseBuffer(pIoman, pBuffer);
			fatEntry = FF_getFatEntry(pIoman, pDirent->DirCluster);
			
			if(FF_isEndOfChain(pIoman, fatEntry)) {
				CurrentCluster = pDirent->DirCluster;
				// ERROR THIS SHOULD NOT OCCUR!
			} else {
				CurrentCluster = fatEntry;
			}

			pBuffer = FF_GetBuffer(pIoman, FF_getRealLBA(pIoman, FF_Cluster2LBA(pIoman, CurrentCluster)), FF_MODE_READ);
			Entry = 0;	
		}

		if(Entry > 15) {
			FF_ReleaseBuffer(pIoman, pBuffer);
			Sector += 1;
			pBuffer = FF_GetBuffer(pIoman, Sector, FF_MODE_READ);
			Entry = 0;
		}

		for(i = 0, y = 0; i < 5; i++, y += 2) {
			filename[i + ((numLFNs - 1) * 13)] = buffer[(Entry * 32) + FF_FAT_LFN_NAME_1 + y];
			lenlfn++;
		}

		for(i = 0, y = 0; i < 6; i++, y += 2) {
			filename[i + ((numLFNs - 1) * 13) + 5] = buffer[(Entry * 32) + FF_FAT_LFN_NAME_2 + y];
			lenlfn++;
		}

		for(i = 0, y = 0; i < 2; i++, y += 2) {
			filename[i + ((numLFNs - 1) * 13) + 11] = buffer[(Entry * 32) + FF_FAT_LFN_NAME_3 + y];
			lenlfn++;
		}

		numLFNs--;

		Entry++;
		pDirent->CurrentItem += 1;
	}
	return 0;
}
#endif

void FF_ProcessShortName(FF_T_INT8 *name) {
	FF_T_INT8	shortName[12];
	FF_T_UINT8	i;
	memcpy(shortName, name, 11);
	
	for(i = 0; i < 8; i++) {
		if(shortName[i] == 0x20) {
			if(shortName[8] != 0x20){
				name[i] = '.';
				name[i+1] = shortName[8];
				name[i+2] = shortName[9];
				name[i+3] = shortName[10];
				name[i+4] = '\0';
			} else {
				name[i] = '\0';
			}
			break;
		}
		name[i] = shortName[i];
	}
}


FF_T_SINT8 FF_GetEntry(FF_IOMAN *pIoman, FF_T_UINT32 nEntry, FF_T_UINT32 DirCluster, FF_DIRENT *pDirent) {
	
	FF_T_UINT8		tester;			///< Unsigned byte for testing if dir is deleted
	FF_T_SINT8		retVal = 0;		///< Return Value for Function
	FF_T_UINT32		CurrentCluster = DirCluster;
	FF_T_UINT16		myShort, numLFNs;	///< 16 bits for calculating item cluster.
	FF_T_UINT32		fatEntry = 0;	///< Used for following Cluster Chain
	FF_T_UINT32		itemLBA;
	FF_PARTITION	*pPart = pIoman->pPartition;	///< Used to make code easier to read.
	FF_BUFFER		*pBuffer;	///< Buffer for acquired sector.
	FF_T_UINT8		i;
	FF_T_UINT32 minorBlockEntry		= FF_getMinorBlockEntry(pIoman, nEntry, 32);
	
	// Lets follow the chain to find its real number
	
	if(FF_getClusterChainNumber(pIoman, nEntry, 32) > pDirent->CurrentCluster) {

		fatEntry = FF_getFatEntry(pIoman, pDirent->DirCluster);
		
		if(FF_isEndOfChain(pIoman, fatEntry)) {
			CurrentCluster = DirCluster;
			return -2;
			// ERROR THIS SHOULD NOT OCCUR!
		} else {
			CurrentCluster = fatEntry;
			pDirent->DirCluster = fatEntry;
		}
		//clusterSwitched = FF_TRUE;
		pDirent->CurrentCluster += 1;
	}

	itemLBA = FF_Cluster2LBA(pIoman, CurrentCluster) + FF_getMajorBlockNumber(pIoman, nEntry, 32);
	itemLBA = FF_getRealLBA(pIoman, itemLBA) + FF_getMinorBlockNumber(pIoman, nEntry, 32);

	pBuffer = FF_GetBuffer(pIoman, itemLBA, FF_MODE_READ);
	{
		for(;minorBlockEntry < (512 / 32); minorBlockEntry++) {
			tester = FF_getChar(pBuffer->pBuffer, (minorBlockEntry * 32));
			if(tester != 0xE5 && tester != 0x00) {
				
				pDirent->Attrib = FF_getChar(pBuffer->pBuffer, (FF_FAT_DIRENT_ATTRIB + (32 * minorBlockEntry)));
				if((pDirent->Attrib & FF_FAT_ATTR_LFN) == FF_FAT_ATTR_LFN) {
					numLFNs = (tester & ~0x40);
#ifdef FF_LFN_SUPPORT
					// May Get the Next Buffer if it needs to!
					FF_getLFN(pIoman, pBuffer, pDirent, pDirent->FileName);
					pDirent->ProcessedLFN = FF_TRUE;
#else
					pDirent->CurrentItem += numLFNs;
					
#endif
					minorBlockEntry += (numLFNs -1);	// (LFN's -1 because the loop will increment once also!)
					retVal = -1;
				} else {
					if(pDirent->ProcessedLFN == FF_TRUE) {
						pDirent->ProcessedLFN = FF_FALSE;
					} else {
						if(pDirent->Attrib == FF_FAT_ATTR_DIR || pDirent->Attrib == FF_FAT_ATTR_VOLID) {
							strncpy(pDirent->FileName, (pBuffer->pBuffer + (32 * minorBlockEntry)), 11);
							for(i = 0; i < 11; i++) {
								if(pDirent->FileName[i] == 0x20) {
									break;
								}
							}
							pDirent->FileName[i] = '\0';
						} else {
							strncpy(pDirent->FileName, (pBuffer->pBuffer + (32 * minorBlockEntry)), 11);
							FF_ProcessShortName(pDirent->FileName);
						}
					}
					myShort					 = FF_getShort(pBuffer->pBuffer, (FF_FAT_DIRENT_CLUS_HIGH + (32 * minorBlockEntry)));
					pDirent->ObjectCluster   = (FF_T_UINT32) (myShort << 16);
					myShort					 = FF_getShort(pBuffer->pBuffer, (FF_FAT_DIRENT_CLUS_LOW + (32 * minorBlockEntry)));
					pDirent->ObjectCluster  |= myShort;
					pDirent->Filesize		 = FF_getLong(pBuffer->pBuffer, (FF_FAT_DIRENT_FILESIZE + (32 * minorBlockEntry)));
					pDirent->CurrentItem	+= 1;
					retVal = 0;
					break;
				}
			} else {
				// Deleted or End of DIR
				if(tester == 0xE5) {
					retVal = -1; // Deleted Entry
					pDirent->CurrentItem += 1;
				} else {
					retVal = -2; // End of DIR
					break;
				}
			}
		}
	}
	FF_ReleaseBuffer(pIoman, pBuffer);

	return retVal;
}
/**
 *	@brief	Find's the first directory entry for the provided path.
 *
 *	All values recorded in pDirent must be preserved to and between calls to
 *	FF_FindNext().
 *
 *	@param	pIoman		FF_IOMAN object that was created by FF_CreateIOMAN().
 *	@param	pDirent		FF_DIRENT object to store the entry information.
 *	@param	path		String to of the path to the Dir being listed.
 *
 **/
FF_T_SINT8 FF_FindFirst(FF_IOMAN *pIoman, FF_DIRENT *pDirent, FF_T_INT8 *path) {

	if(!pIoman) {
		return FF_ERR_FAT_NULL_POINTER;
	}

	pDirent->DirCluster = FF_FindDir(pIoman, path);	// Get the directory cluster, if it exists.

	if(pDirent->DirCluster == 0) {
		return -2;
	}

	pDirent->CurrentCluster = 0;	// Ensure CurrentCluster is 0 so we can traverse clusters.
	pDirent->FileName[11] = '\0';
	pDirent->CurrentItem = 0;	// Set current item to 0
	pDirent->ProcessedLFN = FF_FALSE;

	FF_GetEntry(pIoman, pDirent->CurrentItem, pDirent->DirCluster, pDirent);

	return 0;
}

/**
 *	@brief	Get's the next Entry based on the data recorded in the FF_DIRENT object.
 *
 *	All values recorded in pDirent must be preserved to and between calls to
 *	FF_FindNext().
 *
 *	@param	pIoman		FF_IOMAN object that was created by FF_CreateIOMAN().
 *	@param	pDirent		FF_DIRENT object to store the entry information.
 *
 **/
FF_T_SINT8 FF_FindNext(FF_IOMAN *pIoman, FF_DIRENT *pDirent) {

	FF_T_SINT8 retVal = 0;
	if(!pIoman) {
		return FF_ERR_FAT_NULL_POINTER;
	}
	do {
		retVal = FF_GetEntry(pIoman, pDirent->CurrentItem, pDirent->DirCluster, pDirent);
	}while(retVal == -1);
	
	return retVal;
}

/**
 *	@brief	Opens a File for Access
 *
 *	@param	pIoman		FF_IOMAN object that was created by FF_CreateIOMAN().
 *	@param	path		Path to the File or object.
 *	@param	Mode		Access Mode required.
 *
 **/
FF_FILE *FF_Open(FF_IOMAN *pIoman, FF_T_INT8 *path, FF_T_INT8 *filename, FF_T_UINT8 Mode) {
	FF_FILE		*pFile;
	FF_DIRENT	Object;

	FF_T_UINT32 DirCluster, FileCluster;

	if(!pIoman) {
		return (FF_FILE *)NULL;
	}
	pFile = malloc(sizeof(FF_FILE));
	if(!pFile) {
		return (FF_FILE *)NULL;
	}
	
	DirCluster = FF_FindDir(pIoman, path);

	if(DirCluster) {
		FileCluster = FF_FindEntry(pIoman, DirCluster, filename, 0x00, &Object);
		if(FileCluster) {
			pFile->pIoman = pIoman;
			pFile->FilePointer = 0;
			pFile->ObjectCluster = Object.ObjectCluster;
			pFile->Filesize = Object.Filesize;
			pFile->CurrentCluster = 0;
			pFile->AddrCurrentCluster = pFile->ObjectCluster;
			return pFile;
		}
	}

	return (FF_FILE *)NULL;
}


/**
 *	@brief	Get's the next Entry based on the data recorded in the FF_DIRENT object.
 *
 *	@param	pFile		FF_FILE object that was created by FF_Open().
 *
 *	@return FF_TRUE if End of File was reached. FF_TRUE if not.
 *
 **/
FF_T_BOOL FF_isEOF(FF_FILE *pFile) {
	if(pFile->FilePointer >= pFile->Filesize) {
		return FF_TRUE;
	} else {
		return FF_FALSE;
	}
}


/**
 *	@brief	Equivalent to fread()
 *
 *	@param	pFile		FF_FILE object that was created by FF_Open().
 *	@param	ElementSize	The size of an element to read.
 *	@param	Count		The number of elements to read.
 *	@param	buffer		A pointer to a buffer of adequate size to be filled with the requested data.
 *
 *	@return Number of bytes read.
 *
 **/
FF_T_UINT32 FF_Read(FF_FILE *pFile, FF_T_UINT32 ElementSize, FF_T_UINT32 Count, FF_T_UINT8 *buffer) {
	FF_T_UINT32 Bytes = ElementSize * Count;
	FF_T_UINT32 BytesRead = 0;
	FF_PARTITION *pPart = pFile->pIoman->pPartition;
	FF_T_UINT32 fileLBA, fatEntry;
	FF_BUFFER	*pBuffer;
	FF_T_UINT32 numClusters;
	FF_T_UINT32 clusterNum		= pFile->FilePointer / ((pPart->SectorsPerCluster * pPart->BlkFactor) * 512);
	FF_T_UINT32 relClusterPos	= pFile->FilePointer % ((pPart->SectorsPerCluster * pPart->BlkFactor) * 512);
	FF_T_UINT32 bytesPerCluster = pPart->BlkSize * pPart->SectorsPerCluster;
	FF_T_UINT32 majorBlockNum	 = relClusterPos / pPart->BlkSize;
	FF_T_UINT32 relMajorBlockPos = relClusterPos % pPart->BlkSize;
	
	FF_T_UINT32 minorBlockNum	 = relMajorBlockPos / 512;
	FF_T_UINT32 relMinorBlockPos = relMajorBlockPos % 512;

	FF_T_UINT32 EOFadjust = 0,Sectors = Bytes / 512;

	if(FF_isEOF(pFile)) {
		return 0;
	}

	if(FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1) > pFile->CurrentCluster) {
		fatEntry = FF_getFatEntry(pFile->pIoman, pFile->AddrCurrentCluster);
		if(FF_isEndOfChain(pFile->pIoman, fatEntry)) {
			pFile->AddrCurrentCluster = pFile->ObjectCluster;
			// ERROR THIS SHOULD NOT OCCUR!
			return 0;
		} else {
			pFile->AddrCurrentCluster = fatEntry;
			pFile->CurrentCluster += 1;
		}
	}

	fileLBA = FF_Cluster2LBA(pFile->pIoman, pFile->AddrCurrentCluster);
	fileLBA = FF_getRealLBA(pFile->pIoman, fileLBA + FF_getMajorBlockNumber(pFile->pIoman, pFile->FilePointer, 1)) + FF_getMinorBlockNumber(pFile->pIoman, pFile->FilePointer, 1);

	
	if(Bytes > (pFile->Filesize - pFile->FilePointer)) {
		Bytes = pFile->Filesize - pFile->FilePointer;
	}

	if((Bytes + relMinorBlockPos) < 512) { // We have to memcpy from a buffer!

		pBuffer = FF_GetBuffer(pFile->pIoman, fileLBA, FF_MODE_READ);
		{
			memcpy(buffer, (pBuffer->pBuffer + relMinorBlockPos), Bytes);
		}
		FF_ReleaseBuffer(pFile->pIoman, pBuffer);

		pFile->FilePointer += Bytes;
		return Bytes;
	} else {

		if(relMinorBlockPos > 0) { // Not on a Sector Boundary. Memcpy First Lot
		// Read Across Multiple Clusters
			pBuffer = FF_GetBuffer(pFile->pIoman, fileLBA, FF_MODE_READ);
			{
				memcpy(buffer, (pBuffer->pBuffer + relMinorBlockPos), 512 - relMinorBlockPos);
			}
			FF_ReleaseBuffer(pFile->pIoman, pBuffer);

			buffer += (512 - relMinorBlockPos);
			fileLBA += 1;	// Next Copy just be from the Next LBA.
			// CARE ABOUT CLUSTER BOUNDARIES!
			Bytes -= 512 - relMinorBlockPos;
			BytesRead += 512 - relMinorBlockPos;
			pFile->FilePointer += 512 - relMinorBlockPos;
		}
		
		// Direct Copy :D Remaining Bytes > 512

		while(Bytes > bytesPerCluster) {
			// Direct Copy Size Remaining Cluster's!
			numClusters		 = Bytes / bytesPerCluster;
			relClusterPos	 = pFile->FilePointer % ((pPart->SectorsPerCluster * pPart->BlkFactor) * 512);
			majorBlockNum	 = relClusterPos / pPart->BlkSize;
			relMajorBlockPos = relClusterPos % pPart->BlkSize;
			minorBlockNum	 = relMajorBlockPos / 512;
			relMinorBlockPos = relMajorBlockPos % 512;

			if(FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1) > pFile->CurrentCluster) {
				fatEntry = FF_getFatEntry(pFile->pIoman, pFile->AddrCurrentCluster);
				if(FF_isEndOfChain(pFile->pIoman, fatEntry)) {
					pFile->AddrCurrentCluster = pFile->ObjectCluster;
					// ERROR THIS SHOULD NOT OCCUR!
					return 0;
				} else {
					pFile->AddrCurrentCluster = fatEntry;
					pFile->CurrentCluster += 1;
				}
			}
			
			fileLBA = FF_Cluster2LBA(pFile->pIoman, pFile->AddrCurrentCluster);
			fileLBA = FF_getRealLBA(pFile->pIoman, fileLBA + FF_getMajorBlockNumber(pFile->pIoman, pFile->FilePointer, 1)) + FF_getMinorBlockNumber(pFile->pIoman, pFile->FilePointer, 1);

			Sectors = (bytesPerCluster - relClusterPos) / 512;

			pFile->pIoman->pBlkDevice->fnReadBlocks(buffer, fileLBA, Sectors, pFile->pIoman->pBlkDevice->pParam);
			buffer += (bytesPerCluster - relClusterPos);
			Bytes -= (bytesPerCluster - relClusterPos);
			pFile->FilePointer += (bytesPerCluster - relClusterPos);
			BytesRead += (bytesPerCluster - relClusterPos);
		}

		relClusterPos	 = pFile->FilePointer % ((pPart->SectorsPerCluster * pPart->BlkFactor) * 512);
		majorBlockNum	 = relClusterPos / pPart->BlkSize;
		relMajorBlockPos = relClusterPos % pPart->BlkSize;
		minorBlockNum	 = relMajorBlockPos / 512;
		relMinorBlockPos = relMajorBlockPos % 512;

		if(FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1) > pFile->CurrentCluster) {
			fatEntry = FF_getFatEntry(pFile->pIoman, pFile->AddrCurrentCluster);
			if(FF_isEndOfChain(pFile->pIoman, fatEntry)) {
				pFile->AddrCurrentCluster = pFile->ObjectCluster;
				// ERROR THIS SHOULD NOT OCCUR!
				return 0;
			} else {
				pFile->AddrCurrentCluster = fatEntry;
				pFile->CurrentCluster += 1;
			}
		}
		
		fileLBA = FF_Cluster2LBA(pFile->pIoman, pFile->AddrCurrentCluster);
		fileLBA = FF_getRealLBA(pFile->pIoman, fileLBA + FF_getMajorBlockNumber(pFile->pIoman, pFile->FilePointer, 1)) + FF_getMinorBlockNumber(pFile->pIoman, pFile->FilePointer, 1);


		if(Bytes >= 512) {
			Sectors = Bytes / 512;
			pFile->pIoman->pBlkDevice->fnReadBlocks(buffer, fileLBA, Sectors, pFile->pIoman->pBlkDevice->pParam);
			Bytes -= Sectors * 512;
			buffer += Sectors * 512;
			BytesRead += Sectors * 512;
			pFile->FilePointer += Sectors * 512;
			fileLBA += Sectors;
		}

		if(Bytes > 0) {// Memcpy the remaining Bytes
			pBuffer = FF_GetBuffer(pFile->pIoman, fileLBA, FF_MODE_READ);
			{
				memcpy(buffer, (pBuffer->pBuffer), Bytes);
			}
			FF_ReleaseBuffer(pFile->pIoman, pBuffer);
		}

		buffer += Bytes;
		BytesRead += Bytes;
		pFile->FilePointer += Bytes;	
	
	}
	return BytesRead;
}


/**
 *	@brief	Equivalent to fgetc()
 *
 *	@param	pFile		FF_FILE object that was created by FF_Open().
 *
 *	@return The character that was read.
 *
 **/
FF_T_INT32 FF_GetC(FF_FILE *pFile) {
	FF_T_UINT32 fileLBA;
	FF_BUFFER *pBuffer;
	FF_PARTITION *pPart = pFile->pIoman->pPartition;
	FF_T_UINT8 retChar;
	FF_T_UINT32 fatEntry;

	FF_T_UINT32 clusterNum		= pFile->FilePointer / ((pPart->SectorsPerCluster * pPart->BlkFactor) * 512);
	FF_T_UINT32 relClusterPos	= pFile->FilePointer % ((pPart->SectorsPerCluster * pPart->BlkFactor) * 512);
	
	FF_T_UINT32 majorBlockNum	= relClusterPos / pPart->BlkSize;
	FF_T_UINT32 relMajorBlockPos = relClusterPos % pPart->BlkSize;
	
	FF_T_UINT32 minorBlockNum = relMajorBlockPos / 512;
	FF_T_UINT32 relMinorBlockPos = relMajorBlockPos % 512;

	
	if(pFile->FilePointer >= pFile->Filesize) {
		return -1; // EOF!	
	}
	
	if(clusterNum > pFile->CurrentCluster) {
		fatEntry = FF_getFatEntry(pFile->pIoman, pFile->AddrCurrentCluster);
		if(FF_isEndOfChain(pFile->pIoman, fatEntry)) {
			pFile->AddrCurrentCluster = pFile->ObjectCluster;
			// ERROR THIS SHOULD NOT OCCUR!
			return -2;
		} else {
			pFile->AddrCurrentCluster = fatEntry;
			pFile->CurrentCluster += 1;
		}
	}

	fileLBA = FF_Cluster2LBA(pFile->pIoman, pFile->AddrCurrentCluster);

	fileLBA = FF_getRealLBA(pFile->pIoman, fileLBA + majorBlockNum) + minorBlockNum;

	pBuffer = FF_GetBuffer(pFile->pIoman, fileLBA, FF_MODE_READ);
	{
		retChar = pBuffer->pBuffer[relMinorBlockPos];
	}
	FF_ReleaseBuffer(pFile->pIoman, pBuffer);

	pFile->FilePointer += 1;

	return (FF_T_INT32) retChar;
}


/**
 *	@brief	Equivalent to fclose()
 *
 *	@param	pFile		FF_FILE object that was created by FF_Open().
 *
 *	@return 0 on sucess.
 *
 **/
FF_T_SINT8 FF_Close(FF_FILE *pFile) {
	// If file written, flush to disk

	free(pFile);
	// Simply free the pointer!
	return 0;
}


