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
 *	@file		ff_dir.c
 *	@author		James Walmsley
 *	@ingroup	DIR
 *
 *	@defgroup	DIR Handles Directory Traversal
 *	@brief		Handles DIR access and traversal.
 *
 *	Provides FindFirst() and FindNext() Interfaces
 **/

#include "ff_dir.h"
#include <stdio.h>

static void FF_lockDIR(FF_IOMAN *pIoman) {
	FF_PendSemaphore(pIoman->pSemaphore);	// Use Semaphore to protect FAT modifications.
	{
		while(pIoman->DirLock) {
			FF_ReleaseSemaphore(pIoman->pSemaphore);
			FF_Yield();						// Keep Releasing and Yielding until we have the Fat protector.
			FF_PendSemaphore(pIoman->pSemaphore);
		}
		pIoman->DirLock = 1;
	}
	FF_ReleaseSemaphore(pIoman->pSemaphore);
}

static void FF_unlockDIR(FF_IOMAN *pIoman) {
	FF_PendSemaphore(pIoman->pSemaphore);
	{
		pIoman->DirLock = 0;
	}
	FF_ReleaseSemaphore(pIoman->pSemaphore);
}

static FF_T_UINT8 FF_CreateChkSum(const FF_T_INT8 *pa_pShortName) {
	FF_T_UINT8	cNameLen;
	FF_T_UINT8	ChkSum = 0;

	for(cNameLen = 11; cNameLen != 0; cNameLen--) {
		ChkSum = ((ChkSum & 1) ? 0x80 : 0) + (ChkSum >> 1) + *pa_pShortName++;
	}
	return ChkSum;
}

FF_T_SINT8 FF_FindNextInDir(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_DIRENT *pDirent) {
	
	FF_T_UINT8	numLFNs;
	FF_T_UINT8	EntryBuffer[32];

	if(!pIoman) {
		return FF_ERR_NULL_POINTER;
	}
	
	for(pDirent->CurrentItem; pDirent->CurrentItem < 0xFFFF; pDirent->CurrentItem += 1) {
		if(FF_FetchEntry(pIoman, DirCluster, pDirent->CurrentItem, EntryBuffer)) {
			return -2;
		}
		if(EntryBuffer[0] != 0xE5) {
			if(FF_isEndOfDir(EntryBuffer)){
				return -2;
			}
			pDirent->Attrib = FF_getChar(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB));
			if((pDirent->Attrib & FF_FAT_ATTR_LFN) == FF_FAT_ATTR_LFN) {
				// LFN Processing
				numLFNs = (FF_T_UINT8)(EntryBuffer[0] & ~0x40);
#ifdef FF_LFN_SUPPORT
				FF_PopulateLongDirent(pIoman, pDirent, DirCluster, pDirent->CurrentItem);
				return 0;
#else 
				pDirent->CurrentItem += (numLFNs - 1);
#endif
			} else if((pDirent->Attrib & FF_FAT_ATTR_VOLID) == FF_FAT_ATTR_VOLID) {
				// Do Nothing
			
			} else {
				FF_PopulateShortDirent(pDirent, EntryBuffer);
				pDirent->CurrentItem += 1;
				return 0;
			}
		}
	}
	
	return -1;
}

FF_T_UINT32 FF_FindEntryInDir(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_T_INT8 *name, FF_T_UINT8 pa_Attrib, FF_DIRENT *pDirent) {

	FF_T_UINT16		fnameLen;
	FF_T_UINT16		compareLength;
	FF_T_UINT16		nameLen;
	FF_T_INT8		Filename[FF_MAX_FILENAME];
	
	pDirent->CurrentItem = 0;
	nameLen = (FF_T_UINT16) strlen(name);

	while(1) {	
		if(FF_FindNextInDir(pIoman, DirCluster, pDirent)) {
			return 0;	// end of dir, file not found!
		}

		if((pDirent->Attrib & pa_Attrib) == pa_Attrib){
			strcpy(Filename, pDirent->FileName);
			fnameLen = (FF_T_UINT16) strlen(Filename);
			FF_tolower(Filename, (FF_T_UINT32) fnameLen);
			FF_tolower(name, (FF_T_UINT32) nameLen);
			if(nameLen > fnameLen) {
				compareLength = nameLen;
			} else {
				compareLength = fnameLen;
			}
			if(strncmp(name, Filename, (FF_T_UINT32) compareLength) == 0) {
				// Object found!
				return pDirent->ObjectCluster;	// Return the cluster number
			}
		}
	}
	return 0;
}

