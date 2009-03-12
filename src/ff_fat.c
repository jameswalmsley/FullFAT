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
FF_T_UINT32 FF_getFatEntry(FF_IOMAN *pIoman, FF_T_UINT32 nCluster) {

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
				F12short[0] = FF_getChar(pBuffer->pBuffer, (pIoman->BlkSize - 1));				
			}
			FF_ReleaseBuffer(pIoman, pBuffer);
			// Second Buffer get the first Byte in buffer (second byte of out address)!
			pBuffer = FF_GetBuffer(pIoman, FatSector + LBAadjust + 1, FF_MODE_READ);
			{
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
 **/
FF_T_UINT32 FF_FindEntry(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_T_INT8 *name, FF_T_UINT8 pa_Attrib, FF_DIRENT *pDirent) {
	
	FF_T_INT32		retVal;
	FF_DIRENT		MyDir;
	FF_T_INT8		Filename[FF_MAX_FILENAME];
	FF_T_UINT16		fnameLen;
	FF_T_UINT16		compareLength;
	FF_T_UINT16		nameLen = (FF_T_UINT16) strlen(name);
	
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
			fnameLen = (FF_T_UINT16) strlen(Filename);
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

/**
 *	@private
 **/
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
/**
 *	@private
 **/
FF_T_SINT8 FF_getLFN(FF_IOMAN *pIoman, FF_BUFFER *pBuffer, FF_DIRENT *pDirent, FF_T_INT8 *filename) {

	FF_T_UINT8	 	numLFNs;
	FF_T_UINT16		lenlfn = 0;
	FF_T_UINT8		tester;
	FF_T_UINT16		i,y;
	FF_T_UINT32		CurrentCluster;
	FF_T_UINT32		fatEntry;
	FF_T_UINT8		*buffer = pBuffer->pBuffer;
	FF_T_UINT32		Sector = pBuffer->Sector;
	FF_T_UINT32		Entry		= FF_getMinorBlockEntry(pIoman, pDirent->CurrentItem, 32);

	tester = FF_getChar(pBuffer->pBuffer, (FF_T_UINT16)(Entry * 32));
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

	filename[lenlfn] = '\0';

	return 0;
}
#endif

/**
 *	@private
 **/
void FF_ProcessShortName(FF_T_INT8 *name) {
	FF_T_INT8	shortName[13];
	FF_T_UINT8	i;
	memcpy(shortName, name, 11);
	
	for(i = 0; i < 8; i++) {
		if(shortName[i] == 0x20) {
			name[i] = '\0';
			break;
		}
		name[i] = shortName[i];
	}

	if(i == 7) {
		i++;
	}

	if(shortName[8] != 0x20){
		name[i] = '.';
		name[i+1] = shortName[8];
		name[i+2] = shortName[9];
		name[i+3] = shortName[10];
		name[i+4] = '\0';
		for(i = 8; i < 11; i++) {
			if(name[i] == 0x20) {
				name[i] = '\0';
				break;
			}
		}
	} else {
		name[i] = '\0';
	}


}

/**
 *	@private
 **/
FF_T_SINT8 FF_GetEntry(FF_IOMAN *pIoman, FF_T_UINT32 nEntry, FF_T_UINT32 DirCluster, FF_DIRENT *pDirent) {
	
	FF_T_UINT8		tester;			///< Unsigned byte for testing if dir is deleted
	FF_T_SINT8		retVal = 0;		///< Return Value for Function
	FF_T_UINT32		CurrentCluster = DirCluster;
	FF_T_UINT16		myShort, numLFNs;	///< 16 bits for calculating item cluster.
	FF_T_UINT32		fatEntry = 0;	///< Used for following Cluster Chain
	FF_T_UINT32		itemLBA;
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
		for(;minorBlockEntry < (pIoman->BlkSize / 32); minorBlockEntry++) {
			tester = FF_getChar(pBuffer->pBuffer, (FF_T_UINT16)(minorBlockEntry * 32));
			if(tester != 0xE5 && tester != 0x00) {
				
				pDirent->Attrib = FF_getChar(pBuffer->pBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB + (32 * minorBlockEntry)));
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
						if((pDirent->Attrib & FF_FAT_ATTR_DIR) == FF_FAT_ATTR_DIR || (pDirent->Attrib & FF_FAT_ATTR_VOLID) == FF_FAT_ATTR_VOLID) {
							strncpy(pDirent->FileName, (FF_T_INT8 *)(pBuffer->pBuffer + (32 * minorBlockEntry)), 11);
							if((pDirent->Attrib & FF_FAT_ATTR_DIR) == FF_FAT_ATTR_DIR) {
								for(i = 0; i < 11; i++) {
									if(pDirent->FileName[i] == 0x20) {
										break;
									}
								}
								pDirent->FileName[i] = '\0';	
							}
							if((pDirent->Attrib & FF_FAT_ATTR_VOLID) == FF_FAT_ATTR_VOLID) {
								pDirent->FileName[11] = '\0';
							}
							
						} else {
							strncpy(pDirent->FileName, (FF_T_INT8 *)(pBuffer->pBuffer + (32 * minorBlockEntry)), 11);
							FF_ProcessShortName(pDirent->FileName);
						}
					}
					myShort					 = FF_getShort(pBuffer->pBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_HIGH + (32 * minorBlockEntry)));
					pDirent->ObjectCluster   = (FF_T_UINT32) (myShort << 16);
					myShort					 = FF_getShort(pBuffer->pBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_LOW + (32 * minorBlockEntry)));
					pDirent->ObjectCluster  |= myShort;
					pDirent->Filesize		 = FF_getLong(pBuffer->pBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_FILESIZE + (32 * minorBlockEntry)));
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
 *	@public
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
 *	@public
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
 *	@public
 *	@brief	Opens a File for Access
 *
 *	@param	pIoman		FF_IOMAN object that was created by FF_CreateIOMAN().
 *	@param	path		Path to the File or object.
 *	@param	Mode		Access Mode required.
 *
 **/
