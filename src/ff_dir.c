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
#include "ff_string.h"
#include <stdio.h>

#ifdef FF_UNICODE_SUPPORT
#include <wchar.h>
#endif

#ifdef FF_UNICODE_SUPPORT
static void FF_ProcessShortName(FF_T_WCHAR *name);
#else
static void FF_ProcessShortName(FF_T_INT8 *name);
#endif

void FF_lockDIR(FF_IOMAN *pIoman) {
	FF_PendSemaphore(pIoman->pSemaphore);	// Use Semaphore to protect DIR modifications.
	{
		while((pIoman->Locks & FF_DIR_LOCK)) {
			FF_ReleaseSemaphore(pIoman->pSemaphore);
			FF_Yield();						// Keep Releasing and Yielding until we have the DIR protector.
			FF_PendSemaphore(pIoman->pSemaphore);
		}
		pIoman->Locks |= FF_DIR_LOCK;
	}
	FF_ReleaseSemaphore(pIoman->pSemaphore);
}

void FF_unlockDIR(FF_IOMAN *pIoman) {
	FF_PendSemaphore(pIoman->pSemaphore);
	{
		pIoman->Locks &= ~FF_DIR_LOCK;
	}
	FF_ReleaseSemaphore(pIoman->pSemaphore);
}

static FF_T_UINT8 FF_CreateChkSum(const FF_T_UINT8 *pa_pShortName) {
	FF_T_UINT8	cNameLen;
	FF_T_UINT8	ChkSum = 0;

	for(cNameLen = 11; cNameLen != 0; cNameLen--) {
		ChkSum = ((ChkSum & 1) ? 0x80 : 0) + (ChkSum >> 1) + *pa_pShortName++;
	}
	return ChkSum;
}


FF_ERROR FF_FindNextInDir(FF_IOMAN *pIoman, FF_DIRENT *pDirent, FF_FETCH_CONTEXT *pFetchContext) {
	
	FF_T_UINT8		numLFNs;
	FF_T_UINT8		EntryBuffer[32];
	FF_ERROR		Error;

	if(!pIoman) {
		return FF_ERR_NULL_POINTER;
	}

	for(; pDirent->CurrentItem < 0xFFFF; pDirent->CurrentItem += 1) {
		
		Error = FF_FetchEntryWithContext(pIoman, pDirent->CurrentItem, pFetchContext, EntryBuffer);
		
		if(Error) {
			return Error;
		}
		
		if(EntryBuffer[0] != 0xE5) {
			if(FF_isEndOfDir(EntryBuffer)){
				return FF_ERR_DIR_END_OF_DIR;
			}
			pDirent->Attrib = FF_getChar(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB));
			if((pDirent->Attrib & FF_FAT_ATTR_LFN) == FF_FAT_ATTR_LFN) {
				// LFN Processing
				numLFNs = (FF_T_UINT8)(EntryBuffer[0] & ~0x40);
				//pDirent->NumLFNs = numLFNs;
#ifdef FF_LFN_SUPPORT
				Error = FF_PopulateLongDirent(pIoman, pDirent, pDirent->CurrentItem, pFetchContext);
				if(Error) {
					return Error;
				}
				return FF_ERR_NONE;
#else 
				pDirent->CurrentItem += (numLFNs - 1);
#endif
			} else if((pDirent->Attrib & FF_FAT_ATTR_VOLID) == FF_FAT_ATTR_VOLID) {
				// Do Nothing
			
			} else {
				FF_PopulateShortDirent(pIoman, pDirent, EntryBuffer);
				pDirent->CurrentItem += 1;
				return FF_ERR_NONE;
			}
		}
	}
	
	return FF_ERR_DIR_END_OF_DIR;
}