/**
 *	@private
 **/
/*FF_T_UINT32 FF_FindEntry(FF_IOMAN *pIoman, FF_T_SINT8 *path, FF_T_UINT8 pa_Attrib, FF_DIRENT *pDirent) {
	
	FF_T_INT32		retVal;
	FF_T_INT8		name[FF_MAX_FILENAME];
	FF_T_INT8		Filename[FF_MAX_FILENAME];
	FF_T_UINT16		fnameLen;
	FF_T_UINT16		compareLength;
	FF_T_UINT16		nameLen;
	FF_T_UINT16		i = strlen(path);


	while(i != 0) {
		if(path[i] == '\\' || path[i] == '/') {
			break;
		}
		i--;
	}
	
	if(i == 0) {
		i = 1;
	}

	nameLen = strlen((path + i));
	strncpy(name, (path + i), nameLen);
	name[nameLen] = '\0';	
	

	if(FF_FindFirst(pIoman, pDirent, path)) {
		return 0; // file not found.
	}

	if((pDirent->Attrib & pa_Attrib) == pa_Attrib){
		strcpy(Filename, pDirent->FileName);
		fnameLen = (FF_T_UINT16) strlen(Filename);
		FF_tolower(Filename, (FF_T_UINT32) fnameLen);
		FF_tolower(name, (FF_T_UINT32) nameLen);
		if(nameLen > fnameLen) {
			compareLength = nameLen;
		} else {
			compareLength = fnameLen;
		}
		if(strncmp(name, Filename, (FF_T_UINT32) compareLength) == 0) {
			// Object found!!
			return pDirent->ObjectCluster;	// Return the cluster number
		}
	}

	while(1) {	
		if(FF_FindNext(pIoman, pDirent)) {
			return 0;	// end of dir, file not found!
		}

		if((pDirent->Attrib & pa_Attrib) == pa_Attrib){
			strcpy(Filename, pDirent->FileName);
			fnameLen = (FF_T_UINT16) strlen(Filename);
			FF_tolower(Filename, (FF_T_UINT32) fnameLen);
			FF_tolower(name, (FF_T_UINT32) nameLen);
			if(nameLen > fnameLen) {
				compareLength = nameLen;
			} else {
				compareLength = fnameLen;
			}
			if(strncmp(name, Filename, (FF_T_UINT32) compareLength) == 0) {
				// Object found!
				return pDirent->ObjectCluster;	// Return the cluster number
			}
		}
	}
	return 0;
}*/

/**
 *	@private
 **/
FF_T_UINT32 FF_FindDir(FF_IOMAN *pIoman, FF_T_INT8 *path, FF_T_UINT16 pathLen) {
    FF_T_UINT32		dirCluster = pIoman->pPartition->RootDirCluster;
    FF_T_INT8       mytoken[FF_MAX_FILENAME];
    FF_T_INT8       *token;
    FF_T_UINT16     it = 0;         // Re-entrancy Variables for FF_strtok()
    FF_T_BOOL       last = FF_FALSE;
    FF_DIRENT       MyDir;

    if(pathLen == 1) {      // Must be the root dir! (/ or \)
            return pIoman->pPartition->RootDirCluster;
    }
    
    if(path[pathLen-1] == '\\' || path[pathLen-1] == '/') {
            pathLen--;      
    }

    token = FF_strtok(path, mytoken, &it, &last, pathLen);

     do{
            //lastDirCluster = dirCluster;
            MyDir.CurrentItem = 0;
            dirCluster = FF_FindEntryInDir(pIoman, dirCluster, token, FF_FAT_ATTR_DIR, &MyDir);
			if(dirCluster == 0 && MyDir.CurrentItem == 2 && MyDir.FileName[0] == '.') { // .. Dir Entry pointing to root dir.
				dirCluster = pIoman->pPartition->RootDirCluster;
            }
            token = FF_strtok(path, mytoken, &it, &last, pathLen);
    }while(token != NULL);

    return dirCluster;
}

