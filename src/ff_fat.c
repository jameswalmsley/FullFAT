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
#include <string.h>

FF_T_UINT32 FF_getRealLBA(FF_IOMAN *pIoman, FF_T_UINT32 LBA) {
	return LBA * pIoman->pPartition->BlkFactor;
}

FF_T_UINT32 FF_Cluster2LBA(FF_IOMAN *pIoman, FF_T_UINT32 Cluster) {
	FF_T_UINT32 lba = 0;
	FF_PARTITION *pPart;
	if(pIoman) {
		pPart = pIoman->pPartition;
		if(pPart->Type == FF_T_FAT32) {
			lba = pPart->ClusterBeginLBA + ((Cluster - 2) * pPart->SectorsPerCluster);
		} else {
			lba = pPart->ClusterBeginLBA + ((Cluster - 1) * pPart->SectorsPerCluster);
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

// Calculates the sector number of the FAT table required for entry corresponding to nCluster.
FF_T_UINT32 FF_getFATSector(FF_IOMAN *pIoman, FF_T_UINT32 nCluster) {

	FF_T_UINT32 nSector = 0;
	FF_T_UINT32 nEntries = 0;

	if(pIoman->pPartition->Type == FF_T_FAT32) {
		nEntries = (pIoman->pPartition->BlkSize / 4);
	} else {
		nEntries = (pIoman->pPartition->BlkSize / 2);
	}

	nSector  = pIoman->pPartition->FatBeginLBA;
	nSector += nCluster / nEntries; // + 2
	return nSector;
}

// Release all buffers before calling this!
FF_T_UINT32 FF_getFatEntry(FF_IOMAN *pIoman, FF_T_UINT32 nCluster) {
	FF_T_UINT32 fatEntry = 0;
	FF_BUFFER *pBuffer = 0;
	FF_T_UINT32 LBAadjust;
	FF_T_UINT32 FatSector = FF_getRealLBA(pIoman, FF_getFATSector(pIoman, nCluster));

	FF_T_UINT32 relClusterNum;
	
	if(pIoman->pPartition->Type == FF_T_FAT32) {
		LBAadjust = (nCluster / (512 / 4));
		relClusterNum = nCluster % (512 / 4);
	} else {
		LBAadjust = (nCluster / (512 / 2));
		relClusterNum = nCluster % (512 / 2);
	}

	pBuffer = FF_GetBuffer(pIoman, FatSector, FF_MODE_READ);

	if(pIoman->pPartition->Type == FF_T_FAT32) {
		fatEntry = FF_getLong(pBuffer->pBuffer, (relClusterNum * 4));
	} else {
		fatEntry = (FF_T_UINT32) FF_getShort(pBuffer->pBuffer, (relClusterNum * 2));
	}

	FF_ReleaseBuffer(pIoman, pBuffer);

	return fatEntry;
}

FF_T_BOOL FF_isEndOfChain(FF_IOMAN *pIoman, FF_T_UINT32 fatEntry) {
	FF_T_BOOL result = FF_FALSE;

	if(pIoman->pPartition->Type == FF_T_FAT32) {
		if(fatEntry >= 0x0ffffff8) {
			result = FF_TRUE;
		}
	} else {
		if(fatEntry >= 0x0000fff8) {
			result = FF_TRUE;
		}
	}

	return result;
}

// Finds Entries within a Sector of a Directory
// Will Swap out the Buffer if it has to process an LFN across 2 sectors.
FF_T_UINT32 FF_FindEntryInSector(FF_IOMAN *pIoman, FF_BUFFER *pBuffer, FF_T_INT8 *name, FF_T_UINT8 pa_Attrib, FF_DIRENT *pDirent) {
	FF_T_UINT16 numEntries = 512 / 32;
	FF_T_UINT16 myShort;
	FF_T_UINT32 itemCluster = 0;
	FF_T_UINT8 i, numLFNs;
	FF_T_UINT8 nameLength;
	FF_T_UINT8 Attrib = 0x00;
	FF_T_UINT8 test;			// Used for checking if an entry was deleted.
	
	FF_T_INT8 shortName[13];
	
	nameLength = strlen(name);
	
	for(i = 0; i < numEntries; i++) {
		test = FF_getChar(pBuffer->pBuffer, (32*i));	//
		if(test != 0xe5 && test != 0x00) {
			
			Attrib = FF_getChar(pBuffer->pBuffer, (FF_FAT_DIRENT_ATTRIB + (32 * i)));
			if((Attrib & FF_FAT_ATTR_LFN) == FF_FAT_ATTR_LFN) {
				// Process LFN!
#ifdef FF_LFN_SUPPORT

#endif	
				// Skip Past the LFN!
				if(test == 0x41) {
					numLFNs = 1;
				} else {
					numLFNs = (test & ~0x40);
				}
				i += numLFNs;
				if(i >= numEntries) {
					return 0;	// Real Entry is in the next sector!
				}

				// Get Real Attribute 
				Attrib = FF_getChar(pBuffer->pBuffer, (FF_FAT_DIRENT_ATTRIB + (32 * i)));
			}
			

			if((Attrib & pa_Attrib) == pa_Attrib) {
				memcpy(shortName, (pBuffer->pBuffer + (32 * i)), 11);
				if(strncmp(name, shortName, nameLength) == 0) {
					// Item Found! :D
					myShort = FF_getShort(pBuffer->pBuffer, (FF_FAT_DIRENT_CLUS_HIGH + (32 * i)));
					itemCluster = (FF_T_UINT32) (myShort << 16);
					myShort = FF_getShort(pBuffer->pBuffer, (FF_FAT_DIRENT_CLUS_LOW + (32 * i)));
					itemCluster |= myShort;
					if(pDirent) {
						pDirent->Attrib = Attrib;
						pDirent->ObjectCluster = itemCluster;
						strncpy(pDirent->FileName, (pBuffer->pBuffer + (32 * i)), 11);	
						pDirent->Filesize = FF_getLong(pBuffer->pBuffer, (FF_FAT_DIRENT_FILESIZE + (32 * i)));
					}

					if(!itemCluster) {
						return 1;	// Nothing can have this cluster number!
					} else {
						return itemCluster;
					}
				}
			}
		}
	}

	return 0;
}


// Searches for entries from the beginning of a sector.
// Returns the Cluster Number of the found Entry
//
FF_T_UINT32 FF_FindEntry(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_T_INT8 *name, FF_T_UINT8 pa_Attrib, FF_DIRENT *pa_pDirent) {
	FF_PARTITION *pPart = pIoman->pPartition;

	FF_T_UINT32 currentCluster = DirCluster; // The cluster of the Dir we are working on.
	FF_T_UINT32 dirLBA, itemCluster;
	FF_T_UINT16 i,x,y = 0;
	FF_T_UINT16 numEntries = pIoman->pPartition->BlkSize / 32;
	FF_BUFFER *pBuffer;
	FF_T_UINT32 fatEntry = 0;
	FF_T_BOOL isEndOfDir = FF_FALSE;	// Flag true if the End of the Directory was reached!
		
	while(!FF_isEndOfChain(pIoman, fatEntry)) {	// Not at End of fat chain!

		fatEntry = FF_getFatEntry(pIoman, currentCluster);
		
		for(i = 0; i < pPart->SectorsPerCluster; i++) { // Process Each Sector

			// Process Each sector of the Dirent!
			dirLBA = FF_getRealLBA(pIoman, (FF_Cluster2LBA(pIoman, currentCluster) + i));
			
			// Loop for the BlockFactor
			for(x = 0; x < pIoman->pPartition->BlkFactor; x++) {
				
				pBuffer = FF_GetBuffer(pIoman, dirLBA + x, FF_MODE_READ);

				itemCluster = FF_FindEntryInSector(pIoman, pBuffer, name, pa_Attrib, pa_pDirent);

				FF_ReleaseBuffer(pIoman, pBuffer);
				
				if(itemCluster) {
					return itemCluster;
				}
			}	

			if(isEndOfDir == FF_TRUE) { // End of the Directory was found. Finish!
				break;
			}
		}

		if(isEndOfDir == FF_TRUE) { // End of the Directory was found. Finish!
				break;
		}
		currentCluster = fatEntry;
	}

	return 0;

}

// returns the Cluster of the specified directory
FF_T_UINT32 FF_FindDir(FF_IOMAN *pIoman, FF_T_INT8 *path) {

	FF_T_UINT32 dirCluster = pIoman->pPartition->RootDirCluster;

    char mypath[260];

	FF_T_INT8 *token;

	FF_T_UINT16	it = 0, mod = 0;		// Re-entrancy Variables for FF_strtok()

	FF_T_UINT32 pathLength = strlen(path);

	if(pathLength == 1) {	// Must be the root dir! (/ or \)
		return pIoman->pPartition->RootDirCluster;
	}

	strcpy(mypath, path);

	token = FF_strtok(mypath, &it, &mod); // Tokenise Path, thread-safely

	while(token != NULL) {
		dirCluster = FF_FindEntry(pIoman, dirCluster, token, FF_FAT_ATTR_DIR, NULL);
		token = FF_strtok(mypath, &it, &mod);
	}

	return dirCluster;
}

FF_T_SINT8 FF_getLFN(FF_IOMAN *pIoman, FF_BUFFER *pBuffer, FF_T_UINT32 Entry, FF_T_INT8 *filename) {

	FF_T_UINT8	 	numLFNs;
	FF_T_UINT8	 	retLFNs = 0;
	FF_T_UINT16		lenlfn = 0;
	FF_T_UINT8		tester;
	FF_T_UINT16		i,y;
	FF_T_UINT32		OriginalSector = pBuffer->Sector, Sector = pBuffer->Sector;

	FF_T_UINT8		*buffer = pBuffer->pBuffer;
	
	tester = FF_getChar(pBuffer->pBuffer, (Entry * 32));
	numLFNs = tester & ~0x40;

	while(numLFNs > 0) {
		if(numLFNs + Entry > 15) {
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

		Entry++;
		numLFNs--;
	}

	/*while(numLFNs != 0) {
		numLFNs --;
		lfn = (FF_LFN *)cache;
		for(i = 0, y = 0; i < 5; i++, y += 2) {
			filename[i + (numLFNs * 13)] = lfn->Name_0[y];
			lenlfn++;
		}
		for(i = 0, y = 0; i < 6; i++, y += 2) {
			filename[i + (numLFNs * 13) + 5] = lfn->Name_1[y];
			lenlfn++;
		}
		for(i = 0, y = 0; i < 2; i++, y += 2) {
			filename[i + (numLFNs * 13) + 11] = lfn->Name_2[y];
			lenlfn++;
		}
		cache++;
		retLFNs++;
	}
	filename[lenlfn] = '\0'; // String Terminator required!
	return retLFNs;*/
	return numLFNs;
}

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
	FF_T_UINT32     CurrentCluster = DirCluster;
	FF_T_UINT16		myShort, numLFNs;	///< 16 bits for calculating item cluster.
	FF_T_UINT32		fatEntry = 0;	///< Used for following Cluster Chain
	FF_T_UINT32		itemLBA;
	FF_PARTITION	*pPart = pIoman->pPartition;	///< Used to make code easier to read.
	FF_BUFFER		*pBuffer;	///< Buffer for acquired sector.

	// Calculate which cluster of the chain the entry is in.
	FF_T_UINT32 clusterChainNumber	= nEntry / (512 * (pPart->SectorsPerCluster * pPart->BlkFactor) / 32);
	FF_T_UINT32 relClusterEntry		= nEntry % (512 * (pPart->SectorsPerCluster * pPart->BlkFactor) / 32);
	
	FF_T_UINT32 majorBlockNumber	= relClusterEntry / (pPart->BlkSize / 32);
	FF_T_UINT32 relmajorBlockEntry	= relClusterEntry % (pPart->BlkSize / 32);

	FF_T_UINT32 minorBlockNumber	= relmajorBlockEntry / (512 / 32);
	FF_T_UINT32 minorBlockEntry		= relmajorBlockEntry % (512 / 32);
	
	// Lets follow the chain to find its real number
	
	if(clusterChainNumber > pDirent->CurrentCluster) {

		fatEntry = FF_getFatEntry(pIoman, DirCluster);
		
		if(FF_isEndOfChain(pIoman, fatEntry)) {
			CurrentCluster = DirCluster;
			// ERROR THIS SHOULD NOT OCCUR!
		} else {
			CurrentCluster = fatEntry;
			pDirent->DirCluster = fatEntry;
		}
		
		pDirent->CurrentCluster += 1;
	}

	itemLBA = FF_Cluster2LBA(pIoman, CurrentCluster) + majorBlockNumber;
	itemLBA = FF_getRealLBA(pIoman, itemLBA) + minorBlockNumber;

	pBuffer = FF_GetBuffer(pIoman, itemLBA, FF_MODE_READ);
	{
		
		for(;minorBlockEntry < (512 / 32); minorBlockEntry++) {
			tester = FF_getChar(pBuffer->pBuffer, (minorBlockEntry * 32));
			if(tester != 0xe5 && tester != 0x00) {
				
				pDirent->Attrib = FF_getChar(pBuffer->pBuffer, (FF_FAT_DIRENT_ATTRIB + (32 * minorBlockEntry)));
				if((pDirent->Attrib & FF_FAT_ATTR_LFN) == FF_FAT_ATTR_LFN) {
					numLFNs = (tester & ~0x40);
#ifdef FF_LFN_SUPPORT
					// May Get the Next Buffer if it needs to!
					FF_getLFN(pIoman, pBuffer, minorBlockEntry, pDirent->FileName);
					pDirent->ProcessedLFN = FF_TRUE;
#endif
					pDirent->CurrentItem += numLFNs;
					minorBlockEntry += (numLFNs -1);	// (LFN's -1 because the loop will increment once also!)
					retVal = -1;

				} else {
					if(pDirent->ProcessedLFN == FF_TRUE) {
						pDirent->ProcessedLFN = FF_FALSE;
					} else {
						if(pDirent->Attrib == FF_FAT_ATTR_DIR || pDirent->Attrib == FF_FAT_ATTR_VOLID) {
							strncpy(pDirent->FileName, (pBuffer->pBuffer + (32 * minorBlockEntry)), 11);
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
 **/
FF_T_SINT8 FF_FindFirst(FF_IOMAN *pIoman, FF_DIRENT *pDirent, FF_T_INT8 *path) {

	if(!pIoman) {
		return FF_ERR_FAT_NULL_POINTER;
	}

	pDirent->DirCluster = FF_FindDir(pIoman, path);


	if(pDirent->DirCluster == 0) {
		return -2;
	}

	pDirent->CurrentCluster = 0;
	pDirent->FileName[11] = '\0';
	pDirent->CurrentItem = 0;	// Set current item to 0
	pDirent->ProcessedLFN = FF_FALSE;
//	pDirent->isSectorOffsetSet = FF_FALSE;

	FF_GetEntry(pIoman, pDirent->CurrentItem, pDirent->DirCluster, pDirent);
	//FF_FindEntry(pIoman, pDirent->CurrentCluster, "", 0x00, pDirent);

	return 0;
}

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

FF_T_BOOL FF_isEOF(FF_FILE *pFile) {
	if(pFile->FilePointer >= pFile->Filesize) {
		return FF_TRUE;
	} else {
		return FF_FALSE;
	}
}

FF_T_UINT32 FF_Read(FF_FILE *pFile, FF_T_UINT32 ElementSize, FF_T_UINT32 Count, FF_T_UINT8 *buffer) {
	FF_T_UINT32 Bytes = ElementSize * Count;
	FF_T_UINT32 BytesRead = 0;
	FF_PARTITION *pPart = pFile->pIoman->pPartition;
	FF_T_UINT32 fileLBA, fatEntry;
	FF_BUFFER	*pBuffer;
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

	if(clusterNum > pFile->CurrentCluster) {
		fatEntry = FF_getFatEntry(pFile->pIoman, pFile->AddrCurrentCluster);
		if(FF_isEndOfChain(pFile->pIoman, fatEntry)) {
			pFile->AddrCurrentCluster = pFile->ObjectCluster;
			// ERROR THIS SHOULD NOT OCCUR!
		} else {
			pFile->AddrCurrentCluster = fatEntry;
			pFile->CurrentCluster += 1;
		}
	}

	fileLBA = FF_Cluster2LBA(pFile->pIoman, pFile->AddrCurrentCluster);
	fileLBA = FF_getRealLBA(pFile->pIoman, fileLBA + majorBlockNum) + minorBlockNum;

	
	if(Bytes > (pFile->Filesize - pFile->FilePointer)) {
		Bytes = pFile->Filesize - pFile->FilePointer;
	}

	if(Bytes < 512) { // We have to memcpy from a buffer!

		pBuffer = FF_GetBuffer(pFile->pIoman, fileLBA, FF_MODE_READ);
		{
			memcpy(buffer, (pBuffer->pBuffer + relMinorBlockPos), Bytes);
		}
		FF_ReleaseBuffer(pFile->pIoman, pBuffer);

		pFile->FilePointer += Bytes;
		return Bytes;
	} else {

		if(relMinorBlockPos >= 0) { // Not on a Sector Boundary. Memcpy First Lot
		// Read Across Multiple Clusters
			pBuffer = FF_GetBuffer(pFile->pIoman, fileLBA, FF_MODE_READ);
			{
				memcpy(buffer, (pBuffer->pBuffer + relMinorBlockPos), 512 - relMinorBlockPos);
			}
			FF_ReleaseBuffer(pFile->pIoman, pBuffer);

			buffer += (512 - relMinorBlockPos);
		}
		fileLBA += 1;	// Next Copy just be from the Next LBA.
		// CARE ABOUT CLUSTER BOUNDARIES!
		Bytes -= 512 - relMinorBlockPos;
		BytesRead += 512 - relMinorBlockPos;
		
		// Direct Copy :D Remaining Bytes > 512

		while(Bytes > bytesPerCluster) {
			// Direct Copy Size Remaining Cluster's!
			

		}

		if(Bytes >= 512) {
			Sectors = Bytes / 512;
			pFile->pIoman->pBlkDevice->fnReadBlocks(buffer, fileLBA, Sectors, pFile->pIoman->pBlkDevice->pParam);
			Bytes -= Sectors * 512;
			buffer += Sectors * 512;
			BytesRead += Sectors * 512;
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
				
		pFile->FilePointer += BytesRead;
	}


			//pIoman->pBlkDevice->fnReadBlocks(pBuffer, Sector, 1, pIoman->pBlkDevice->pParam);


	return BytesRead;
}

FF_T_UINT8 FF_GetC(FF_FILE *pFile) {
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

	if(clusterNum > pFile->CurrentCluster) {
		fatEntry = FF_getFatEntry(pFile->pIoman, pFile->AddrCurrentCluster);
		if(FF_isEndOfChain(pFile->pIoman, fatEntry)) {
			pFile->AddrCurrentCluster = pFile->ObjectCluster;
			// ERROR THIS SHOULD NOT OCCUR!
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

	return retChar;
}

FF_T_SINT8 FF_Close(FF_FILE *pFile) {
	// If file written, flush to disk
	free(pFile);
	// Simply free the pointer!
	return 0;
}


