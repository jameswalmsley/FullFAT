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
		nEntries = (512 / 4);
	} else {
		nEntries = (512 / 2);
	}

	nSector  = pIoman->pPartition->FatBeginLBA;
	nSector += nCluster / nEntries; // + 2
	return nSector;
}

// Release all buffers before calling this!
FF_T_UINT32 FF_getFatEntry(FF_IOMAN *pIoman, FF_T_UINT32 nCluster) {
	FF_T_UINT32 fatEntry = 0;
	FF_BUFFER *pBuffer = 0;
	
	pBuffer = FF_GetBuffer(pIoman, FF_getFATSector(pIoman, nCluster), FF_MODE_READ);

	if(pIoman->pPartition->Type == FF_T_FAT32) {
		fatEntry = FF_getLong(pBuffer->pBuffer, (nCluster % (512 / 4)));
	} else {
		fatEntry = (FF_T_UINT32) FF_getShort(pBuffer->pBuffer, (nCluster % (512 / 2)));
	}

	FF_ReleaseBuffer(pIoman, pBuffer);

	return fatEntry;
}

FF_T_BOOL FF_isEndOfChain(FF_IOMAN *pIoman, FF_T_UINT32 fatEntry) {
	FF_T_BOOL result = FF_FALSE;
	
	if(pIoman->pPartition->Type == FF_T_FAT32) {
		if(fatEntry == 0xffffffff)
			result = FF_TRUE;
	} else {
		if(fatEntry == 0x0000ffff)
			result = FF_TRUE;
	}

	return result;
}

// Finds Entries within a Sector of a Directory
// Will Swap out the Buffer if it has to process an LFN across 2 sectors.
FF_T_UINT32 FF_FindEntryInSector(FF_IOMAN *pIoman, FF_BUFFER *pBuffer, FF_T_INT8 *name, FF_T_UINT8 pa_Attrib, FF_DIRENT *pa_pDirent) {
	FF_T_UINT16 numEntries = 512 / 32;
	FF_T_UINT16 myShort;
	FF_T_UINT32 itemCluster = 0;
	FF_T_UINT8 i;
	FF_T_UINT8 nameLength;
	char Attrib = 0;
	char shortName[13];

	nameLength = strlen(name);

	for(i = 0; i < numEntries; i++) {
		
		Attrib = FF_getChar(pBuffer->pBuffer, (FF_FAT_DIRENT_ATTRIB + (32 * i)));
		
		if((Attrib & pa_Attrib) == pa_Attrib) {
			memcpy(shortName, (pBuffer->pBuffer + (32 * i)), 11);
			if(strncmp(name, shortName, nameLength) == 0) {
				// Item Found! :D
				myShort = FF_getShort(pBuffer->pBuffer, (FF_FAT_DIRENT_CLUS_HIGH + (32 * i)));
				itemCluster = (FF_T_UINT32) (myShort << 16);
				myShort = FF_getShort(pBuffer->pBuffer, (FF_FAT_DIRENT_CLUS_LOW + (32 * i)));
				itemCluster |= myShort;

				if(pa_pDirent) {
					// Extract Dirent Information!
					pa_pDirent->Filesize	= FF_getLong(pBuffer->pBuffer, (FF_FAT_DIRENT_FILESIZE + (32 * i)));
					pa_pDirent->Attrib		= FF_getChar(pBuffer->pBuffer, (FF_FAT_DIRENT_ATTRIB + (32 * i)));
					memcpy(pa_pDirent->ShortNAME, shortName, 11);
					pa_pDirent->ShortNAME[11] = '\0';
				}

				return itemCluster;
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

	char filename[260];

	FF_T_UINT16 numEntries = 512 / 32; // Entries in the current sector
	FF_T_UINT16 dirSector = 0;	// Current sector of the  current Dir entry
	FF_T_UINT16 dirEntry = 0;
	FF_T_UINT32 currentCluster = DirCluster; // The cluster of the Dir we are working on.
	FF_T_UINT32 dirLBA, fatLBA, itemCluster;
	FF_T_UINT16 i,x;

	FF_BUFFER *pBuffer;
	FF_T_UINT32 fatEntry = 0;
	FF_T_BOOL isEndOfDir = FF_FALSE;	// Flag true if the End of the Directory was reached!



	
	//pBuffer = FF_GetBuffer(pIoman, dirLBA, FF_MODE_READ);

	while(!FF_isEndOfChain(pIoman, fatEntry)) {	// Not at End of fat chain!

		fatEntry = FF_getFatEntry(pIoman, currentCluster);
		
		for(i = 0; i < pPart->SectorsPerCluster; i++) { // Process Each Sector
			// Process Each sector of the Dirent!
			dirLBA = FF_Cluster2LBA(pIoman, currentCluster);
			pBuffer = FF_GetBuffer(pIoman, dirLBA + i, FF_MODE_READ);

			itemCluster = FF_FindEntryInSector(pIoman, pBuffer, name, pa_Attrib, pa_pDirent);

			FF_ReleaseBuffer(pIoman, pBuffer);

			if(itemCluster) {
				return itemCluster;
			}

			if(isEndOfDir == FF_TRUE) { // End of the Directory was found. Finish!
				break;
			}
		}
		
		if(isEndOfDir == FF_TRUE) { // End of the Directory was found. Finish!
				break;
		}
		currentCluster++;
	}

	return 0;

}

// returns the Cluster of the specified directory
FF_T_UINT32 FF_FindDir(FF_IOMAN *pIoman, FF_T_INT8 *path) {
	FF_T_UINT32 dirLBA = 0;
	FF_T_UINT32 dirCluster = pIoman->pPartition->RootDirCluster;
	FF_BUFFER *pBuffer;

	FF_DIRENT mydir;

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

/**
 *	@brief	Find's the first directory entry for the provided path.
 *
 **/
FF_T_SINT8 FF_FindFirst(FF_IOMAN *pIoman, FF_DIRENT *pDirent, FF_T_INT8 *path) {

	FF_T_UINT32	dirCluster, currentCluster;

	if(!pIoman) {
		return FF_ERR_FAT_NULL_POINTER;
	}

	dirCluster = FF_FindDir(pIoman, path);
	
	FF_FindEntry(pIoman, dirCluster, "", 0x00, pDirent);

	return 0;
}

/*
FF_T_SINT8 FF_FindNext(FF_IOMAN *pIoman, FF_DIRENT *pDirent, FF_T_INT8 *path) {
	FF_BUFFER pBuffer;
	int i;
	FF_T_UINT32	dirCluster, currentCluster;

	if(!pIoman) {
		return FF_ERR_FAT_NULL_POINTER;
	}

	dirCluster = FF_FindDir(pIoman, path);
	
	while(!FF_isEndOfChain(pIoman, dirCluster)) {	// Not at End of fat chain!
		
		for(i = 0; i < pPart->SectorsPerCluster; i++) { // Process Each Sector
			// Process Each sector of the Dirent!
			dirLBA = FF_Cluster2LBA(pIoman, currentCluster);
			pBuffer = FF_GetBuffer(pIoman, dirLBA + i, FF_MODE_READ);

		}

		currentCluster++;
	}
}
*/