FF_FILE *FF_Open(FF_IOMAN *pIoman, FF_T_INT8 *path, FF_T_UINT8 Mode) {
	FF_FILE		*pFile;
	FF_DIRENT	Object;
	FF_T_UINT32 DirCluster, FileCluster;
	FF_T_INT8	filename[FF_MAX_FILENAME];

	FF_T_UINT16	i;

	FF_T_INT8 *mypath;

	if(!pIoman) {
		return (FF_FILE *)NULL;
	}
	pFile = malloc(sizeof(FF_FILE));
	if(!pFile) {
		return (FF_FILE *)NULL;
	}

	i = (FF_T_UINT16) strlen(path);

	while(i > 0) {
		if(path[i] == '\\' || path[i] == '/') {
			break;
		}
		i--;
	}


	strncpy(filename, (path + i + 1), FF_MAX_FILENAME);

	mypath = (FF_T_INT8 *) &path;	// Allows us to modify a constant path, without an exception

	mypath[i + 1] = '\0';
				
	DirCluster = FF_FindDir(pIoman, mypath);
	
	if(DirCluster) {
		FileCluster = FF_FindEntry(pIoman, DirCluster, filename, 0x00, &Object);
		if(!FileCluster) {	// If 0 was returned, it might be because the file has no allocated cluster
			FF_tolower(Object.FileName, FF_MAX_FILENAME);
			FF_tolower(filename, FF_MAX_FILENAME);
			if(Object.Filesize == 0 && strcmp(filename, Object.FileName) == 0) {
				// The file really was found!
				FileCluster = 1;
			}
		}
		if(FileCluster) {
			pFile->pIoman = pIoman;
			pFile->FilePointer = 0;
			pFile->ObjectCluster = Object.ObjectCluster;
			pFile->Filesize = Object.Filesize;
			pFile->CurrentCluster = 0;
			pFile->AddrCurrentCluster = pFile->ObjectCluster;
			pFile->Mode = Mode;
			return pFile;
		}
	}

	return (FF_FILE *)NULL;
}


/**
 *	@public
 *	@brief	Get's the next Entry based on the data recorded in the FF_DIRENT object.
 *
 *	@param	pFile		FF_FILE object that was created by FF_Open().
 *
 *	@return FF_TRUE if End of File was reached. FF_FALSE if not.
 *	@return FF_FALSE if a null pointer was provided.
 *
 **/