#ifdef FF_LFN_SUPPORT
/**
 *	@private
 **//*
FF_T_SINT8 FF_getLFN(FF_IOMAN *pIoman, FF_BUFFER *pBuffer, FF_DIRENT *pDirent, FF_T_INT8 *filename) {

	FF_T_UINT8	 	numLFNs;
	FF_T_UINT16		lenlfn = 0;
	FF_T_UINT8		tester;
	FF_T_UINT16		i,y;
	FF_T_UINT32		CurrentCluster;
	FF_T_UINT32		fatEntry;
	FF_T_UINT8		*buffer		= pBuffer->pBuffer;
	FF_T_UINT32		Sector		= pBuffer->Sector;
	FF_T_UINT32		Entry		= FF_getMinorBlockEntry(pIoman, pDirent->CurrentItem, 32);

	tester = FF_getChar(pBuffer->pBuffer, (FF_T_UINT16)(Entry * 32));
	numLFNs = (FF_T_UINT8) (tester & ~0x40);

	while(numLFNs > 0) {
		if(FF_getClusterChainNumber(pIoman, pDirent->CurrentItem, 32) > pDirent->CurrentCluster) {
			FF_ReleaseBuffer(pIoman, pBuffer);
			fatEntry = FF_getFatEntry(pIoman, pDirent->DirCluster);
			if(fatEntry == (FF_T_UINT32) FF_ERR_DEVICE_DRIVER_FAILED) {
				return FF_ERR_DEVICE_DRIVER_FAILED;
			}

			if(FF_isEndOfChain(pIoman, fatEntry)) {
				CurrentCluster = pDirent->DirCluster;
				// ERROR THIS SHOULD NOT OCCUR!
			} else {
				CurrentCluster = fatEntry;
			}

			pBuffer = FF_GetBuffer(pIoman, FF_getRealLBA(pIoman, FF_Cluster2LBA(pIoman, CurrentCluster)), FF_MODE_READ);
			if(!pBuffer) {
				return FF_ERR_DEVICE_DRIVER_FAILED;
			}
			Entry = 0;	
		}

		if(Entry > 15) {
			FF_ReleaseBuffer(pIoman, pBuffer);
			Sector += 1;
			pBuffer = FF_GetBuffer(pIoman, Sector, FF_MODE_READ);
			if(!pBuffer) {
				return FF_ERR_DEVICE_DRIVER_FAILED;
			}
			buffer = pBuffer->pBuffer;
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
}*/
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

	if(shortName[8] != 0x20){
		name[i] = '.';
		name[i+1] = shortName[8];
		name[i+2] = shortName[9];
		name[i+3] = shortName[10];
		name[i+4] = '\0';
		for(i = 0; i < 11; i++) {
			if(name[i] == 0x20) {
				name[i] = '\0';
				break;
			}
		}
	} else {
		name[i] = '\0';
	}

}

void FF_PopulateShortDirent(FF_DIRENT *pDirent, FF_T_UINT8 *EntryBuffer) {
	FF_T_UINT16 myShort;
	
	memcpy(pDirent->FileName, EntryBuffer, 11);	// Copy the filename into the Dirent object.
	FF_ProcessShortName(pDirent->FileName);		// Format the shortname, for pleasant viewing.
	FF_tolower(pDirent->FileName, strlen(pDirent->FileName));
	// Get the item's Cluster address.
	myShort = FF_getShort(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_HIGH));
	pDirent->ObjectCluster = (FF_T_UINT32) (myShort << 16);
	myShort = FF_getShort(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_LOW));
	pDirent->ObjectCluster |= myShort;

	// Get the filesize.
	pDirent->Filesize = FF_getLong(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_FILESIZE));
	// Get the attribute.
	pDirent->Attrib = FF_getChar(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB));
}

FF_T_SINT8 FF_FetchEntry(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_T_UINT16 nEntry, FF_T_UINT8 *buffer) {
	FF_BUFFER *pBuffer;
	FF_T_UINT32 itemLBA;
	FF_T_UINT32 chainLength		= FF_GetChainLength(pIoman, DirCluster);	// BottleNeck
	FF_T_UINT32 clusterNum		= FF_getClusterChainNumber	(pIoman, nEntry, (FF_T_UINT16)32);
	FF_T_UINT32 relItem			= FF_getMinorBlockEntry		(pIoman, nEntry, (FF_T_UINT16)32);
	FF_T_UINT32 clusterAddress	= FF_TraverseFAT(pIoman, DirCluster, clusterNum);	// BottleNeck
	
	if((clusterNum + 1) > chainLength) {
		return -3;	// End of Dir was reached!
	}

	itemLBA = FF_Cluster2LBA(pIoman, clusterAddress)	+ FF_getMajorBlockNumber(pIoman, nEntry, (FF_T_UINT16)32);
	itemLBA = FF_getRealLBA	(pIoman, itemLBA)			+ FF_getMinorBlockNumber(pIoman, relItem, (FF_T_UINT16)32);
	
	pBuffer = FF_GetBuffer(pIoman, itemLBA, FF_MODE_READ);
	{
		memcpy(buffer, (pBuffer->pBuffer + (relItem*32)), 32);
	}
	FF_ReleaseBuffer(pIoman, pBuffer);
 
    return 0;
}