#ifdef FF_UNICODE_SUPPORT
static FF_T_BOOL FF_ShortNameExists(FF_IOMAN *pIoman, FF_T_UINT32 ulDirCluster, FF_T_WCHAR *szShortName, FF_ERROR *pError) {
#else
static FF_T_BOOL FF_ShortNameExists(FF_IOMAN *pIoman, FF_T_UINT32 ulDirCluster, FF_T_INT8 *szShortName, FF_ERROR *pError) {
#endif

    FF_T_UINT16 		i;
    FF_T_UINT8      	EntryBuffer[32];
    FF_T_UINT8     		Attrib;
	FF_FETCH_CONTEXT	FetchContext;

#ifdef FF_UNICODE_SUPPORT
	FF_T_WCHAR			UTF16EntryBuffer[32];
#endif
	
#ifdef FF_HASH_CACHE
	FF_T_UINT32			ulHash;
#endif

	*pError = FF_ERR_NONE;


#ifdef FF_HASH_CACHE
	if(!FF_DirHashed(pIoman, ulDirCluster)) {
		// Hash the directory
		FF_HashDir(pIoman, ulDirCluster);
	}

#if FF_HASH_FUNCTION == CRC16
	ulHash = (FF_T_UINT32) FF_GetCRC16((FF_T_UINT8 *) szShortName, strlen(szShortName));
#elif FF_HASH_FUNCTION == CRC8
	ulHash = (FF_T_UINT32) FF_GetCRC8((FF_T_UINT8 *) szShortName, strlen(szShortName));
#endif
	
	if(!FF_CheckDirentHash(pIoman, ulDirCluster, ulHash)) {
		return FF_FALSE;
	}

#endif

	*pError = FF_InitEntryFetch(pIoman, ulDirCluster, &FetchContext);
	if(*pError) {
		return FF_FALSE;
	}

	for(i = 0; i < 0xFFFF; i++) {
		*pError = FF_FetchEntryWithContext(pIoman, i, &FetchContext, EntryBuffer);
		if(*pError) {
			FF_CleanupEntryFetch(pIoman, &FetchContext);
			return FF_FALSE;			
		}
		Attrib = FF_getChar(EntryBuffer, FF_FAT_DIRENT_ATTRIB);
		if(FF_getChar(EntryBuffer, 0x00) != 0xE5) {
			if(Attrib != FF_FAT_ATTR_LFN) {
#ifdef FF_UNICODE_SUPPORT
				// Convert Entry Buffer into UTF16
				FF_cstrntowcs(UTF16EntryBuffer, (FF_T_INT8 *) EntryBuffer, 32);
				FF_ProcessShortName(UTF16EntryBuffer);
#else
				FF_ProcessShortName((FF_T_INT8 *)EntryBuffer);
#endif
				if(FF_isEndOfDir(EntryBuffer)) {
					FF_CleanupEntryFetch(pIoman, &FetchContext);
					return FF_FALSE;
				}
#ifdef FF_UNICODE_SUPPORT
				if(wcscmp(szShortName, UTF16EntryBuffer) == 0) {
#else
				if(strcmp(szShortName, (FF_T_INT8 *)EntryBuffer) == 0) {
#endif
					FF_CleanupEntryFetch(pIoman, &FetchContext);
					return FF_TRUE;
				}
			}
		}
	}

	FF_CleanupEntryFetch(pIoman, &FetchContext);
    return FF_FALSE;
}

#ifdef FF_UNICODE_SUPPORT
FF_T_UINT32 FF_FindEntryInDir(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, const FF_T_WCHAR *name, FF_T_UINT8 pa_Attrib, FF_DIRENT *pDirent, FF_ERROR *pError) {
#else
FF_T_UINT32 FF_FindEntryInDir(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, const FF_T_INT8 *name, FF_T_UINT8 pa_Attrib, FF_DIRENT *pDirent, FF_ERROR *pError) {
#endif

	FF_T_UINT16		fnameLen;
	FF_T_UINT16		nameLen;

#ifdef FF_UNICODE_SUPPORT
	FF_T_WCHAR		Filename[FF_MAX_FILENAME];
	FF_T_WCHAR		MyFname[FF_MAX_FILENAME];
#else
	FF_T_INT8		Filename[FF_MAX_FILENAME];
	FF_T_INT8		MyFname[FF_MAX_FILENAME];
#endif

	FF_T_BOOL		bBreak = FF_FALSE;
	FF_FETCH_CONTEXT FetchContext;

	*pError = FF_ERR_NONE;
	
	pDirent->CurrentItem = 0;
#ifdef FF_UNICODE_SUPPORT
	nameLen = (FF_T_UINT16) wcslen(name);
#else
	nameLen = (FF_T_UINT16) strlen(name);
#endif

	*pError = FF_InitEntryFetch(pIoman, DirCluster, &FetchContext);
	if(*pError) {
		return 0;
	}

	while(!bBreak) {
		*pError = FF_FindNextInDir(pIoman, pDirent, &FetchContext);
		if(*pError) {
			FF_CleanupEntryFetch(pIoman, &FetchContext);
			return 0;	// end of dir, file not found!
		}

		if((pDirent->Attrib & pa_Attrib) == pa_Attrib){
#ifdef FF_UNICODE_SUPPORT
			wcscpy(Filename, pDirent->FileName);
			fnameLen = (FF_T_UINT16) wcslen(Filename);
#else
			strcpy(Filename, pDirent->FileName);
			fnameLen = (FF_T_UINT16) strlen(Filename);
#endif
			FF_tolower(Filename, (FF_T_UINT32) fnameLen);
			if(nameLen < FF_MAX_FILENAME) {
#ifdef FF_UNICODE_SUPPORT
				memcpy(MyFname, name, (nameLen + 1) * sizeof(FF_T_WCHAR));
#else
				memcpy(MyFname, name, nameLen + 1);
#endif
			} else {
#ifdef FF_UNICODE_SUPPORT
				memcpy(MyFname, name, FF_MAX_FILENAME * sizeof(FF_T_WCHAR));
#else
				memcpy(MyFname, name, FF_MAX_FILENAME);
#endif
				MyFname[FF_MAX_FILENAME - 1] = '\0';
			}
			FF_tolower(MyFname, (FF_T_UINT32) nameLen);
			
			if(nameLen == fnameLen) {
#ifdef FF_UNICODE_SUPPORT				
				if(wcsncmp(MyFname, Filename, (FF_T_UINT32) nameLen) == 0) {
#else
				if(strncmp(MyFname, Filename, (FF_T_UINT32) nameLen) == 0) {
#endif
					// Object found!
					FF_CleanupEntryFetch(pIoman, &FetchContext);
					return pDirent->ObjectCluster;	// Return the cluster number
				}
			}
		}
	}

	FF_CleanupEntryFetch(pIoman, &FetchContext);
	
	return 0;	// Cluster Number of 0 is invalid.	-- End of Dir
}



/**
 *	@private
 **/
#ifdef FF_UNICODE_SUPPORT
FF_T_UINT32 FF_FindDir(FF_IOMAN *pIoman, const FF_T_WCHAR *path, FF_T_UINT16 pathLen, FF_ERROR *pError) {
#else
FF_T_UINT32 FF_FindDir(FF_IOMAN *pIoman, const FF_T_INT8 *path, FF_T_UINT16 pathLen, FF_ERROR *pError) {
#endif
    FF_T_UINT32     dirCluster = pIoman->pPartition->RootDirCluster;
#ifdef FF_UNICODE_SUPPORT
	FF_T_WCHAR		mytoken[FF_MAX_FILENAME];
	FF_T_WCHAR		*token;
#else
	FF_T_INT8		mytoken[FF_MAX_FILENAME];
	FF_T_INT8       *token;
#endif
    
    FF_T_UINT16     it = 0;         // Re-entrancy Variables for FF_strtok()
    FF_T_BOOL       last = FF_FALSE;
    FF_DIRENT       MyDir;
#ifdef FF_PATH_CACHE
	FF_T_UINT32		i;
#endif

	*pError = FF_ERR_NONE;

    if(pathLen <= 1) {      // Must be the root dir! (/ or \)
		return pIoman->pPartition->RootDirCluster;
    }
    
    if(path[pathLen-1] == '\\' || path[pathLen-1] == '/') {
		pathLen--;      
    }
	
#ifdef FF_PATH_CACHE	// Is the requested path in the PATH CACHE?
	FF_PendSemaphore(pIoman->pSemaphore);	// Thread safety on shared object!
	{
		for(i = 0; i < FF_PATH_CACHE_DEPTH; i++) {
#ifdef FF_UNICODE_SUPPORT
			if(wcslen(pIoman->pPartition->PathCache[i].Path) == pathLen) {
				if(FF_strmatch(pIoman->pPartition->PathCache[i].Path, path, pathLen)) {
#else
			if(strlen(pIoman->pPartition->PathCache[i].Path) == pathLen) {
				if(FF_strmatch(pIoman->pPartition->PathCache[i].Path, path, pathLen)) {
#endif
				
					FF_ReleaseSemaphore(pIoman->pSemaphore);
					return pIoman->pPartition->PathCache[i].DirCluster;
				}
			}
		}
	}
	FF_ReleaseSemaphore(pIoman->pSemaphore);
#endif

    token = FF_strtok(path, mytoken, &it, &last, pathLen);

     do{
            MyDir.CurrentItem = 0;
            dirCluster = FF_FindEntryInDir(pIoman, dirCluster, token, FF_FAT_ATTR_DIR, &MyDir, pError);

			if(*pError) {
				return 0;
			}

			/*if(dirCluster == 0 && MyDir.CurrentItem == 2 && MyDir.FileName[0] == '.') { // .. Dir Entry pointing to root dir.
				dirCluster = pIoman->pPartition->RootDirCluster;
            }*/
            token = FF_strtok(path, mytoken, &it, &last, pathLen);
    }while(token != NULL);

#ifdef FF_PATH_CACHE	// Update the PATH CACHE with a new PATH
	 if(dirCluster) {	// Only cache if the dir was actually found!
		FF_PendSemaphore(pIoman->pSemaphore);
		{
			if(pathLen < FF_MAX_PATH) {	// Ensure the PATH won't cause a buffer overrun.
#ifdef FF_UNICODE_SUPPORT
				memcpy(pIoman->pPartition->PathCache[pIoman->pPartition->PCIndex].Path, path, pathLen * sizeof(FF_T_WCHAR));
#else
				memcpy(pIoman->pPartition->PathCache[pIoman->pPartition->PCIndex].Path, path, pathLen);
#endif
				pIoman->pPartition->PathCache[pIoman->pPartition->PCIndex].Path[pathLen] = '\0';
				pIoman->pPartition->PathCache[pIoman->pPartition->PCIndex].DirCluster = dirCluster;
#ifdef FF_HASH_TABLE_SUPPORT				
				FF_ClearHashTable(pIoman->pPartition->PathCache[pIoman->pPartition->PCIndex].pHashTable);
#endif			
				pIoman->pPartition->PCIndex += 1;
				if(pIoman->pPartition->PCIndex >= FF_PATH_CACHE_DEPTH) {
					pIoman->pPartition->PCIndex = 0;
				}
			}
		}
		FF_ReleaseSemaphore(pIoman->pSemaphore);
	 }
#endif

    return dirCluster;
}


/**
 *	@private
 **/
#ifdef FF_UNICODE_SUPPORT
static void FF_ProcessShortName(FF_T_WCHAR *name) {
	FF_T_WCHAR	shortName[13];
#else
static void FF_ProcessShortName(FF_T_INT8 *name) {
	FF_T_INT8	shortName[13];
#endif
	
	FF_T_UINT8	i;
#ifdef FF_UNICODE_SUPPORT
	memcpy(shortName, name, 11 * sizeof(FF_T_WCHAR));
#else
	memcpy(shortName, name, 11);
#endif
	
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

#ifdef FF_TIME_SUPPORT
static void FF_PlaceTime(FF_T_UINT8 *EntryBuffer, FF_T_UINT32 Offset) {
	FF_T_UINT16		myShort;
	FF_SYSTEMTIME	str_t;

	FF_GetSystemTime(&str_t);
				
	myShort = 0;
	myShort |= ((str_t.Hour    << 11) & 0xF800);
	myShort |= ((str_t.Minute  <<  5) & 0x07E0);
	myShort |= ((str_t.Second   /  2) & 0x001F);
	FF_putShort(EntryBuffer, (FF_T_UINT16) Offset, myShort);
}

static void FF_PlaceDate(FF_T_UINT8 *EntryBuffer, FF_T_UINT32 Offset) {
	FF_T_UINT16		myShort;
	FF_SYSTEMTIME	str_t;

	FF_GetSystemTime(&str_t);
	
	myShort = 0;
	myShort |= (((str_t.Year- 1980)  <<  9) & 0xFE00) ;
	myShort |= ((str_t.Month <<  5) & 0x01E0);
	myShort |= (str_t.Day & 0x001F);
	FF_putShort(EntryBuffer, (FF_T_UINT16) Offset, myShort);
}


static void FF_GetTime(FF_SYSTEMTIME *pTime, FF_T_UINT8 *EntryBuffer, FF_T_UINT32 Offset) {
	FF_T_UINT16 myShort;
	myShort = FF_getShort(EntryBuffer, (FF_T_UINT16) Offset);
	pTime->Hour		= (((myShort & 0xF800) >> 11) & 0x001F);
	pTime->Minute	= (((myShort & 0x07E0) >>  5) & 0x003F);
	pTime->Second	= 2 * (myShort & 0x01F);
}

static void FF_GetDate(FF_SYSTEMTIME *pTime, FF_T_UINT8 *EntryBuffer, FF_T_UINT32 Offset) {
	FF_T_UINT16 myShort;
	myShort = FF_getShort(EntryBuffer, (FF_T_UINT16) Offset);
	pTime->Year		= 1980 + (((myShort & 0xFE00) >> 9) & 0x07F);
	pTime->Month	= (((myShort & 0x01E0) >> 5) & 0x000F);
	pTime->Day		= myShort & 0x01F;
}



#endif

void FF_PopulateShortDirent(FF_IOMAN *pIoman, FF_DIRENT *pDirent, FF_T_UINT8 *EntryBuffer) {
	FF_T_UINT16 myShort;
#ifdef FF_UNICODE_SUPPORT
	FF_T_WCHAR UTF16EntryBuffer[12];
	FF_cstrntowcs(UTF16EntryBuffer, (FF_T_INT8 *) EntryBuffer, 11);
	memcpy(pDirent->FileName, UTF16EntryBuffer, 11 * sizeof(FF_T_WCHAR));
#else
	memcpy(pDirent->FileName, EntryBuffer, 11);	// Copy the filename into the Dirent object.
#endif
	FF_ProcessShortName(pDirent->FileName);		// Format the shortname, for pleasant viewing.

#ifdef FF_HASH_TABLE_SUPPORT
/*#if FF_HASH_FUNCTION == CRC16
	FF_AddDirentHash(pIoman, pDirent->DirCluster, (FF_T_UINT32)FF_GetCRC16((FF_T_UINT8 *) pDirent->FileName, strlen(pDirent->FileName)));
#elif FF_HASH_FUNCTION == CRC8
	FF_AddDirentHash(pIoman, pDirent->DirCluster, (FF_T_UINT32)FF_GetCRC8((FF_T_UINT8 *) pDirent->FileName, strlen(pDirent->FileName)));
#endif*/
#else
	pIoman = NULL;	// Silence a compiler warning, about not referencing pIoman.
#endif

#ifdef FF_UNICODE_SUPPORT
	FF_tolower(pDirent->FileName, (FF_T_UINT32)wcslen(pDirent->FileName));
#else
	FF_tolower(pDirent->FileName, (FF_T_UINT32)strlen(pDirent->FileName));
#endif

	// Get the item's Cluster address.
	myShort = FF_getShort(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_HIGH));
	pDirent->ObjectCluster = (FF_T_UINT32) (myShort << 16);
	myShort = FF_getShort(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_LOW));
	pDirent->ObjectCluster |= myShort;
#ifdef FF_TIME_SUPPORT
	// Get the creation Time & Date
	FF_GetTime(&pDirent->CreateTime, EntryBuffer, FF_FAT_DIRENT_CREATE_TIME);
	FF_GetDate(&pDirent->CreateTime, EntryBuffer, FF_FAT_DIRENT_CREATE_DATE);
	// Get the modified Time & Date
	FF_GetTime(&pDirent->CreateTime, EntryBuffer, FF_FAT_DIRENT_LASTMOD_TIME);
	FF_GetDate(&pDirent->CreateTime, EntryBuffer, FF_FAT_DIRENT_LASTMOD_DATE);
	// Get the last accessed Date.
	FF_GetDate(&pDirent->AccessedTime, EntryBuffer, FF_FAT_DIRENT_LASTACC_DATE);
	pDirent->AccessedTime.Hour		= 0;
	pDirent->AccessedTime.Minute	= 0;
	pDirent->AccessedTime.Second	= 0;
#endif
	// Get the filesize.
	pDirent->Filesize = FF_getLong(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_FILESIZE));
	// Get the attribute.
	pDirent->Attrib = FF_getChar(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB));
}

/*
	Initialises a context object for FF_FetchEntryWithContext()
*/
FF_ERROR FF_InitEntryFetch(FF_IOMAN *pIoman, FF_T_UINT32 ulDirCluster, FF_FETCH_CONTEXT *pContext) {
	
	FF_ERROR Error;

	memset(pContext, 0, sizeof(FF_FETCH_CONTEXT));

	pContext->ulChainLength 		= FF_GetChainLength(pIoman, ulDirCluster, NULL, &Error);	// Get the total length of the chain.
	if(Error) {
		return Error;
	}
	pContext->ulDirCluster			= ulDirCluster;
	pContext->ulCurrentClusterLCN 	= ulDirCluster;
	pContext->ulCurrentClusterNum 	= 0;
	pContext->ulCurrentEntry		= 0;

	if(pIoman->pPartition->Type != FF_T_FAT32) {
		// Handle Root Dirs that don't have cluster chains!
		if(pContext->ulDirCluster == pIoman->pPartition->RootDirCluster) {
			// This is a RootDIR, special consideration needs to be made, because it doesn't have a Cluster chain!
			pContext->ulChainLength = pIoman->pPartition->RootDirSectors / pIoman->pPartition->SectorsPerCluster;
			if(!pContext->ulChainLength) {		// Some media has RootDirSectors < SectorsPerCluster. This is wrong, as it should be atleast 1 cluster!
				pContext->ulChainLength = 1;
			}
		}
	}

	return FF_ERR_NONE;
}

void FF_CleanupEntryFetch(FF_IOMAN *pIoman, FF_FETCH_CONTEXT *pContext) {
	if(pContext->pBuffer) {
		FF_ReleaseBuffer(pIoman, pContext->pBuffer);
		pContext->pBuffer = NULL;
	}
}

FF_ERROR FF_FetchEntryWithContext(FF_IOMAN *pIoman, FF_T_UINT32 ulEntry, FF_FETCH_CONTEXT *pContext, FF_T_UINT8 *pEntryBuffer) {
	
	FF_T_UINT32	ulItemLBA;
	FF_T_UINT32	ulRelItem;
	FF_T_UINT32	ulClusterNum;
	FF_ERROR	Error;

	ulClusterNum 					= FF_getClusterChainNumber(pIoman, ulEntry, (FF_T_UINT16)32);
	ulRelItem						= FF_getMinorBlockEntry		(pIoman, ulEntry, (FF_T_UINT16)32);

	if(ulClusterNum != pContext->ulCurrentClusterNum) {
		// Traverse the fat gently!
		if(ulClusterNum > pContext->ulCurrentClusterNum) {
			pContext->ulCurrentClusterLCN = FF_TraverseFAT(pIoman, pContext->ulCurrentClusterLCN, (ulClusterNum - pContext->ulCurrentClusterNum), &Error);
			if(Error) {
				return Error;
			}
		} else {
			pContext->ulCurrentClusterLCN = FF_TraverseFAT(pIoman, pContext->ulDirCluster, ulClusterNum, &Error);
			if(Error) {
				return Error;
			}
		}
		pContext->ulCurrentClusterNum = ulClusterNum;
	}

	if(pIoman->pPartition->Type != FF_T_FAT32) {
		// Handle Root Dirs that don't have cluster chains!
		if(pContext->ulDirCluster == pIoman->pPartition->RootDirCluster) {
			// This is a RootDIR, special consideration needs to be made, because it doesn't have a Cluster chain!
			pContext->ulCurrentClusterLCN = pContext->ulDirCluster;
			ulClusterNum = 0;
			if(ulEntry > ((pIoman->pPartition->RootDirSectors * pIoman->pPartition->BlkSize) / 32)) {
				return FF_ERR_DIR_END_OF_DIR;
			}
		}
	}

	if((ulClusterNum + 1) > pContext->ulChainLength) {
		return FF_ERR_DIR_END_OF_DIR;	// End of Dir was reached!
	}

	ulItemLBA = FF_Cluster2LBA	(pIoman, pContext->ulCurrentClusterLCN) + FF_getMajorBlockNumber(pIoman, ulEntry, (FF_T_UINT16)32);
	ulItemLBA = FF_getRealLBA	(pIoman, ulItemLBA)	+ FF_getMinorBlockNumber(pIoman, ulRelItem, (FF_T_UINT16)32);

	if(!pContext->pBuffer || (pContext->pBuffer->Sector != ulItemLBA)) {
		if(pContext->pBuffer) {
			FF_ReleaseBuffer(pIoman, pContext->pBuffer);
		}
		pContext->pBuffer = FF_GetBuffer(pIoman, ulItemLBA, FF_MODE_READ);
		if(!pContext->pBuffer) {
			return FF_ERR_DEVICE_DRIVER_FAILED;
		}
	}

	memcpy(pEntryBuffer, (pContext->pBuffer->pBuffer + (ulRelItem*32)), 32);
	 
    return FF_ERR_NONE;
}


FF_ERROR FF_PushEntryWithContext(FF_IOMAN *pIoman, FF_T_UINT32 ulEntry, FF_FETCH_CONTEXT *pContext, FF_T_UINT8 *pEntryBuffer) {
	FF_T_UINT32	ulItemLBA;
	FF_T_UINT32	ulRelItem;
	FF_T_UINT32	ulClusterNum;
	FF_ERROR	Error;

	ulClusterNum 					= FF_getClusterChainNumber(pIoman, ulEntry, (FF_T_UINT16)32);
	ulRelItem						= FF_getMinorBlockEntry		(pIoman, ulEntry, (FF_T_UINT16)32);

	if(ulClusterNum != pContext->ulCurrentClusterNum) {
		// Traverse the fat gently!
		if(ulClusterNum > pContext->ulCurrentClusterNum) {
			pContext->ulCurrentClusterLCN = FF_TraverseFAT(pIoman, pContext->ulCurrentClusterLCN, (ulClusterNum - pContext->ulCurrentClusterNum), &Error);
			if(Error) {
				return Error;
			}
		} else {
			pContext->ulCurrentClusterLCN = FF_TraverseFAT(pIoman, pContext->ulDirCluster, ulClusterNum, &Error);
			if(Error) {
				return Error;
			}
		}
		pContext->ulCurrentClusterNum = ulClusterNum;
	}

	if(pIoman->pPartition->Type != FF_T_FAT32) {
		// Handle Root Dirs that don't have cluster chains!
		if(pContext->ulDirCluster == pIoman->pPartition->RootDirCluster) {
			// This is a RootDIR, special consideration needs to be made, because it doesn't have a Cluster chain!
			pContext->ulCurrentClusterLCN = pContext->ulDirCluster;
			ulClusterNum = 0;
			if(ulEntry > ((pIoman->pPartition->RootDirSectors * pIoman->pPartition->BlkSize) / 32)) {
				return FF_ERR_DIR_END_OF_DIR;
			}
		}
	}

	if((ulClusterNum + 1) > pContext->ulChainLength) {
		return FF_ERR_DIR_END_OF_DIR;	// End of Dir was reached!
	}

	ulItemLBA = FF_Cluster2LBA	(pIoman, pContext->ulCurrentClusterLCN) + FF_getMajorBlockNumber(pIoman, ulEntry, (FF_T_UINT16)32);
	ulItemLBA = FF_getRealLBA	(pIoman, ulItemLBA)	+ FF_getMinorBlockNumber(pIoman, ulRelItem, (FF_T_UINT16)32);

	if(!pContext->pBuffer || (pContext->pBuffer->Sector != ulItemLBA)) {
		if(pContext->pBuffer) {
			FF_ReleaseBuffer(pIoman, pContext->pBuffer);
		}
		pContext->pBuffer = FF_GetBuffer(pIoman, ulItemLBA, FF_MODE_READ);
		if(!pContext->pBuffer) {
			return FF_ERR_DEVICE_DRIVER_FAILED;
		}
	}

	memcpy((pContext->pBuffer->pBuffer + (ulRelItem*32)), pEntryBuffer, 32);
	pContext->pBuffer->Mode = FF_MODE_WRITE;
	pContext->pBuffer->Modified = FF_TRUE;
	 
    return FF_ERR_NONE;
}


/**
 *	@private
 **/
FF_ERROR FF_GetEntry(FF_IOMAN *pIoman, FF_T_UINT16 nEntry, FF_T_UINT32 DirCluster, FF_DIRENT *pDirent) {
	FF_T_UINT8			EntryBuffer[32];
	FF_T_UINT8			numLFNs;
	FF_FETCH_CONTEXT	FetchContext;
	FF_ERROR			Error;

	Error = FF_InitEntryFetch(pIoman, DirCluster, &FetchContext);
	if(Error) {
		return Error;
	}
	
	Error = FF_FetchEntryWithContext(pIoman, nEntry, &FetchContext, EntryBuffer);
	if(Error) {
		FF_CleanupEntryFetch(pIoman, &FetchContext);
		return Error;
	}
	if(EntryBuffer[0] != 0xE5) {
		if(FF_isEndOfDir(EntryBuffer)){
			FF_CleanupEntryFetch(pIoman, &FetchContext);
			return FF_ERR_DIR_END_OF_DIR;
		}
		
		pDirent->Attrib = FF_getChar(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB));
		
		if((pDirent->Attrib & FF_FAT_ATTR_LFN) == FF_FAT_ATTR_LFN) {
			// LFN Processing
			numLFNs = (FF_T_UINT8)(EntryBuffer[0] & ~0x40);
	#ifdef FF_LFN_SUPPORT
			Error = FF_PopulateLongDirent(pIoman, pDirent, nEntry, &FetchContext);
			FF_CleanupEntryFetch(pIoman, &FetchContext);
			if(Error) {
				return Error;
			}
			return FF_ERR_NONE;
	#else 
			pDirent->CurrentItem += (numLFNs - 1);
	#endif
		} else if((pDirent->Attrib & FF_FAT_ATTR_VOLID) == FF_FAT_ATTR_VOLID) {
			// Do Nothing
		
		} else {
			FF_PopulateShortDirent(pIoman, pDirent, EntryBuffer);
			pDirent->CurrentItem += 1;
			FF_CleanupEntryFetch(pIoman, &FetchContext);
			return 0;
		}
	}

	FF_CleanupEntryFetch(pIoman, &FetchContext);
	return FF_ERR_NONE;
}

FF_T_BOOL FF_isEndOfDir(FF_T_UINT8 *EntryBuffer) {
	if(EntryBuffer[0] == 0x00) {
		return FF_TRUE;
	}
	return FF_FALSE;
}

#ifdef FF_HASH_CACHE
FF_ERROR FF_AddDirentHash(FF_IOMAN *pIoman, FF_T_UINT32 ulDirCluster, FF_T_UINT32 ulHash) {
	FF_T_UINT32 i;
	FF_HASH_TABLE pHash = NULL;
	for(i = 0; i < FF_HASH_CACHE_DEPTH; i++) {
		if(pIoman->HashCache[i].ulDirCluster == ulDirCluster) {
			pHash = pIoman->HashCache[i].pHashTable;
			break;
		}
	}

	if(pHash) {
		FF_SetHash(pHash, ulHash);
	}

	return FF_ERR_NONE;
}

FF_T_BOOL FF_CheckDirentHash(FF_IOMAN *pIoman, FF_T_UINT32 ulDirCluster, FF_T_UINT32 ulHash) {
	FF_T_UINT32 i;
	FF_HASH_TABLE pHash = NULL;
	for(i = 0; i < FF_HASH_CACHE_DEPTH; i++) {
		if(pIoman->HashCache[i].ulDirCluster == ulDirCluster) {
			pHash = pIoman->HashCache[i].pHashTable;
			break;
		}
	}

	if(pHash) {
		return FF_isHashSet(pHash, ulHash);
	}

	return FF_FALSE;
}

FF_T_BOOL FF_DirHashed(FF_IOMAN *pIoman, FF_T_UINT32 ulDirCluster) {
	FF_T_UINT32 i;
	for(i = 0; i < FF_HASH_CACHE_DEPTH; i++) {
		if(pIoman->HashCache[i].ulDirCluster == ulDirCluster) {
			return FF_TRUE;
		}
	}

	return FF_FALSE;
}

FF_ERROR FF_HashDir(FF_IOMAN *pIoman, FF_T_UINT32 ulDirCluster) {
	// Find most suitable Hash Table to replace!
	FF_T_UINT32 		i;
	FF_HASHCACHE 		*pHashCache = NULL;
	FF_FETCH_CONTEXT	FetchContext;
	FF_T_UINT8			EntryBuffer[32], ucAttrib;
	FF_T_UINT32			ulHash;

	if(FF_DirHashed(pIoman, ulDirCluster)) {
		return FF_ERR_NONE;			// Don't wastefully re-hash a dir!
	}

	//printf("----- Hashing Directory\n");

	for(i = 0; i < FF_HASH_CACHE_DEPTH; i++) {
		if(!pIoman->HashCache[i].ulNumHandles) {
			if(!pHashCache) {
				pHashCache = &pIoman->HashCache[i];
			} else {
				if((pIoman->HashCache[i].ulMisses > pHashCache->ulMisses)) {
					pHashCache = &pIoman->HashCache[i];
				}
			}
		}
	}

	if(pHashCache) {
		// Clear the hash table!
		FF_ClearHashTable(pHashCache->pHashTable);
		pHashCache->ulDirCluster = ulDirCluster;
		pHashCache->ulMisses = 0;
		
		// Hash the directory!
		
		FF_InitEntryFetch(pIoman, ulDirCluster, &FetchContext);

		for(i = 0; i < 0xFFFF; i++) {
			FF_FetchEntryWithContext(pIoman, i, &FetchContext, EntryBuffer);
			ucAttrib = FF_getChar(EntryBuffer, FF_FAT_DIRENT_ATTRIB);
			if(FF_getChar(EntryBuffer, 0x00) != 0xE5) {
				if(ucAttrib != FF_FAT_ATTR_LFN) {
					FF_ProcessShortName((FF_T_INT8 *)EntryBuffer);
					if(FF_isEndOfDir(EntryBuffer)) {
						//return FF_FALSE;
					}
					
					// Generate the Hash
#if FF_HASH_FUNCTION == CRC16
					ulHash = FF_GetCRC16(EntryBuffer, strlen((const FF_T_INT8 *) EntryBuffer));
#elif FF_HASH_FUNCTION == CRC8
					ulHash = FF_GetCRC8(EntryBuffer, strlen((const FF_T_INT8 *) EntryBuffer));
#endif
					FF_SetHash(pHashCache->pHashTable, ulHash);
					
				}
			}
		}

		FF_CleanupEntryFetch(pIoman, &FetchContext);

		return FF_ERR_NONE;
	}

	return -1;
}

/*void FF_SetDirHashed(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster) {
	int i;
	for(i = 0; i < FF_PATH_CACHE_DEPTH; i++) {
		if(pIoman->pPartition->PathCache[i].DirCluster == DirCluster) {
			pIoman->pPartition->PathCache[i].bHashed = FF_TRUE;
			return;
		}
	}
}*/
#endif


FF_ERROR FF_PopulateLongDirent(FF_IOMAN *pIoman, FF_DIRENT *pDirent, FF_T_UINT16 nEntry, FF_FETCH_CONTEXT *pFetchContext) {
	FF_T_UINT8	EntryBuffer[32];
#ifdef FF_UNICODE_SUPPORT
	FF_T_WCHAR	UTF16EntryBuffer[32];
	FF_T_WCHAR	ShortName[13];
#if WCHAR_MAX > 0xFFFF
	FF_T_UINT16 i,y;
#endif
#else
	FF_T_INT8	ShortName[13];
	FF_T_UINT16 i,y;
#endif
	FF_T_UINT8 numLFNs;
	FF_T_UINT8 x;
	FF_T_UINT8 CheckSum = 0;

	FF_T_UINT16 lenlfn = 0;
	FF_T_UINT16 myShort;
	FF_ERROR	Error;
	
	Error = FF_FetchEntryWithContext(pIoman, nEntry++, pFetchContext, EntryBuffer);
	if(Error) {
		return Error;
	}

	numLFNs = (FF_T_UINT8)(EntryBuffer[0] & ~0x40);
	// Handle the name
	CheckSum = FF_getChar(EntryBuffer, FF_FAT_LFN_CHECKSUM);

	x = numLFNs;
	while(numLFNs) {
		if(numLFNs > 1) {
			numLFNs = numLFNs;
		}

#ifdef FF_UNICODE_SUPPORT
		// Simply fill the FileName buffer with UTF-16 Filename!
#if WCHAR_MAX <= 0xFFFF
		memcpy(pDirent->FileName + ((numLFNs - 1) * 13) + 0,	&EntryBuffer[FF_FAT_LFN_NAME_1], (5 * sizeof(FF_T_WCHAR)));
		memcpy(pDirent->FileName + ((numLFNs - 1) * 13) + 5,	&EntryBuffer[FF_FAT_LFN_NAME_2], (6 * sizeof(FF_T_WCHAR)));
		memcpy(pDirent->FileName + ((numLFNs - 1) * 13) + 11,	&EntryBuffer[FF_FAT_LFN_NAME_3], (2 * sizeof(FF_T_WCHAR)));
		lenlfn += 13;
#else
		for(i = 0, y = 0; i < 5; i++, y += 2) {
			FF_Utf16ctoUtf32c((FF_T_UINT32 *)&pDirent->FileName[i + ((numLFNs - 1) * 13)], (FF_T_UINT16 *) &EntryBuffer[FF_FAT_LFN_NAME_1 + y]);
			//pDirent->FileName[i + ((numLFNs - 1) * 13)] = (FF_T_WCHAR) ((FF_T_WCHAR) EntryBuffer[FF_FAT_LFN_NAME_1 + y] | ((FF_T_WCHAR) EntryBuffer[FF_FAT_LFN_NAME_1 + y + 1] >> 8));
			lenlfn++;
		}
		for(i = 0, y = 0; i < 6; i++, y += 2) {
			FF_Utf16ctoUtf32c((FF_T_UINT32 *)&pDirent->FileName[i + ((numLFNs - 1) * 13) + 5], (FF_T_UINT16 *) &EntryBuffer[FF_FAT_LFN_NAME_2 + y]);
			//pDirent->FileName[i + ((numLFNs - 1) * 13) + 5] = (FF_T_WCHAR) ((FF_T_WCHAR) EntryBuffer[FF_FAT_LFN_NAME_2 + y] | ((FF_T_WCHAR) EntryBuffer[FF_FAT_LFN_NAME_2 + y + 1] >> 8));
			lenlfn++;
		}
		for(i = 0, y = 0; i < 2; i++, y += 2) {
			FF_Utf16ctoUtf32c((FF_T_UINT32 *)&pDirent->FileName[i + ((numLFNs - 1) * 13) + 11], (FF_T_UINT16 *) &EntryBuffer[FF_FAT_LFN_NAME_3 + y]);
			//pDirent->FileName[i + ((numLFNs - 1) * 13) + 11] = (FF_T_WCHAR) ((FF_T_WCHAR) EntryBuffer[FF_FAT_LFN_NAME_3 + y] | ((FF_T_WCHAR)EntryBuffer[FF_FAT_LFN_NAME_3 + y + 1] >> 8));
			lenlfn++;
		}
#endif
		// Copy each part of the LFNS
#else
		// Attempts to pull ASCII from UTF-8 encoding. 
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
#endif

		Error = FF_FetchEntryWithContext(pIoman, nEntry++, pFetchContext, EntryBuffer);
		if(Error) {
			return Error;
		}
		numLFNs--;
	}

	pDirent->FileName[lenlfn] = '\0';
	
	// Process the ShortName Entry
#ifdef FF_UNICODE_SUPPORT
	FF_cstrntowcs(UTF16EntryBuffer, (FF_T_INT8 *) EntryBuffer, 32);
	memcpy(ShortName, UTF16EntryBuffer, 11 * sizeof(FF_T_WCHAR));
#else
	memcpy(ShortName, EntryBuffer, 11);
#endif
	if(CheckSum != FF_CreateChkSum(EntryBuffer)) {
		FF_ProcessShortName(ShortName);
#ifdef FF_UNICODE_SUPPORT
		wcscpy(pDirent->FileName, ShortName);
#else
		strcpy(pDirent->FileName, ShortName);
#endif
	} else {
		FF_ProcessShortName(ShortName);
	}

#ifdef FF_HASH_TABLE_SUPPORT
/*#if FF_HASH_FUNCTION == CRC16
	FF_AddDirentHash(pIoman, pFetchContext->ulDirCluster, (FF_T_UINT32)FF_GetCRC16((FF_T_UINT8 *) ShortName, strlen(ShortName)));
#elif FF_HASH_FUNCTION == CRC8
	FF_AddDirentHash(pIoman, DirCluster, (FF_T_UINT32)FF_GetCRC8((FF_T_UINT8 *) ShortName, strlen(ShortName)));
#endif*/
#endif
	
	myShort = FF_getShort(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_HIGH));
	pDirent->ObjectCluster = (FF_T_UINT32) (myShort << 16);
	myShort = FF_getShort(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_LOW));
	pDirent->ObjectCluster |= myShort;

#ifdef FF_TIME_SUPPORT
	// Get the creation Time & Date
	FF_GetTime(&pDirent->CreateTime, EntryBuffer, FF_FAT_DIRENT_CREATE_TIME);
	FF_GetDate(&pDirent->CreateTime, EntryBuffer, FF_FAT_DIRENT_CREATE_DATE);
	// Get the modified Time & Date
	FF_GetTime(&pDirent->CreateTime, EntryBuffer, FF_FAT_DIRENT_LASTMOD_TIME);
	FF_GetDate(&pDirent->CreateTime, EntryBuffer, FF_FAT_DIRENT_LASTMOD_DATE);
	// Get the last accessed Date.
	FF_GetDate(&pDirent->AccessedTime, EntryBuffer, FF_FAT_DIRENT_LASTACC_DATE);
	pDirent->AccessedTime.Hour		= 0;
	pDirent->AccessedTime.Minute	= 0;
	pDirent->AccessedTime.Second	= 0;
#endif

	// Get the filesize.
	pDirent->Filesize = FF_getLong(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_FILESIZE));
	// Get the attribute.
	pDirent->Attrib = FF_getChar(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB));
	
	pDirent->CurrentItem = nEntry;
	//return x;
	return FF_ERR_NONE;
}