FF_T_BOOL FF_isEOF(FF_FILE *pFile) {
	if(!pFile) {
		return FF_FALSE;
	}
	if(pFile->FilePointer >= pFile->Filesize) {
		return FF_TRUE;
	} else {
		return FF_FALSE;
	}
}


/**
 *	@public
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
	FF_T_UINT32 relClusterPos	= pFile->FilePointer % ((pPart->SectorsPerCluster * pPart->BlkFactor) * pFile->pIoman->BlkSize);
	FF_T_UINT32 bytesPerCluster = pPart->BlkSize * pPart->SectorsPerCluster;
	FF_T_UINT32 majorBlockNum	 = relClusterPos / pPart->BlkSize;
	FF_T_UINT32 relMajorBlockPos = relClusterPos % pPart->BlkSize;
	
	FF_T_UINT32 minorBlockNum	 = relMajorBlockPos / pFile->pIoman->BlkSize;
	FF_T_UINT32 relMinorBlockPos = relMajorBlockPos % pFile->pIoman->BlkSize;

	FF_T_UINT32 Sectors = Bytes / pFile->pIoman->BlkSize;

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

	if((Bytes + relMinorBlockPos) < pFile->pIoman->BlkSize) { // We have to memcpy from a buffer!

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
				memcpy(buffer, (pBuffer->pBuffer + relMinorBlockPos), pFile->pIoman->BlkSize - relMinorBlockPos);
			}
			FF_ReleaseBuffer(pFile->pIoman, pBuffer);

			buffer += (pFile->pIoman->BlkSize - relMinorBlockPos);
			fileLBA += 1;	// Next Copy just be from the Next LBA.
			// CARE ABOUT CLUSTER BOUNDARIES!
			Bytes -= pFile->pIoman->BlkSize - relMinorBlockPos;
			BytesRead += pFile->pIoman->BlkSize - relMinorBlockPos;
			pFile->FilePointer += pFile->pIoman->BlkSize - relMinorBlockPos;
		}
		
		// Direct Copy :D Remaining Bytes > IOMAN BlkSIze

		while(Bytes > bytesPerCluster) {
			// Direct Copy Size Remaining Cluster's!
			numClusters		 = Bytes / bytesPerCluster;
			relClusterPos	 = pFile->FilePointer % ((pPart->SectorsPerCluster * pPart->BlkFactor) * pFile->pIoman->BlkSize);
			majorBlockNum	 = relClusterPos / pPart->BlkSize;
			relMajorBlockPos = relClusterPos % pPart->BlkSize;
			minorBlockNum	 = relMajorBlockPos / pFile->pIoman->BlkSize;
			relMinorBlockPos = relMajorBlockPos % pFile->pIoman->BlkSize;

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

			Sectors = (bytesPerCluster - relClusterPos) / pFile->pIoman->BlkSize;

			pFile->pIoman->pBlkDevice->fnReadBlocks(buffer, fileLBA, Sectors, pFile->pIoman->pBlkDevice->pParam);
			buffer += (bytesPerCluster - relClusterPos);
			Bytes -= (bytesPerCluster - relClusterPos);
			pFile->FilePointer += (bytesPerCluster - relClusterPos);
			BytesRead += (bytesPerCluster - relClusterPos);
		}

		relClusterPos	 = pFile->FilePointer % ((pPart->SectorsPerCluster * pPart->BlkFactor) * pFile->pIoman->BlkSize);
		majorBlockNum	 = relClusterPos / pPart->BlkSize;
		relMajorBlockPos = relClusterPos % pPart->BlkSize;
		minorBlockNum	 = relMajorBlockPos / pFile->pIoman->BlkSize;
		relMinorBlockPos = relMajorBlockPos % pFile->pIoman->BlkSize;

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


		if(Bytes >= pFile->pIoman->BlkSize) {
			Sectors = Bytes / pFile->pIoman->BlkSize;
			pFile->pIoman->pBlkDevice->fnReadBlocks(buffer, fileLBA, Sectors, pFile->pIoman->pBlkDevice->pParam);
			Bytes -= Sectors * pFile->pIoman->BlkSize;
			buffer += Sectors * pFile->pIoman->BlkSize;
			BytesRead += Sectors * pFile->pIoman->BlkSize;
			pFile->FilePointer += Sectors * pFile->pIoman->BlkSize;
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
 *	@public
 *	@brief	Equivalent to fgetc()
 *
 *	@param	pFile		FF_FILE object that was created by FF_Open().
 *
 *	@return The character that was read (cast as a 32-bit interger). -1 on EOF.
 *	@return -2 If a null file pointer was provided.
 *
 **/