FF_T_SINT8 FF_PushEntry(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_T_UINT16 nEntry, FF_T_UINT8 *buffer) {
	FF_BUFFER *pBuffer;
	FF_T_UINT32 itemLBA;
	FF_T_UINT32 chainLength		= FF_GetChainLength(pIoman, DirCluster);	// BottleNeck
	FF_T_UINT32 clusterNum		= FF_getClusterChainNumber	(pIoman, nEntry, (FF_T_UINT16)32);
	FF_T_UINT32 relItem			= FF_getMinorBlockEntry		(pIoman, nEntry, (FF_T_UINT16)32);
	FF_T_UINT32 clusterAddress	= FF_TraverseFAT(pIoman, DirCluster, clusterNum);	// BottleNeck
	
	if((clusterNum + 1) > chainLength) {
		return -3;	// End of Dir was reached!
	}

	itemLBA = FF_Cluster2LBA(pIoman, clusterAddress)	+ FF_getMajorBlockNumber(pIoman, nEntry, (FF_T_UINT16)32);
	itemLBA = FF_getRealLBA	(pIoman, itemLBA)			+ FF_getMinorBlockNumber(pIoman, relItem, (FF_T_UINT16)32);
	
	pBuffer = FF_GetBuffer(pIoman, itemLBA, FF_MODE_WRITE);
	{
		memcpy((pBuffer->pBuffer + (relItem*32)), buffer, 32);
	}
	FF_ReleaseBuffer(pIoman, pBuffer);
 
    return 0;
}


/**
 *	@private
 **/
FF_T_SINT8 FF_GetEntry(FF_IOMAN *pIoman, FF_T_UINT16 nEntry, FF_T_UINT32 DirCluster, FF_DIRENT *pDirent) {
	FF_T_UINT8 EntryBuffer[32];
	FF_T_UINT8 numLFNs;
	
	if(FF_FetchEntry(pIoman, DirCluster, nEntry, EntryBuffer)) {
			return -2;
	}
	if(EntryBuffer[0] != 0xE5) {
		if(FF_isEndOfDir(EntryBuffer)){
			return -2;
		}
		
		pDirent->Attrib = FF_getChar(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB));
		
		if((pDirent->Attrib & FF_FAT_ATTR_LFN) == FF_FAT_ATTR_LFN) {
			// LFN Processing
			numLFNs = (FF_T_UINT8)(EntryBuffer[0] & ~0x40);
	#ifdef FF_LFN_SUPPORT
			FF_PopulateLongDirent(pIoman, pDirent, DirCluster, nEntry);
			return 0;
	#else 
			pDirent->CurrentItem += (numLFNs - 1);
	#endif
		} else if((pDirent->Attrib & FF_FAT_ATTR_VOLID) == FF_FAT_ATTR_VOLID) {
			// Do Nothing
		
		} else {
			FF_PopulateShortDirent(pDirent, EntryBuffer);
			pDirent->CurrentItem += 1;
			return 0;
		}
	}
	return 0;
}

FF_T_BOOL FF_isEndOfDir(FF_T_UINT8 *EntryBuffer) {
	if(EntryBuffer[0] == 0x00) {
		return FF_TRUE;
	}
	return FF_FALSE;
}