/**
 *	@public
 *	@brief	Find's the first directory entry for the provided path.
 *
 *	All values recorded in pDirent must be preserved to and between calls to
 *	FF_FindNext().
 *
 *	If FF_FINDAPI_ALLOW_WILDCARDS is defined, then path will have the following behaviour:
 *
 *	path = "\" 					- Open the root dir, and iterate through all items.
 *	path = "\*.c"				- Open the root dir, showing only files matching *.c wildcard.
 *	path = "\sub1\newdir"		- Get the DIRENT for the newdir directory in /sub1/ if one exists.
 *	path = "\sub1\newdir\"		- Open the directory /sub1/newdir/ and iterate through all items.
 *	path = "\sub1\newdir\*.c"	- Open the directory /sub1/newdir/ and iterate through all items matching the *.c wildcard.
 *
 *	It is important to distinguish the differences in behaviour between opening a Find operation
 *	on a path like /sub1 and /sub1/. (/sub1 gets the sub1 dirent from the / dir, whereas /sub/ opens the sub1 dir).
 *
 *	Note, as compatible with other similar APIs, FullFAT also accepts \sub1\* for the same behaviour as
 *	/sub1/.
 *
 *	For more up-to-date information please see the FullFAT wiki pages.
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
#ifdef FF_UNICODE_SUPPORT
FF_ERROR FF_FindFirst(FF_IOMAN *pIoman, FF_DIRENT *pDirent, const FF_T_WCHAR *path) {
#else
FF_ERROR FF_FindFirst(FF_IOMAN *pIoman, FF_DIRENT *pDirent, const FF_T_INT8 *path) {
#endif
	FF_T_UINT8	numLFNs;
	FF_T_UINT8	EntryBuffer[32];
#ifdef FF_UNICODE_SUPPORT
	FF_T_UINT16	PathLen = (FF_T_UINT16) wcslen(path);
#else
	FF_T_UINT16	PathLen = (FF_T_UINT16) strlen(path);
#endif
	FF_ERROR	Error;

#ifdef FF_FINDAPI_ALLOW_WILDCARDS	
	FF_T_UINT16 i = 0;
#ifdef FF_UNICODE_SUPPORT
	const FF_T_WCHAR *szWildCard;	// Check for a Wild-card.
#else
	const FF_T_INT8	*szWildCard;	// Check for a Wild-card.
#endif
#endif

	if(!pIoman) {
		return FF_ERR_NULL_POINTER;
	}

	// Detect a Wild-Card on the End, or Filename, as apposed to a complete path.
#ifndef FF_FINDAPI_ALLOW_WILDCARDS
	pDirent->DirCluster = FF_FindDir(pIoman, path, PathLen);	// Get the directory cluster, if it exists.
#endif

#ifdef FF_FINDAPI_ALLOW_WILDCARDS
	pDirent->szWildCard[0] = '\0';	// WildCard blank if its not a wildCard.

	szWildCard = &path[PathLen - 1];

	if(PathLen) {
		while(*szWildCard != '\\' && *szWildCard != '/') {	// Open the dir of the last token.
			i++;
			szWildCard--;
			if(!(PathLen - i)) {
				break;
			}
		}
	}
			
	pDirent->DirCluster = FF_FindDir(pIoman, path, PathLen - i, &Error);
	if(Error) {
		return Error;
	}
	if(pDirent->DirCluster) {
		// Valid Dir found, copy the wildCard to filename!
#ifdef FF_UNICODE_SUPPORT
		wcsncpy(pDirent->szWildCard, ++szWildCard, FF_MAX_FILENAME);
#else
		strncpy(pDirent->szWildCard, ++szWildCard, FF_MAX_FILENAME);
#endif
	}
#endif

	if(pDirent->DirCluster == 0) {
		return FF_ERR_DIR_INVALID_PATH;
	}

	// Initialise the Fetch Context
	Error = FF_InitEntryFetch(pIoman, pDirent->DirCluster, &pDirent->FetchContext);
	if(Error) {
		return Error;
	}
	
	for(pDirent->CurrentItem = 0; pDirent->CurrentItem < 0xFFFF; pDirent->CurrentItem += 1) {
		Error =	FF_FetchEntryWithContext(pIoman, pDirent->CurrentItem, &pDirent->FetchContext, EntryBuffer);
		if(Error) {
			FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
			return Error;
		}
		if(EntryBuffer[0] != FF_FAT_DELETED) {
			if(FF_isEndOfDir(EntryBuffer)){
				FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
				return FF_ERR_DIR_END_OF_DIR;
			}
			pDirent->Attrib = FF_getChar(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB));
			if((pDirent->Attrib & FF_FAT_ATTR_LFN) == FF_FAT_ATTR_LFN) {
				// LFN Processing
				numLFNs = (FF_T_UINT8)(EntryBuffer[0] & ~0x40);
				// Get the shortname and check if it is marked deleted.
#ifdef FF_LFN_SUPPORT
				// Fetch the shortname, and get it's checksum, or for a deleted item with
				// orphaned LFN entries.
				
				Error = FF_FetchEntryWithContext(pIoman, (pDirent->CurrentItem + numLFNs), &pDirent->FetchContext, EntryBuffer);
				if(Error) {
					FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
					return Error;
				}
				
				if(EntryBuffer[0] != FF_FAT_DELETED) {
					Error = FF_PopulateLongDirent(pIoman, pDirent, pDirent->CurrentItem, &pDirent->FetchContext);
					if(Error) {
						FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
						return Error;
					}
					
#ifdef FF_FINDAPI_ALLOW_WILDCARDS
#ifdef FF_UNICODE_SUPPORT
					if(wcscmp(pDirent->szWildCard, L"")) {
#else
					if(strcmp(pDirent->szWildCard, "")) {
#endif
						if(FF_wildcompare(pDirent->szWildCard, pDirent->FileName)) {
							FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
							return FF_ERR_NONE;							
						}
						pDirent->CurrentItem -= 1;
					} else {
						FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
						return FF_ERR_NONE;
					}
#else
					FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
					return FF_ERR_NONE;
#endif
				}
#else 
				pDirent->CurrentItem += (numLFNs - 1);
#endif
			} else if((pDirent->Attrib & FF_FAT_ATTR_VOLID) == FF_FAT_ATTR_VOLID) {
				// Do Nothing
			
			} else {
				FF_PopulateShortDirent(pIoman, pDirent, EntryBuffer);
#ifdef FF_FINDAPI_ALLOW_WILDCARDS
#ifdef FF_UNICODE_SUPPORT
				if(wcscmp(pDirent->szWildCard, L"")) {
#else
				if(strcmp(pDirent->szWildCard, "")) {
#endif
					if(FF_wildcompare(pDirent->szWildCard, pDirent->FileName)) {
						FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
						pDirent->CurrentItem += 1;
						return FF_ERR_NONE;							
					}
				} else {
					FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
					pDirent->CurrentItem += 1;
					return FF_ERR_NONE;
				}
#else

				FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
				pDirent->CurrentItem += 1;
				return FF_ERR_NONE;
#endif
			}
		}
	}

	FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);

	return FF_ERR_DIR_END_OF_DIR;
}

/**
 *	@public
 *	@brief	Get's the next Entry based on the data recorded in the FF_DIRENT object.
 *
 *	All values recorded in pDirent must be preserved to and between calls to
 *	FF_FindNext(). Please see @see FF_FindFirst() for find initialisation.
 *
 *	@param	pIoman		FF_IOMAN object that was created by FF_CreateIOMAN().
 *	@param	pDirent		FF_DIRENT object to store the entry information. (As initialised by FF_FindFirst()).
 *
 *	@return FF_ERR_DEVICE_DRIVER_FAILED is device access failed.
 *
 **/