FF_T_INT32 FF_GetC(FF_FILE *pFile) {
	FF_T_UINT32 fileLBA;
	FF_BUFFER *pBuffer;
	FF_PARTITION *pPart = pFile->pIoman->pPartition;
	FF_T_UINT8 retChar;
	FF_T_UINT32 fatEntry;

	/*
		These can be replaced with the ff_blk.c functions! 
		Will be done for the 1.0 release.
	*/
	FF_T_UINT32 clusterNum		= pFile->FilePointer / ((pPart->SectorsPerCluster * pPart->BlkFactor) * pFile->pIoman->BlkSize);
	FF_T_UINT32 relClusterPos	= pFile->FilePointer % ((pPart->SectorsPerCluster * pPart->BlkFactor) * pFile->pIoman->BlkSize);
	
	FF_T_UINT32 majorBlockNum	= relClusterPos / pPart->BlkSize;
	FF_T_UINT32 relMajorBlockPos = relClusterPos % pPart->BlkSize;
	
	FF_T_UINT32 minorBlockNum = relMajorBlockPos / pFile->pIoman->BlkSize;
	FF_T_UINT32 relMinorBlockPos = relMajorBlockPos % pFile->pIoman->BlkSize;

	if(!pFile) {
		return -2;
	}
	
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
 *	@public
 *	@brief	Equivalent to fseek()
 *
 *	@param	pFile		FF_FILE object that was created by FF_Open().
 *	@param	Offset		An integer (+/-) to seek to, from the specified origin.
 *	@param	Origin		Where to seek from. (FF_SEEK_SET seek from start, FF_SEEK_CUR seek from current position, or FF_SEEK_END seek from end of file).
 *
 *	@return 0 on Sucess, 
 *	@return -2 if offset results in an invalid position in the file. 
 *	@return -1 if a FF_FILE pointer was not recieved.
 *	@return -3 if an invalid origin was provided.
 *	
 **/
FF_T_SINT8 FF_Seek(FF_FILE *pFile, FF_T_SINT32 Offset, FF_T_INT8 Origin) {
	if(!pFile) {
		return -1;
	}

	switch(Origin) {
		case FF_SEEK_SET:
			if(Offset <= pFile->Filesize && Offset >= 0) {
				pFile->FilePointer = Offset;
			} else {
				return -2;
			}
			break;

		case FF_SEEK_CUR:
			if((Offset + pFile->FilePointer) <= pFile->Filesize && (Offset + (FF_T_SINT32) pFile->FilePointer) >= 0) {
				pFile->FilePointer = Offset + pFile->FilePointer;
			} else {
				return -2;
			}
			break;
	
		case FF_SEEK_END:
			if((Offset + (FF_T_SINT32) pFile->Filesize) >= 0 && (Offset + pFile->Filesize) <= pFile->Filesize) {
				pFile->FilePointer = Offset + pFile->Filesize;
			} else {
				return -2;
			}
			break;

		default:
			return -3;
		
	}

	return 0;
}


/**
 *	@public
 *	@brief	Equivalent to fclose()
 *
 *	@param	pFile		FF_FILE object that was created by FF_Open().
 *
 *	@return 0 on sucess.
 *	@return -1 if a null pointer was provided.
 *
 **/
FF_T_SINT8 FF_Close(FF_FILE *pFile) {

	if(!pFile) {
		return -1;
	}
	// If file written, flush to disk
	free(pFile);
	// Simply free the pointer!
	return 0;
}