FF_T_SINT8 FF_PopulateLongDirent(FF_IOMAN *pIoman, FF_DIRENT *pDirent, FF_T_UINT32 DirCluster, FF_T_UINT16 nEntry) {
	FF_T_UINT8	EntryBuffer[32];
	FF_T_INT8	ShortName[13];
	FF_T_UINT8 numLFNs;
	FF_T_UINT8 x;
	FF_T_UINT8 CheckSum = 0;
	FF_T_UINT16 i,y;
	FF_T_UINT16 lenlfn = 0;
	FF_T_UINT16 myShort;
	
	FF_FetchEntry(pIoman, DirCluster, nEntry++, EntryBuffer);
	numLFNs = (FF_T_UINT8)(EntryBuffer[0] & ~0x40);
	// Handle the name
	CheckSum = FF_getChar(EntryBuffer, FF_FAT_LFN_CHECKSUM);

	x = numLFNs;
	while(numLFNs) {
		for(i = 0, y = 0; i < 5; i++, y += 2) {
			pDirent->FileName[i + ((numLFNs - 1) * 13)] = EntryBuffer[FF_FAT_LFN_NAME_1 + y];
			lenlfn++;
		}

		for(i = 0, y = 0; i < 6; i++, y += 2) {
			pDirent->FileName[i + ((numLFNs - 1) * 13) + 5] = EntryBuffer[FF_FAT_LFN_NAME_2 + y];
			lenlfn++;
		}

		for(i = 0, y = 0; i < 2; i++, y += 2) {
			pDirent->FileName[i + ((numLFNs - 1) * 13) + 11] = EntryBuffer[FF_FAT_LFN_NAME_3 + y];
			lenlfn++;
		}

		FF_FetchEntry(pIoman, DirCluster, nEntry++, EntryBuffer);
		numLFNs--;
	}

	pDirent->FileName[lenlfn] = '\0';
	
	// Process the ShortName Entry
	memcpy(ShortName, EntryBuffer, 11);
	if(CheckSum != FF_CreateChkSum(ShortName)) {
		FF_ProcessShortName(ShortName);
		strcpy(pDirent->FileName, ShortName);
	} else {
		FF_ProcessShortName(ShortName);
	}
	
	myShort = FF_getShort(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_HIGH));
	pDirent->ObjectCluster = (FF_T_UINT32) (myShort << 16);
	myShort = FF_getShort(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_LOW));
	pDirent->ObjectCluster |= myShort;

	// Get the filesize.
	pDirent->Filesize = FF_getLong(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_FILESIZE));
	// Get the attribute.
	pDirent->Attrib = FF_getChar(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB));
	
	pDirent->CurrentItem = nEntry;
	return x;
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
 *	@return	0 on success
 *	@return	FF_ERR_DEVICE_DRIVER_FAILED if device access failed.
 *	@return -2 if Dir was not found.
 *
 **/
FF_T_SINT8 FF_FindFirst(FF_IOMAN *pIoman, FF_DIRENT *pDirent, FF_T_INT8 *path) {
	FF_T_UINT8	numLFNs;
	FF_T_UINT8	EntryBuffer[32];
	FF_T_UINT16	PathLen = (FF_T_UINT16) strlen(path);

	if(!pIoman) {
		return FF_ERR_NULL_POINTER;
	}

	pDirent->DirCluster = FF_FindDir(pIoman, path, PathLen);	// Get the directory cluster, if it exists.

	if(pDirent->DirCluster == 0) {
		return -2;
	}
	
	for(pDirent->CurrentItem = 0; pDirent->CurrentItem < 0xFFFF; pDirent->CurrentItem += 1) {
		if(FF_FetchEntry(pIoman, pDirent->DirCluster, pDirent->CurrentItem, EntryBuffer)) {
			return -2;
		}
		if(EntryBuffer[0] != 0xE5) {
			if(FF_isEndOfDir(EntryBuffer)){
				return -2;
			}
			pDirent->Attrib = FF_getChar(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB));
			if((pDirent->Attrib & FF_FAT_ATTR_LFN) == FF_FAT_ATTR_LFN) {
				// LFN Processing
				numLFNs = (FF_T_UINT8)(EntryBuffer[0] & ~0x40);
#ifdef FF_LFN_SUPPORT
				FF_PopulateLongDirent(pIoman, pDirent, pDirent->DirCluster, pDirent->CurrentItem);
				return 0;
#else 
				pDirent->CurrentItem += (numLFNs - 1);
#endif
			} else if((pDirent->Attrib & FF_FAT_ATTR_VOLID) == FF_FAT_ATTR_VOLID) {
				// Do Nothing
			
			} else {
				FF_PopulateShortDirent(pDirent, EntryBuffer);
				pDirent->CurrentItem += 1;
				return 0;
			}
		}
	}

	return -2;
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
 *	@return FF_ERR_DEVICE_DRIVER_FAILED is device access failed.
 *
 **/