FF_ERROR FF_FindNext(FF_IOMAN *pIoman, FF_DIRENT *pDirent) {
	
	FF_ERROR	Error;
	FF_T_UINT8	numLFNs;
	FF_T_UINT8	EntryBuffer[32];

	if(!pIoman) {
		return FF_ERR_NULL_POINTER;
	}
	
	for(; pDirent->CurrentItem < 0xFFFF; pDirent->CurrentItem += 1) {
		Error = FF_FetchEntryWithContext(pIoman, pDirent->CurrentItem, &pDirent->FetchContext, EntryBuffer);
		if(Error) {
			FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
			return Error;
		}
		if(EntryBuffer[0] != FF_FAT_DELETED) {
			if(FF_isEndOfDir(EntryBuffer)){
				FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
				return FF_ERR_DIR_END_OF_DIR;
			}
			pDirent->Attrib = FF_getChar(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB));
			if((pDirent->Attrib & FF_FAT_ATTR_LFN) == FF_FAT_ATTR_LFN) {
				// LFN Processing
				numLFNs = (FF_T_UINT8)(EntryBuffer[0] & ~0x40);
				// Get the shortname and check if it is marked deleted.
#ifdef FF_LFN_SUPPORT
				// Fetch the shortname, and get it's checksum, or for a deleted item with
				// orphaned LFN entries.
				Error = FF_FetchEntryWithContext(pIoman, (pDirent->CurrentItem + numLFNs), &pDirent->FetchContext, EntryBuffer);
				if(Error) {
					FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
					return Error;
				}
				
				if(EntryBuffer[0] != FF_FAT_DELETED) {
					Error = FF_PopulateLongDirent(pIoman, pDirent, pDirent->CurrentItem, &pDirent->FetchContext);
					if(Error) {
						FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
						return Error;
					}
#ifdef FF_FINDAPI_ALLOW_WILDCARDS
#ifdef FF_UNICODE_SUPPORT
					if(wcscmp(pDirent->szWildCard, L"")) {
#else
					if(strcmp(pDirent->szWildCard, "")) {
#endif
						if(FF_wildcompare(pDirent->szWildCard, pDirent->FileName)) {
							FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
							return FF_ERR_NONE;							
						}
						pDirent->CurrentItem -= 1;
					} else {
						FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
						return FF_ERR_NONE;
					}
#else
					FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
					return FF_ERR_NONE;
#endif
				}
#else 
				pDirent->CurrentItem += (numLFNs - 1);
#endif
			} else if((pDirent->Attrib & FF_FAT_ATTR_VOLID) == FF_FAT_ATTR_VOLID) {
				// Do Nothing
			
			} else {
				FF_PopulateShortDirent(pIoman, pDirent, EntryBuffer);
#ifdef FF_FINDAPI_ALLOW_WILDCARDS
#ifdef FF_UNICODE_SUPPORT
				if(wcscmp(pDirent->szWildCard, L"")) {
#else
				if(strcmp(pDirent->szWildCard, "")) {
#endif
					if(FF_wildcompare(pDirent->szWildCard, pDirent->FileName)) {
						FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
						pDirent->CurrentItem += 1;
						return FF_ERR_NONE;
					}
				} else {
					FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
					pDirent->CurrentItem += 1;
					return FF_ERR_NONE;
				}
#else

				FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
				pDirent->CurrentItem += 1;
				return FF_ERR_NONE;
#endif
			}
		}
	}

	FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
	
	return FF_ERR_DIR_END_OF_DIR;
}


FF_T_SINT8 FF_RewindFind(FF_IOMAN *pIoman, FF_DIRENT *pDirent) {
	if(!pIoman) {
		return FF_ERR_NULL_POINTER;
	}
	pDirent->CurrentItem = 0;
	return 0;
}

/*
	Returns >= 0 for a free dirent entry.
	Returns <  0 with and Error code if anything goes wrong.
*/
static FF_T_SINT32 FF_FindFreeDirent(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_T_UINT16 Sequential) {

	FF_T_UINT8			EntryBuffer[32];
	FF_T_UINT16			i = 0;
	FF_T_UINT16			nEntry;
	FF_ERROR			Error;
	FF_T_UINT32			DirLength;
	FF_FETCH_CONTEXT	FetchContext;

	Error = FF_InitEntryFetch(pIoman, DirCluster, &FetchContext);
	if(Error) {
		return Error;
	}
	
	for(nEntry = 0; nEntry < 0xFFFF; nEntry++) {
		Error = FF_FetchEntryWithContext(pIoman, nEntry, &FetchContext, EntryBuffer);
		if(Error == FF_ERR_DIR_END_OF_DIR) {
			
			Error = FF_ExtendDirectory(pIoman, DirCluster);
			FF_CleanupEntryFetch(pIoman, &FetchContext);

			if(Error) {
				return Error;
			}

			return nEntry;
		} else {
			if(Error) {
				FF_CleanupEntryFetch(pIoman, &FetchContext);
				return Error;
			}
		}
		if(FF_isEndOfDir(EntryBuffer)) {	// If its the end of the Dir, then FreeDirents from here.
			// Check Dir is long enough!
			DirLength = FetchContext.ulChainLength;//FF_GetChainLength(pIoman, DirCluster, &iEndOfChain);
			if((nEntry + Sequential) > (FF_T_UINT16)(((pIoman->pPartition->SectorsPerCluster * pIoman->pPartition->BlkSize) * DirLength) / 32)) {
				Error = FF_ExtendDirectory(pIoman, DirCluster);
			}

			FF_CleanupEntryFetch(pIoman, &FetchContext);

			if(Error) {
				return Error;
			}

			return nEntry;
		}
		if(EntryBuffer[0] == 0xE5) {
			i++;
		} else {
			i = 0;
		}

		if(i == Sequential) {
			FF_CleanupEntryFetch(pIoman, &FetchContext);
			return (nEntry - (Sequential - 1));// Return the beginning entry in the sequential sequence.
		}
	}
	
	FF_CleanupEntryFetch(pIoman, &FetchContext);

	return FF_ERR_DIR_DIRECTORY_FULL;	
}




FF_ERROR FF_PutEntry(FF_IOMAN *pIoman, FF_T_UINT16 Entry, FF_T_UINT32 DirCluster, FF_DIRENT *pDirent) {
	FF_BUFFER *pBuffer;
	FF_ERROR	Error;
	FF_T_UINT32 itemLBA;
	FF_T_UINT32 clusterNum		= FF_getClusterChainNumber	(pIoman, Entry, (FF_T_UINT16)32);
	FF_T_UINT32 relItem			= FF_getMinorBlockEntry		(pIoman, Entry, (FF_T_UINT16)32);
	FF_T_UINT32 clusterAddress	= FF_TraverseFAT(pIoman, DirCluster, clusterNum, &Error);

	if(Error) {
		return Error;
	}

	itemLBA = FF_Cluster2LBA(pIoman, clusterAddress)	+ FF_getMajorBlockNumber(pIoman, Entry, (FF_T_UINT16)32);
	itemLBA = FF_getRealLBA	(pIoman, itemLBA)			+ FF_getMinorBlockNumber(pIoman, relItem, (FF_T_UINT16)32);
	
	pBuffer = FF_GetBuffer(pIoman, itemLBA, FF_MODE_WRITE);
	{
		if(!pBuffer) {
			return FF_ERR_DEVICE_DRIVER_FAILED;
		}
		// Modify the Entry!
		//memcpy((pBuffer->pBuffer + (32*relItem)), pDirent->FileName, 11);
		FF_putChar(pBuffer->pBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB + (32 * relItem)), pDirent->Attrib);
		FF_putShort(pBuffer->pBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_HIGH + (32 * relItem)), (FF_T_UINT16)(pDirent->ObjectCluster >> 16));
		FF_putShort(pBuffer->pBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_LOW  + (32 * relItem)), (FF_T_UINT16)(pDirent->ObjectCluster));
		FF_putLong(pBuffer->pBuffer,  (FF_T_UINT16)(FF_FAT_DIRENT_FILESIZE  + (32 * relItem)), pDirent->Filesize);