FF_T_SINT8 FF_FindNext(FF_IOMAN *pIoman, FF_DIRENT *pDirent) {
	
	FF_T_UINT8	numLFNs;
	FF_T_UINT8	EntryBuffer[32];

	if(!pIoman) {
		return FF_ERR_NULL_POINTER;
	}
	
	for(pDirent->CurrentItem; pDirent->CurrentItem < 0xFFFF; pDirent->CurrentItem += 1) {
		if(FF_FetchEntry(pIoman, pDirent->DirCluster, pDirent->CurrentItem, EntryBuffer)) {
			return -2;
		}
		if(EntryBuffer[0] != FF_FAT_DELETED) {
			if(FF_isEndOfDir(EntryBuffer)){
				return -2;
			}
			pDirent->Attrib = FF_getChar(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB));
			if((pDirent->Attrib & FF_FAT_ATTR_LFN) == FF_FAT_ATTR_LFN) {
				// LFN Processing
				numLFNs = (FF_T_UINT8)(EntryBuffer[0] & ~0x40);
				// Fetch the shortname, and get it's checksum, or for a deleted item with
				// orphaned LFN entries.

				FF_FetchEntry(pIoman, pDirent->DirCluster, (pDirent->CurrentItem + numLFNs), EntryBuffer);

				
				
				// Get the shortname and check if it is marked deleted.
#ifdef FF_LFN_SUPPORT
				if(EntryBuffer[0] != FF_FAT_DELETED) {
					FF_PopulateLongDirent(pIoman, pDirent, pDirent->DirCluster, pDirent->CurrentItem);
					return 0;
				}
#else 
				pDirent->CurrentItem += (numLFNs - 1);
#endif
			} else if((pDirent->Attrib & FF_FAT_ATTR_VOLID) == FF_FAT_ATTR_VOLID) {
				// Do Nothing
			
			} else {
				FF_PopulateShortDirent(pDirent, EntryBuffer);
				pDirent->CurrentItem += 1;
				return 0;
			}
		}
	}
	
	return -1;
}


FF_T_SINT8 FF_RewindFind(FF_IOMAN *pIoman, FF_DIRENT *pDirent) {
	pDirent->CurrentItem = 0;
	return 0;
}


FF_T_UINT32 FF_FindFreeDirent(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_T_UINT16 Sequential) {

	FF_T_UINT8	EntryBuffer[32];
	//FF_T_UINT8	i;
	FF_T_UINT16 nEntry = 0;
	
	FF_FetchEntry(pIoman, DirCluster, nEntry++, EntryBuffer);
	if(FF_isEndOfDir(EntryBuffer)) {
		return (nEntry-1);
	}
	while(EntryBuffer[0] != 0xE5) {
		FF_FetchEntry(pIoman, DirCluster, nEntry, EntryBuffer);
		if(FF_isEndOfDir(EntryBuffer)) {
			return (nEntry);
		}
		nEntry++;
	}

	return (nEntry-1);	// Return the beginning entry in the sequential sequence.
}




FF_T_SINT8 FF_PutEntry(FF_IOMAN *pIoman, FF_T_UINT16 Entry, FF_T_UINT32 DirCluster, FF_DIRENT *pDirent) {
	FF_BUFFER *pBuffer;
	FF_T_UINT32 itemLBA;
	FF_T_UINT32 clusterNum		= FF_getClusterChainNumber	(pIoman, Entry, (FF_T_UINT16)32);
	FF_T_UINT32 relItem			= FF_getMinorBlockEntry		(pIoman, Entry, (FF_T_UINT16)32);
	FF_T_UINT32 clusterAddress	= FF_TraverseFAT(pIoman, DirCluster, clusterNum);

	itemLBA = FF_Cluster2LBA(pIoman, clusterAddress)	+ FF_getMajorBlockNumber(pIoman, Entry, (FF_T_UINT16)32);
	itemLBA = FF_getRealLBA	(pIoman, itemLBA)			+ FF_getMinorBlockNumber(pIoman, relItem, (FF_T_UINT16)32);
	
	pBuffer = FF_GetBuffer(pIoman, itemLBA, FF_MODE_WRITE);
	{
		// Modify the Entry!
		//memcpy((pBuffer->pBuffer + (32*relItem)), pDirent->FileName, 11);
		FF_putChar(pBuffer->pBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB + (32 * relItem)), pDirent->Attrib);
		FF_putShort(pBuffer->pBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_HIGH + (32 * relItem)), (FF_T_UINT16)(pDirent->ObjectCluster >> 16));
		FF_putShort(pBuffer->pBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_LOW  + (32 * relItem)), (FF_T_UINT16)(pDirent->ObjectCluster));
		FF_putLong(pBuffer->pBuffer,  (FF_T_UINT16)(FF_FAT_DIRENT_FILESIZE  + (32 * relItem)), pDirent->Filesize);
	}
	FF_ReleaseBuffer(pIoman, pBuffer);
 
    return 0;
}