#ifdef FF_TIME_SUPPORT
	FF_PlaceDate((pBuffer->pBuffer + (32 * relItem)), FF_FAT_DIRENT_LASTACC_DATE);	// Last accessed date.
#endif
	}
	FF_ReleaseBuffer(pIoman, pBuffer);
 
    return 0;
}
#ifdef FF_UNICODE_SUPPORT
FF_ERROR FF_CreateShortName(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_T_WCHAR *ShortName, FF_T_WCHAR *LongName) {
#else
FF_ERROR FF_CreateShortName(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_T_INT8 *ShortName, FF_T_INT8 *LongName) {
#endif
	FF_T_UINT16 i,x,y;
#ifdef FF_UNICODE_SUPPORT
	FF_T_WCHAR	TempName[FF_MAX_FILENAME];
	FF_T_WCHAR	MyShortName[13];
	FF_T_WCHAR	NumberBuf[6];
#else
	FF_T_INT8	TempName[FF_MAX_FILENAME];
	FF_T_INT8	MyShortName[13];
	FF_T_INT8	NumberBuf[6];
#endif
	FF_T_UINT16 NameLen; 
//	FF_T_BOOL	FitsShort = FF_FALSE;
//	FF_DIRENT	MyDir;
	
	FF_ERROR	Error;
	// Create a Short Name
#ifdef FF_UNICODE_SUPPORT
	wcsncpy(TempName, LongName, FF_MAX_FILENAME);
	NameLen = (FF_T_UINT16) wcslen(TempName);
#else
	strncpy(TempName, LongName, FF_MAX_FILENAME);
	NameLen = (FF_T_UINT16) strlen(TempName);
#endif
	
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

	/*if(x <= 11) {
		//FitsShort = FF_TRUE;
	}*/

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
#ifdef FF_UNICODE_SUPPORT
	memcpy(MyShortName, ShortName, 11 * sizeof(FF_T_WCHAR));
#else
	memcpy(MyShortName, ShortName, 11);
#endif
	
	FF_ProcessShortName(MyShortName);
	
	/*if(FitsShort && !FF_FindEntryInDir(pIoman, DirCluster, MyShortName, 0x00, &MyDir, &Error)) {
		if(Error) {
			return Error;	// If there was an error report it.
		}
		return FF_ERR_NONE;	// Else return No Error.
	} else {
		if(FitsShort) {
			return FF_ERR_DIR_OBJECT_EXISTS;
		}*/
		for(i = 1; i < 0x0000FFFF; i++) { // Max Number of Entries in a DIR!
#ifdef FF_UNICODE_SUPPORT
			swprintf(NumberBuf, 6, L"%d", i);
			NameLen = (FF_T_UINT16) wcslen(NumberBuf);
#else
			sprintf(NumberBuf, "%d", i);
			NameLen = (FF_T_UINT16) strlen(NumberBuf);
#endif
			x = 7 - NameLen;
			ShortName[x++] = '~';
			for(y = 0; y < NameLen; y++) {
				ShortName[x+y] = NumberBuf[y];
			}
#ifdef FF_UNICODE_SUPPORT
			memcpy(MyShortName, ShortName, 11 * sizeof(FF_T_WCHAR));
#else
			memcpy(MyShortName, ShortName, 11);
#endif
			FF_ProcessShortName(MyShortName);
			
			if(!FF_ShortNameExists(pIoman, DirCluster, MyShortName, &Error)) {

				if(Error) {
					return Error;
				}

				//FF_AddDirentHash(pIoman, DirCluster, (FF_T_UINT32) FF_GetCRC16(MyShortName, strlen(MyShortName)));
				return FF_ERR_NONE;
			}

			if(Error) {
				return Error;
			}
		}
		// Add a tail and special number until we're happy :D
	//}

	return FF_ERR_DIR_DIRECTORY_FULL;
}
#ifdef FF_LFN_SUPPORT
#ifdef FF_UNICODE_SUPPORT
static FF_T_SINT8 FF_CreateLFNEntry(FF_T_UINT8 *EntryBuffer, FF_T_WCHAR *Name, FF_T_UINT8 NameLen, FF_T_UINT8 nLFN, FF_T_UINT8 CheckSum) {
#else
static FF_T_SINT8 FF_CreateLFNEntry(FF_T_UINT8 *EntryBuffer, FF_T_INT8 *Name, FF_T_UINT8 NameLen, FF_T_UINT8 nLFN, FF_T_UINT8 CheckSum) {
#endif
	
	FF_T_UINT8 i, x;
	
	memset(EntryBuffer, 0, 32);

	FF_putChar(EntryBuffer, FF_FAT_LFN_ORD,			(FF_T_UINT8) ((nLFN & ~0x40)));
	FF_putChar(EntryBuffer, FF_FAT_DIRENT_ATTRIB,	(FF_T_UINT8) FF_FAT_ATTR_LFN);
	FF_putChar(EntryBuffer, FF_FAT_LFN_CHECKSUM,	(FF_T_UINT8) CheckSum);

#ifdef FF_UNICODE_SUPPORT
	// Name_1
	for(i = 0, x = 0; i < 5; i++, x += sizeof(FF_T_WCHAR)) {
		if(i < NameLen) {
			*((FF_T_WCHAR *) &EntryBuffer[FF_FAT_LFN_NAME_1 + x]) = Name[i];
		} else if (i == NameLen) {
			EntryBuffer[FF_FAT_LFN_NAME_1 + x]		= '\0';
			EntryBuffer[FF_FAT_LFN_NAME_1 + x + 1]	= '\0';
		}else {
			EntryBuffer[FF_FAT_LFN_NAME_1 + x]		= 0xFF;
			EntryBuffer[FF_FAT_LFN_NAME_1 + x + 1]	= 0xFF;
		}
	}

	// Name_2
	for(i = 0, x = 0; i < 6; i++, x += 2) {
		if((i + 5) < NameLen) {
			//EntryBuffer[FF_FAT_LFN_NAME_2 + x] = Name[i+5];
			*((FF_T_WCHAR *) &EntryBuffer[FF_FAT_LFN_NAME_2 + x]) = Name[i+5];
		} else if ((i + 5) == NameLen) {
			EntryBuffer[FF_FAT_LFN_NAME_2 + x]		= '\0';
			EntryBuffer[FF_FAT_LFN_NAME_2 + x + 1]	= '\0';
		}else {
			EntryBuffer[FF_FAT_LFN_NAME_2 + x]		= 0xFF;
			EntryBuffer[FF_FAT_LFN_NAME_2 + x + 1]	= 0xFF;
		}
	}

	// Name_3
	for(i = 0, x = 0; i < 2; i++, x += 2) {
		if((i + 11) < NameLen) {
			*((FF_T_WCHAR *) &EntryBuffer[FF_FAT_LFN_NAME_3 + x]) = Name[i+11];
			//EntryBuffer[FF_FAT_LFN_NAME_3 + x] = Name[i+11];
		} else if ((i + 11) == NameLen) {
			EntryBuffer[FF_FAT_LFN_NAME_3 + x]		= '\0';
			EntryBuffer[FF_FAT_LFN_NAME_3 + x + 1]	= '\0';
		}else {
			EntryBuffer[FF_FAT_LFN_NAME_3 + x]		= 0xFF;
			EntryBuffer[FF_FAT_LFN_NAME_3 + x + 1]	= 0xFF;
		}
	}

#else

	// Name_1
	for(i = 0, x = 0; i < 5; i++, x += 2) {
		if(i < NameLen) {
			EntryBuffer[FF_FAT_LFN_NAME_1 + x] = Name[i];
		} else if (i == NameLen) {
			EntryBuffer[FF_FAT_LFN_NAME_1 + x]		= '\0';
			EntryBuffer[FF_FAT_LFN_NAME_1 + x + 1]	= '\0';
		}else {
			EntryBuffer[FF_FAT_LFN_NAME_1 + x]		= 0xFF;
			EntryBuffer[FF_FAT_LFN_NAME_1 + x + 1]	= 0xFF;
		}
	}

	// Name_2
	for(i = 0, x = 0; i < 6; i++, x += 2) {
		if((i + 5) < NameLen) {
			EntryBuffer[FF_FAT_LFN_NAME_2 + x] = Name[i+5];
		} else if ((i + 5) == NameLen) {
			EntryBuffer[FF_FAT_LFN_NAME_2 + x] = '\0';
		}else {
			EntryBuffer[FF_FAT_LFN_NAME_2 + x]		= 0xFF;
			EntryBuffer[FF_FAT_LFN_NAME_2 + x + 1]	= 0xFF;
		}
	}

	// Name_3
	for(i = 0, x = 0; i < 2; i++, x += 2) {
		if((i + 11) < NameLen) {
			EntryBuffer[FF_FAT_LFN_NAME_3 + x] = Name[i+11];
		} else if ((i + 11) == NameLen) {
			EntryBuffer[FF_FAT_LFN_NAME_3 + x] = '\0';
		}else {
			EntryBuffer[FF_FAT_LFN_NAME_3 + x]		= 0xFF;
			EntryBuffer[FF_FAT_LFN_NAME_3 + x + 1]	= 0xFF;
		}
	}
#endif
	
	return FF_ERR_NONE;
}

#ifdef FF_UNICODE_SUPPORT
static FF_ERROR FF_CreateLFNs(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_T_WCHAR *Name, FF_T_UINT8 CheckSum, FF_T_UINT16 nEntry) {
#else
static FF_ERROR FF_CreateLFNs(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_T_INT8 *Name, FF_T_UINT8 CheckSum, FF_T_UINT16 nEntry) {
#endif
	FF_T_UINT8	EntryBuffer[32];
#ifdef FF_UNICODE_SUPPORT
	FF_T_UINT16 NameLen = (FF_T_UINT16) wcslen(Name);
#else
	FF_T_UINT16 NameLen = (FF_T_UINT16) strlen(Name);
#endif
	FF_T_UINT8	NumLFNs = (FF_T_UINT8)	(NameLen / 13);
	FF_T_UINT8	i;
	FF_T_UINT8	EndPos = (NameLen % 13);
	FF_ERROR	Error;

	FF_FETCH_CONTEXT FetchContext;

	if(EndPos) {
		NumLFNs ++;
	} else {
		EndPos = 13;
	}

	Error = FF_InitEntryFetch(pIoman, DirCluster, &FetchContext);
	if(Error) {
		return Error;
	}

	for(i = NumLFNs; i > 0; i--) {
		if(i == NumLFNs) {
			FF_CreateLFNEntry(EntryBuffer, (Name + (13 * (i - 1))), EndPos, i, CheckSum);
			EntryBuffer[0] |= 0x40;
		} else {
			FF_CreateLFNEntry(EntryBuffer, (Name + (13 * (i - 1))), 13, i, CheckSum);
		}
		
		Error = FF_PushEntryWithContext(pIoman, nEntry + (NumLFNs - i), &FetchContext, EntryBuffer);
		if(Error) {
			FF_CleanupEntryFetch(pIoman, &FetchContext);
			return Error;
		}
	}

	FF_CleanupEntryFetch(pIoman, &FetchContext);

	return FF_ERR_NONE;
}
#endif

FF_ERROR FF_ExtendDirectory(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster) {
	FF_T_UINT32 CurrentCluster;
	FF_T_UINT32 NextCluster;
	FF_ERROR Error;

	if(pIoman->pPartition->Type != FF_T_FAT32) {
		if(DirCluster == pIoman->pPartition->RootDirCluster) {
			return FF_ERR_DIR_CANT_EXTEND_ROOT_DIR;
		}
	}

	if(!pIoman->pPartition->FreeClusterCount) {
		pIoman->pPartition->FreeClusterCount = FF_CountFreeClusters(pIoman, &Error);
		if(Error) {
			return Error;
		}
		if(pIoman->pPartition->FreeClusterCount == 0) {
			return FF_ERR_FAT_NO_FREE_CLUSTERS;
		}
	}
	
	FF_lockFAT(pIoman);
	{
		CurrentCluster = FF_FindEndOfChain(pIoman, DirCluster, &Error);
		if(Error) {
			FF_unlockFAT(pIoman);
			return Error;
		}

		NextCluster = FF_FindFreeCluster(pIoman, &Error);
		if(Error) {
			FF_unlockFAT(pIoman);
			return Error;
		}

		Error = FF_putFatEntry(pIoman, CurrentCluster, NextCluster);
		if(Error) {
			FF_unlockFAT(pIoman);
			return Error;
		}

		Error = FF_putFatEntry(pIoman, NextCluster, 0xFFFFFFFF);
		if(Error) {
			FF_unlockFAT(pIoman);
			return Error;
		}
	}
	FF_unlockFAT(pIoman);

	Error = FF_ClearCluster(pIoman, NextCluster);
	if(Error) {
		FF_unlockFAT(pIoman);
		return Error;
	}
	
	Error = FF_DecreaseFreeClusters(pIoman, 1);
	if(Error) {
		FF_unlockFAT(pIoman);
		return Error;
	}

	return FF_ERR_NONE;
}

#ifdef FF_UNICODE_SUPPORT
static void FF_MakeNameCompliant(FF_T_WCHAR *Name) {
#else
static void FF_MakeNameCompliant(FF_T_INT8 *Name) {
#endif
	
	if((FF_T_UINT8) Name[0] == 0xE5) {	// Support Japanese KANJI symbol.
		Name[0] = 0x05;
	}
	
	while(*Name) {
		if(*Name < 0x20 || *Name == 0x7F || *Name == 0x22 || *Name == 0x7C) {	// Leave all extended chars as they are.
			*Name = '_';
		}
		if(*Name >= 0x2A && *Name <= 0x2F && *Name != 0x2B && *Name != 0x2E && *Name != 0x2D) {
			*Name = '_';
		}
		if(*Name >= 0x3A && *Name <= 0x3F) {
			*Name = '_';
		}
		if(*Name >= 0x5B && *Name <= 0x5C) {
			*Name = '_';
		}
		Name++;
	}
}

FF_ERROR FF_CreateDirent(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_DIRENT *pDirent) {
	
	FF_T_UINT8	EntryBuffer[32];
#ifdef FF_UNICODE_SUPPORT
	FF_T_UINT16	NameLen = (FF_T_UINT16) wcslen(pDirent->FileName);
#else
	FF_T_UINT16	NameLen = (FF_T_UINT16) strlen(pDirent->FileName);
#endif
	FF_T_UINT8	numLFNs = (FF_T_UINT8) (NameLen / 13);
	FF_T_SINT32	FreeEntry;
	FF_ERROR	RetVal = FF_ERR_NONE;
	FF_T_UINT8	Entries;

	FF_FETCH_CONTEXT FetchContext;

#ifdef FF_LFN_SUPPORT
	FF_T_UINT8	CheckSum;
#endif

#ifdef FF_UNICODE_SUPPORT
	FF_T_WCHAR	UTF16EntryBuffer[32];
#if WCHAR_MAX > 0xFFFF
	// Check that the filename won't exceed the max LFN length if converted to UTF-16.
	if(FF_Utf32GetUtf16Len((FF_T_UINT32 *) pDirent->FileName) > FF_MAX_FILENAME) {
		return FF_ERR_UNICODE_CONVERSION_EXCEEDED; 
	}
#endif

#endif

	FF_MakeNameCompliant(pDirent->FileName);	// Ensure we don't break the Dir tables.
	memset(EntryBuffer, 0, 32);

	if(NameLen % 13) {
		numLFNs ++;
	}

#ifdef FF_LFN_SUPPORT
	// Create and push the LFN's
	Entries = numLFNs + 1;	// Find enough places for the LFNs and the ShortName	
#else
	Entries = 1;
#endif

	// Create the ShortName
	FF_lockDIR(pIoman);
	{
		if((FreeEntry = FF_FindFreeDirent(pIoman, DirCluster, Entries)) >= 0) {
#ifdef FF_UNICODE_SUPPORT
			FF_cstrntowcs(UTF16EntryBuffer, (FF_T_INT8 *) EntryBuffer, 32);
			RetVal = FF_CreateShortName(pIoman, DirCluster, UTF16EntryBuffer, pDirent->FileName);
#else
			RetVal = FF_CreateShortName(pIoman, DirCluster, (FF_T_INT8 *) EntryBuffer, pDirent->FileName);
#endif
			
			if(!RetVal) {
#ifdef FF_LFN_SUPPORT
				CheckSum = FF_CreateChkSum(EntryBuffer);
				FF_CreateLFNs(pIoman, DirCluster, pDirent->FileName, CheckSum, (FF_T_UINT16) FreeEntry);
#else
				numLFNs = 0;
#endif				
				
#ifdef FF_TIME_SUPPORT
				FF_PlaceTime(EntryBuffer, FF_FAT_DIRENT_CREATE_TIME);
				FF_PlaceDate(EntryBuffer, FF_FAT_DIRENT_CREATE_DATE);
				FF_PlaceTime(EntryBuffer, FF_FAT_DIRENT_LASTMOD_TIME);
				FF_PlaceDate(EntryBuffer, FF_FAT_DIRENT_LASTMOD_DATE);
#endif

				FF_putChar(EntryBuffer,  (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB), pDirent->Attrib);
				FF_putShort(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_HIGH), (FF_T_UINT16)(pDirent->ObjectCluster >> 16));
				FF_putShort(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_LOW), (FF_T_UINT16)(pDirent->ObjectCluster));
				FF_putLong(EntryBuffer,  (FF_T_UINT16)(FF_FAT_DIRENT_FILESIZE), pDirent->Filesize);

				RetVal = FF_InitEntryFetch(pIoman, DirCluster, &FetchContext);
				if(RetVal) {
					FF_unlockDIR(pIoman);
					return RetVal;
				}
				RetVal = FF_PushEntryWithContext(pIoman, (FF_T_UINT16) (FreeEntry + numLFNs), &FetchContext, EntryBuffer);
				FF_CleanupEntryFetch(pIoman, &FetchContext);
				if(RetVal) {
					FF_unlockDIR(pIoman);
					return RetVal;
				}
			} else {
				FF_unlockDIR(pIoman);
				return RetVal;
			}
		}else {
			FF_unlockDIR(pIoman);
			return FreeEntry;
		}
	}
	FF_unlockDIR(pIoman);

	if(RetVal) {
		return RetVal;
	}

	if(pDirent) {
		pDirent->CurrentItem = (FF_T_UINT16) (FreeEntry + numLFNs);
	}
	
	return FF_ERR_NONE;
}


#ifdef FF_UNICODE_SUPPORT
FF_T_UINT32 FF_CreateFile(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_T_WCHAR *FileName, FF_DIRENT *pDirent, FF_ERROR *pError) {
#else
FF_T_UINT32 FF_CreateFile(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_T_INT8 *FileName, FF_DIRENT *pDirent, FF_ERROR *pError) {
#endif
	FF_DIRENT	MyFile;
	*pError	= FF_ERR_NONE;
#ifdef FF_UNICODE_SUPPORT
	wcsncpy(MyFile.FileName, FileName, FF_MAX_FILENAME);
#else
	strncpy(MyFile.FileName, FileName, FF_MAX_FILENAME);
#endif

	MyFile.Attrib = 0x00;
	MyFile.Filesize = 0;
	MyFile.ObjectCluster = FF_CreateClusterChain(pIoman, pError);
	if(*pError) {
		FF_UnlinkClusterChain(pIoman, MyFile.ObjectCluster, 0);
		FF_FlushCache(pIoman);
		return 0;
	}
	MyFile.CurrentItem = 0;

	*pError = FF_CreateDirent(pIoman, DirCluster, &MyFile);

	if(*pError) {
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


/**
 *	@brief Creates a Directory of the specified path.
 *
 *	@param	pIoman	Pointer to the FF_IOMAN object.
 *	@param	Path	Path of the directory to create.
 *
 *	@return	FF_ERR_NULL_POINTER if pIoman was NULL.
 *	@return FF_ERR_DIR_OBJECT_EXISTS if the object specified by path already exists.
 *	@return	FF_ERR_DIR_INVALID_PATH
 *	@return FF_ERR_NONE on success.
 **/
#ifdef FF_UNICODE_SUPPORT
FF_ERROR FF_MkDir(FF_IOMAN *pIoman, const FF_T_WCHAR *Path) {
#else
FF_ERROR FF_MkDir(FF_IOMAN *pIoman, const FF_T_INT8 *Path) {
#endif
	FF_DIRENT	MyDir;
	FF_T_UINT32 DirCluster;
#ifdef FF_UNICODE_SUPPORT
	FF_T_WCHAR	DirName[FF_MAX_FILENAME];
#else
	FF_T_INT8	DirName[FF_MAX_FILENAME];
#endif
	FF_T_UINT8	EntryBuffer[32];
	FF_T_UINT32 DotDotCluster;
	FF_T_UINT16	i;
	FF_ERROR	Error = FF_ERR_NONE;

	FF_FETCH_CONTEXT FetchContext;

	if(!pIoman) {
		return FF_ERR_NULL_POINTER;
	}

#ifdef FF_UNICODE_SUPPORT
	i = (FF_T_UINT16) wcslen(Path);
#else
	i = (FF_T_UINT16) strlen(Path);
#endif

	while(i != 0) {
		if(Path[i] == '\\' || Path[i] == '/') {
			break;
		}
		i--;
	}

#ifdef FF_UNICODE_SUPPORT
	wcsncpy(DirName, (Path + i + 1), FF_MAX_FILENAME);
#else
	strncpy(DirName, (Path + i + 1), FF_MAX_FILENAME);
#endif

	if(i == 0) {
		i = 1;
	}

	DirCluster = FF_FindDir(pIoman, Path, i, &Error);

	if(Error) {
		return Error;
	}

	if(DirCluster) {
		if(FF_FindEntryInDir(pIoman, DirCluster, DirName, 0x00, &MyDir, &Error)) {
			return FF_ERR_DIR_OBJECT_EXISTS;
		}

		if(Error && Error != FF_ERR_DIR_END_OF_DIR) {
			return Error;	
		}

#ifdef FF_UNICODE_SUPPORT
		wcsncpy(MyDir.FileName, DirName, FF_MAX_FILENAME);
#else
		strncpy(MyDir.FileName, DirName, FF_MAX_FILENAME);
#endif
		MyDir.Filesize		= 0;
		MyDir.Attrib		= FF_FAT_ATTR_DIR;
		MyDir.ObjectCluster	= FF_CreateClusterChain(pIoman, &Error);
		if(Error) {
			return Error;
		}
		if(MyDir.ObjectCluster) {
			Error = FF_ClearCluster(pIoman, MyDir.ObjectCluster);
			if(Error) {
				FF_UnlinkClusterChain(pIoman, MyDir.ObjectCluster, 0);
				FF_FlushCache(pIoman);
				return Error;
			}
		} else {
			// Couldn't allocate any space for the dir!
			return FF_ERR_DIR_EXTEND_FAILED;
		}

		Error = FF_CreateDirent(pIoman, DirCluster, &MyDir);

		if(Error) {
			FF_UnlinkClusterChain(pIoman, MyDir.ObjectCluster, 0);
			FF_FlushCache(pIoman);
			return Error;
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

		Error = FF_InitEntryFetch(pIoman, MyDir.ObjectCluster, &FetchContext);
		if(Error) {
			FF_UnlinkClusterChain(pIoman, MyDir.ObjectCluster, 0);
			FF_FlushCache(pIoman);
			return Error;
		}
		
		Error = FF_PushEntryWithContext(pIoman, 0, &FetchContext, EntryBuffer);
		if(Error) {
			FF_UnlinkClusterChain(pIoman, MyDir.ObjectCluster, 0);
			FF_FlushCache(pIoman);
			FF_CleanupEntryFetch(pIoman, &FetchContext);
			return Error;
		}

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
		
		//FF_PushEntry(pIoman, MyDir.ObjectCluster, 1, EntryBuffer);
		Error = FF_PushEntryWithContext(pIoman, 1, &FetchContext, EntryBuffer);
		if(Error) {
			FF_UnlinkClusterChain(pIoman, MyDir.ObjectCluster, 0);
			FF_FlushCache(pIoman);
			FF_CleanupEntryFetch(pIoman, &FetchContext);
			return Error;
		}
		FF_CleanupEntryFetch(pIoman, &FetchContext);

		FF_FlushCache(pIoman);	// Ensure dir was flushed to the disk!

		return FF_ERR_NONE;
	}
	
	return FF_ERR_DIR_INVALID_PATH;
}



FF_ERROR FF_RmLFNs(FF_IOMAN *pIoman, FF_T_UINT16 usDirEntry, FF_FETCH_CONTEXT *pContext) {

	FF_ERROR	Error;
	FF_T_UINT8	EntryBuffer[32];

	usDirEntry--;

	do {
		Error = FF_FetchEntryWithContext(pIoman, usDirEntry, pContext, EntryBuffer);
		if(Error) {
			return Error;
		}
		
		if(FF_getChar(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB)) == FF_FAT_ATTR_LFN) {
			FF_putChar(EntryBuffer, (FF_T_UINT16) 0, (FF_T_UINT8) 0xE5);
			Error = FF_PushEntryWithContext(pIoman, usDirEntry, pContext, EntryBuffer);
			if(Error) {
				return Error;
			}
		}

		if(usDirEntry == 0) {
			break;
		}
		usDirEntry--;
	}while(FF_getChar(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB)) == FF_FAT_ATTR_LFN);

	return FF_ERR_NONE;
}