static FF_T_BOOL FF_isShortName(const FF_T_UINT8 *Name, FF_T_UINT16 StrLen) {
	FF_T_UINT16 i;
	for(i = 0; i < StrLen; i++) {
		if(Name[i] == '.') {
			i--;
		}
	}
	if(i < 11) {
		return FF_TRUE;
	}
	return FF_FALSE;
}

FF_T_SINT8 FF_CreateShortName(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_T_INT8 *ShortName, FF_T_INT8 *LongName) {
	FF_T_UINT16 i,x,y;
	FF_T_INT8	TempName[FF_MAX_FILENAME];
	FF_T_INT8	MyShortName[13];
	FF_T_UINT16 NameLen; 
	FF_T_BOOL	FitsShort = FF_FALSE;
	FF_DIRENT	MyDir;
	//FF_T_SINT8	RetVal = 0;
	FF_T_INT8	NumberBuf[6];
	// Create a Short Name
	strncpy(TempName, LongName, FF_MAX_FILENAME);
	NameLen = (FF_T_UINT16) strlen(TempName);
	FF_toupper(TempName, NameLen);

	// Initialise Shortname

	for(i = 0; i < 11; i++) {
		ShortName[i] = 0x20;
	}

	// Does LongName fit a shortname?

	for(i = 0, x = 0; i < NameLen; i++) {
		if(TempName[i] != '.') {
			x++;
		}
	}

	if(x <= 11) {
		FitsShort = FF_TRUE;
	}

	// Main part of the name
	for(i = 0, x = 0; i < 8; i++, x++) {
		if(i == 0 && TempName[x] == '.') {
			i--;
		} else {
			if(TempName[x] == '.') {
				break;
			} else if(TempName[x] == ' ') {
				i--;
			} else {
				ShortName[i] = TempName[x];
				if(ShortName[i] == 0x00) {
					ShortName[i] = 0x20;
				}
			}
		}
	}
	
	for(i = NameLen; i > x; i--) {
		if(TempName[i] == '.') {
			break;
		}
	}

	if(TempName[i] == '.') {
		x = i + 1;
		for(i = 0; i < 3; i++) {
			if(x < NameLen) {
				ShortName[8 + i] = TempName[x++];
			}
		}
	}

	// Tail :
	memcpy(MyShortName, ShortName, 11);
	FF_ProcessShortName(MyShortName);
	
	if(!FF_FindEntryInDir(pIoman, DirCluster, MyShortName, 0x00, &MyDir) && FitsShort) {
		return 0;
	} else {
		if(FitsShort) {
			return FF_ERR_DIR_OBJECT_EXISTS;
		}
		for(i = 1; i < 0x0000FFFF; i++) { // Max Number of Entries in a DIR!
			sprintf(NumberBuf, "%d", i);
			NameLen = (FF_T_UINT16) strlen(NumberBuf);
			x = 7 - NameLen;
			ShortName[x++] = '~';
			for(y = 0; y < NameLen; y++) {
				ShortName[x+y] = NumberBuf[y];
			}
			memcpy(MyShortName, ShortName, 11);
			FF_ProcessShortName(MyShortName);
			if(!FF_FindEntryInDir(pIoman, DirCluster, MyShortName, 0x00, &MyDir)) {
				return 0;
			}
		}
		// Add a tail and special number until we're happy :D
	}

	return FF_ERR_DIR_DIRECTORY_FULL;
}

FF_T_SINT8 FF_CreateDirent(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_DIRENT *pDirent) {
	
	FF_T_UINT8	EntryBuffer[32];	// 
//	FF_T_UINT16	NameLen = (FF_T_UINT16) strlen(pDirent->FileName);
//	FF_T_UINT8	numLFNs = (FF_T_UINT8) (NameLen / 32);
	FF_T_UINT16	FreeEntry;
	FF_T_SINT8	RetVal = 0;
	
	memset(EntryBuffer, 0, 32);
	// Create the ShortName
	RetVal = FF_CreateShortName(pIoman, DirCluster, (FF_T_INT8 *) EntryBuffer, pDirent->FileName);

	if(RetVal) {
		return RetVal;
	}

#ifdef FF_LFN_SUPPORT
	// Create and push the LFN's
	FreeEntry = (FF_T_UINT16) FF_FindFreeDirent(pIoman, DirCluster, 1);
#else
	FreeEntry = FF_FindFreeDirent(pIoman, DirCluster, 1);
#endif

	//if(FreeEntry) {
		FF_putChar(EntryBuffer,  (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB), pDirent->Attrib);
		FF_putShort(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_HIGH), (FF_T_UINT16)(pDirent->ObjectCluster >> 16));
		FF_putShort(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_LOW), (FF_T_UINT16)(pDirent->ObjectCluster));
		FF_putLong(EntryBuffer,  (FF_T_UINT16)(FF_FAT_DIRENT_FILESIZE), pDirent->Filesize);
		
		FF_PushEntry(pIoman, DirCluster, FreeEntry, EntryBuffer);
		
		if(pDirent) {
			pDirent->CurrentItem = FreeEntry;
		}
		
		return 0;
	//}

	//return 0;
}

FF_T_UINT32 FF_CreateFile(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_T_INT8 *FileName, FF_DIRENT *pDirent) {
	FF_DIRENT MyFile;
	strncpy(MyFile.FileName, FileName, FF_MAX_FILENAME);

	MyFile.Attrib = 0x00;
	MyFile.Filesize = 0;
	MyFile.ObjectCluster = FF_CreateClusterChain(pIoman);
	MyFile.CurrentItem = 0;

	if(FF_CreateDirent(pIoman, DirCluster, &MyFile)) {
		FF_UnlinkClusterChain(pIoman, MyFile.ObjectCluster, 0);
		FF_FlushCache(pIoman);
		return 0;
	}

	FF_FlushCache(pIoman);

	if(pDirent) {
		memcpy(pDirent, &MyFile, sizeof(FF_DIRENT));
	}

	return MyFile.ObjectCluster;
}

FF_T_SINT8 FF_MkDir(FF_IOMAN *pIoman, FF_T_INT8 *Path, FF_T_INT8 *DirName) {
	FF_DIRENT	MyDir;
	FF_T_UINT32 DirCluster;
	FF_T_UINT8	EntryBuffer[32];
	FF_T_UINT32 DotDotCluster;
	FF_T_UINT8	i;
	FF_T_SINT8	RetVal = 0;

	if(!pIoman) {
		return FF_ERR_NULL_POINTER;
	}

	DirCluster = FF_FindDir(pIoman, Path, (FF_T_UINT16) strlen(Path));

	if(DirCluster) {
		strncpy(MyDir.FileName, DirName, FF_MAX_FILENAME);
		MyDir.Filesize		= 0;
		MyDir.Attrib		= FF_FAT_ATTR_DIR;
		MyDir.ObjectCluster	= FF_CreateClusterChain(pIoman);
		FF_ClearCluster(pIoman, MyDir.ObjectCluster);

		RetVal = FF_CreateDirent(pIoman, DirCluster, &MyDir);

		if(RetVal) {
			FF_UnlinkClusterChain(pIoman, MyDir.ObjectCluster, 0);
			FF_FlushCache(pIoman);
			return RetVal;
		}
		
		memset(EntryBuffer, 0, 32);
		EntryBuffer[0] = '.';
		for(i = 1; i < 11; i++) {
			EntryBuffer[i] = 0x20;
		}
		FF_putChar(EntryBuffer,  (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB), FF_FAT_ATTR_DIR);
		FF_putShort(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_HIGH), (FF_T_UINT16)(MyDir.ObjectCluster >> 16));
		FF_putShort(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_LOW), (FF_T_UINT16) MyDir.ObjectCluster);
		FF_putLong(EntryBuffer,  (FF_T_UINT16)(FF_FAT_DIRENT_FILESIZE), 0);

		FF_PushEntry(pIoman, MyDir.ObjectCluster, 0, EntryBuffer);

		memset(EntryBuffer, 0, 32);
		EntryBuffer[0] = '.';
		EntryBuffer[1] = '.';
		for(i = 2; i < 11; i++) {
			EntryBuffer[i] = 0x20;
		}
		
		if(DirCluster == pIoman->pPartition->RootDirCluster) {
			DotDotCluster = 0;
		} else {
			DotDotCluster = DirCluster;
		}

		FF_putChar(EntryBuffer,  (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB), FF_FAT_ATTR_DIR);
		FF_putShort(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_HIGH), (FF_T_UINT16)(DotDotCluster >> 16));
		FF_putShort(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_LOW), (FF_T_UINT16) DotDotCluster);
		FF_putLong(EntryBuffer,  (FF_T_UINT16)(FF_FAT_DIRENT_FILESIZE), 0);
		
		FF_PushEntry(pIoman, MyDir.ObjectCluster, 1, EntryBuffer);
		
		FF_FlushCache(pIoman);	// Ensure dir was flushed to the disk!

		return 0;
	}
	
	return -3;
}

