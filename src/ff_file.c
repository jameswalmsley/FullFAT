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
 *	@file		ff_file.c
 *	@author		James Walmsley
 *	@ingroup	FILEIO
 *
 *	@defgroup	FILEIO FILE I/O Access
 *	@brief		Provides an interface to allow File I/O on a mounted IOMAN.
 *
 *	Provides file-system interfaces for the FAT file-system.
 **/

#include "ff_file.h"
#include "ff_string.h"
#include "ff_config.h"

#ifdef FF_UNICODE_SUPPORT
#include <wchar.h>
#endif

/**
 *	@public
 *	@brief	Converts STDIO mode strings into the equivalent FullFAT mode.
 *
 *	@param	Mode	The mode string e.g. "rb" "rb+" "w" "a" "r" "w+" "a+" etc
 *
 *	@return	Returns the mode bits that should be passed to the FF_Open function.
 **/
FF_T_UINT8 FF_GetModeBits(FF_T_INT8 *Mode) {
	FF_T_UINT8 ModeBits = 0x00;
	while(*Mode) {
		switch(*Mode) {
			case 'r':	// Allow Read
			case 'R':
				ModeBits |= FF_MODE_READ;
				break;

			case 'w':	// Allow Write
			case 'W':
				ModeBits |= FF_MODE_WRITE;
				ModeBits |= FF_MODE_CREATE;	// Create if not exist.
				ModeBits |= FF_MODE_TRUNCATE;
				break;

			case 'a':	// Append new writes to the end of the file.
			case 'A':
				ModeBits |= FF_MODE_WRITE;
				ModeBits |= FF_MODE_APPEND;
				ModeBits |= FF_MODE_CREATE;	// Create if not exist.
				break;

			case '+':	// Update the file, don't Append!
				ModeBits |= FF_MODE_READ;	// RW Mode
				ModeBits |= FF_MODE_WRITE;	// RW Mode
				break;

			/*case 'D':	// Internal use only!
				ModeBits |= FF_MODE_DIR;
				break;*/

			default:	// b|B flags not supported (Binary mode is native anyway).
				break;
		}
		Mode++;
	}

	return ModeBits;
}

/**
 * FF_Open() Mode Information
 * - FF_MODE_WRITE
 *   - Allows WRITE access to the file.
 *   .
 * - FF_MODE_READ
 *   - Allows READ access to the file.
 *   .
 * - FF_MODE_CREATE
 *   - Create file if it doesn't exist.
 *   .
 * - FF_MODE_TRUNCATE
 *   - Erase the file if it already exists and overwrite.
 *   *
 * - FF_MODE_APPEND
 *   - Causes all writes to occur at the end of the file. (Its impossible to overwrite other data in a file with this flag set).
 *   . 
 * .
 *
 * Some sample modes:
 * - (FF_MODE_WRITE | FF_MODE_CREATE | FF_MODE_TRUNCATE)
 *   - Write access to the file. (Equivalent to "w").
 *   .
 * - (FF_MODE_WRITE | FF_MODE_READ)
 *   - Read and Write access to the file. (Equivalent to "rb+").
 *   .
 * - (FF_MODE_WRITE | FF_MODE_READ | FF_MODE_APPEND | FF_MODE_CREATE)
 *   - Read and Write append mode access to the file. (Equivalent to "a+").
 *   .
 * .
 * Be careful when choosing modes. For those using FF_Open() at the application layer
 * its best to use the provided FF_GetModeBits() function, as this complies to the same
 * behaviour as the stdio.h fopen() function.
 *
 **/


/**
 *	@public
 *	@brief	Opens a File for Access
 *
 *	@param	pIoman		FF_IOMAN object that was created by FF_CreateIOMAN().
 *	@param	path		Path to the File or object.
 *	@param	Mode		Access Mode required. Modes are a little complicated, the function FF_GetModeBits()
 *	@param	Mode		will convert a stdio Mode string into the equivalent Mode bits for this parameter.
 *	@param	pError		Pointer to a signed byte for error checking. Can be NULL if not required.
 *	@param	pError		To be checked when a NULL pointer is returned.
 *
 *	@return	NULL pointer on Error, in which case pError should be checked for more information.
 *	@return	pError can be:
 **/
#ifdef FF_UNICODE_SUPPORT
FF_FILE *FF_Open(FF_IOMAN *pIoman, const FF_T_WCHAR *path, FF_T_UINT8 Mode, FF_ERROR *pError) {
#else
FF_FILE *FF_Open(FF_IOMAN *pIoman, const FF_T_INT8 *path, FF_T_UINT8 Mode, FF_ERROR *pError) {
#endif
	FF_FILE		*pFile;
	FF_FILE		*pFileChain;
	FF_DIRENT	Object;
	FF_T_UINT32 DirCluster, FileCluster;
	FF_T_UINT32	nBytesPerCluster;
#ifdef FF_UNICODE_SUPPORT
	FF_T_WCHAR	filename[FF_MAX_FILENAME];
#else
	FF_T_INT8	filename[FF_MAX_FILENAME];
#endif
	FF_ERROR	Error;

	FF_T_UINT16	i;

	if(pError) {
		*pError = FF_ERR_NONE;
	}
	
	if(!pIoman) {
		if(pError) {
			*pError = (FF_ERR_NULL_POINTER | FF_OPEN);
		}
		return (FF_FILE *)NULL;
	}
	pFile = FF_MALLOC(sizeof(FF_FILE));
	if(!pFile) {
		if(pError) {
			*pError = (FF_ERR_NOT_ENOUGH_MEMORY | FF_OPEN);
		}
		return (FF_FILE *)NULL;
	}

	memset (pFile, 0, sizeof *pFile);
	// Get the Mode Bits.
	pFile->Mode = Mode;

#ifdef FF_UNICODE_SUPPORT
	i = (FF_T_UINT16) wcslen(path);
#else 
	i = (FF_T_UINT16) strlen(path);
#endif

	while(i != 0) {
		if(path[i] == '\\' || path[i] == '/') {
			break;
		}
		i--;
	}
#ifdef FF_UNICODE_SUPPORT
	wcsncpy(filename, (path + i + 1), FF_MAX_FILENAME);
#else
	strncpy(filename, (path + i + 1), FF_MAX_FILENAME);
#endif

	if(i == 0) {
		i = 1;
	}
	

	DirCluster = FF_FindDir(pIoman, path, i, &Error);
	if(Error) {
		if(pError) {
			*pError = Error;
		}
		FF_FREE(pFile);
		return (FF_FILE *) NULL;
	}

	if(DirCluster) {

		FileCluster = FF_FindEntryInDir(pIoman, DirCluster, filename, 0x00, &Object, &Error);
		if(Error) {
			if(pError) {
				*pError = Error;	
			}
			FF_FREE(pFile);
			return (FF_FILE *) NULL;
		}

		if(!FileCluster) {	// If 0 was returned, it might be because the file has no allocated cluster
#ifdef FF_UNICODE_SUPPORT
			if(wcslen(filename) == wcslen(Object.FileName)) {
				if(Object.Filesize == 0 && FF_strmatch(filename, Object.FileName, (FF_T_UINT16) wcslen(filename)) == FF_TRUE) {
#else
			if(strlen(filename) == strlen(Object.FileName)) {
				if(Object.Filesize == 0 && FF_strmatch(filename, Object.FileName, (FF_T_UINT16) strlen(filename)) == FF_TRUE) {
#endif
					// The file really was found!
					FileCluster = 1;
				} 
			}
		}

		if(!FileCluster) {
			if((pFile->Mode & FF_MODE_CREATE)) {
				FileCluster = FF_CreateFile(pIoman, DirCluster, filename, &Object, &Error);
				if(Error) {
					if(pError) {
						*pError = Error;
					}
					FF_FREE(pFile);
					return (FF_FILE *) NULL;
				}
				Object.CurrentItem += 1;
			}
		}
		
		if(FileCluster) {
			if(Object.Attrib == FF_FAT_ATTR_DIR) {
				if(!(pFile->Mode & FF_MODE_DIR)) {
					// Not the object, File Not Found!
					FF_FREE(pFile);
					if(pError) {
						*pError = (FF_ERR_FILE_OBJECT_IS_A_DIR | FF_OPEN);
					}
					return (FF_FILE *) NULL;
				}
			}
			
			//---------- Ensure Read-Only files don't get opened for Writing.
			if((pFile->Mode & FF_MODE_WRITE) || (pFile->Mode & FF_MODE_APPEND)) {
				if((Object.Attrib & FF_FAT_ATTR_READONLY)) {
					FF_FREE(pFile);
					if(pError) {
						*pError = (FF_ERR_FILE_IS_READ_ONLY | FF_OPEN);
					}
					return (FF_FILE *) NULL;
				}
			}
			pFile->pIoman				= pIoman;
			pFile->FilePointer			= 0;
			pFile->ObjectCluster		= Object.ObjectCluster;
			pFile->Filesize				= Object.Filesize;
			pFile->CurrentCluster		= 0;
			pFile->AddrCurrentCluster	= pFile->ObjectCluster;
			//pFile->Mode					= Mode;
			pFile->Next					= NULL;
			pFile->DirCluster			= DirCluster;
			pFile->DirEntry				= Object.CurrentItem - 1;
			nBytesPerCluster			= pFile->pIoman->pPartition->SectorsPerCluster / pIoman->BlkSize;
			pFile->iChainLength			= 0;
			pFile->iEndOfChain			= 0;
			pFile->ValidFlags			&= ~(FF_VALID_FLAG_DELETED); //FF_FALSE;

			// File Permission Processing
			// Only "w" and "w+" mode strings can erase a file's contents.
			// Any other combinations will not cause an erase.
			if((pFile->Mode & FF_MODE_TRUNCATE)) {
				pFile->Filesize = 0;
				pFile->FilePointer = 0;
			}

			/*
				Add pFile onto the end of our linked list of FF_FILE objects.
			*/
			FF_PendSemaphore(pIoman->pSemaphore);
			{
				if(!pIoman->FirstFile) {
					pIoman->FirstFile = pFile;
				} else {
					pFileChain = (FF_FILE *) pIoman->FirstFile;
					do {
						if(pFileChain->ObjectCluster == pFile->ObjectCluster) {
							// HT: Only fail if any of them has write access...
							// Why not have 2 open read handles to a single file?
							if ((pFileChain->Mode | pFile->Mode) & (FF_MODE_WRITE | FF_MODE_APPEND)) {
								// File is already open! DON'T ALLOW IT!
								FF_ReleaseSemaphore(pIoman->pSemaphore);
								FF_FREE(pFile);
								if(pError) {
									*pError = (FF_ERR_FILE_ALREADY_OPEN | FF_OPEN);
								}
								return (FF_FILE *) NULL;
							}
						}
						if(!pFileChain->Next) {
							pFileChain->Next = pFile;
							break;
						}
						pFileChain = (FF_FILE *) pFileChain->Next;
					}while(pFileChain != NULL);
				}
			}
			FF_ReleaseSemaphore(pIoman->pSemaphore);

			return pFile;
		}else {
			FF_FREE(pFile);
			if(pError) {
				*pError = (FF_ERR_FILE_NOT_FOUND | FF_OPEN);
			}
			return (FF_FILE *) NULL;
		} 
	}
	if(pError) {
		*pError = (FF_ERR_FILE_INVALID_PATH | FF_OPEN);
	}

	FF_FREE(pFile);

	return (FF_FILE *)NULL;
}


/**
 *	@public
 *	@brief	Tests if a Directory contains any other files or folders.
 *
 *	@param	pIoman	FF_IOMAN object returned from the FF_CreateIOMAN() function.
 *
 **/
#ifdef FF_UNICODE_SUPPORT
FF_T_BOOL FF_isDirEmpty(FF_IOMAN *pIoman, const FF_T_WCHAR *Path) {
#else
FF_T_BOOL FF_isDirEmpty(FF_IOMAN *pIoman, const FF_T_INT8 *Path) {
#endif
	
	FF_DIRENT	MyDir;
	FF_ERROR	RetVal = FF_ERR_NONE;
	FF_T_UINT8	i = 0;

	if(!pIoman) {
		return FF_FALSE;
	}
	
	RetVal = FF_FindFirst(pIoman, &MyDir, Path);
	while(RetVal == 0) {
		i++;
		RetVal = FF_FindNext(pIoman, &MyDir);
		if(i > 2) {
			return FF_FALSE;
		}
	}

	return FF_TRUE;
}

#ifdef FF_UNICODE_SUPPORT
FF_ERROR FF_RmDir(FF_IOMAN *pIoman, const FF_T_WCHAR *path) {
#else
FF_ERROR FF_RmDir(FF_IOMAN *pIoman, const FF_T_INT8 *path) {
#endif
	FF_FILE 			*pFile;
	FF_ERROR 			Error = FF_ERR_NONE;
	FF_T_UINT8 			EntryBuffer[32];
	FF_FETCH_CONTEXT	FetchContext;
	FF_ERROR 			RetVal = FF_ERR_NONE;
#ifdef FF_PATH_CACHE
	FF_T_UINT32 i;
#endif

	if(!pIoman) {
		return (FF_ERR_NULL_POINTER | FF_RMDIR);
	}

	pFile = FF_Open(pIoman, path, FF_MODE_DIR, &Error);

	if(!pFile) {
		return Error;	// File in use or File not found!
	}

	pFile->ValidFlags |= FF_VALID_FLAG_DELETED;//FF_TRUE;
	
	FF_lockDIR(pIoman);
	{
		if(FF_isDirEmpty(pIoman, path)) {
			FF_lockFAT(pIoman);
			{
				Error = FF_UnlinkClusterChain(pIoman, pFile->ObjectCluster, 0);	// 0 to delete the entire chain!
			}
			FF_unlockFAT(pIoman);

			if(Error) {
				FF_unlockDIR(pIoman);
				FF_Close(pFile);
				return Error;				
			}

			// Initialise the dirent Fetch Context object for faster removal of dirents.

			Error = FF_InitEntryFetch(pIoman, pFile->DirCluster, &FetchContext);
			if(Error) {
				FF_unlockDIR(pIoman);
				FF_Close(pFile);
				return Error;
			}
			
			// Edit the Directory Entry! (So it appears as deleted);
			Error = FF_RmLFNs(pIoman, pFile->DirEntry, &FetchContext);
			if(Error) {
				FF_CleanupEntryFetch(pIoman, &FetchContext);
				FF_unlockDIR(pIoman);
				FF_Close(pFile);
				return Error;
			}
			Error = FF_FetchEntryWithContext(pIoman, pFile->DirEntry, &FetchContext, EntryBuffer);
			if(Error) {
				FF_CleanupEntryFetch(pIoman, &FetchContext);
				FF_unlockDIR(pIoman);
				FF_Close(pFile);
				return Error;
			}
			EntryBuffer[0] = 0xE5;
			Error = FF_PushEntryWithContext(pIoman, pFile->DirEntry, &FetchContext, EntryBuffer);
			if(Error) {
				FF_CleanupEntryFetch(pIoman, &FetchContext);
				FF_unlockDIR(pIoman);
				FF_Close(pFile);
				return Error;
			}
#ifdef FF_PATH_CACHE
			FF_PendSemaphore(pIoman->pSemaphore);	// Thread safety on shared object!
			{
				for(i = 0; i < FF_PATH_CACHE_DEPTH; i++) {
#ifdef FF_UNICODE_SUPPORT
					if(FF_strmatch(pIoman->pPartition->PathCache[i].Path, path, (FF_T_UINT16)wcslen(path))) {
#else
					if(FF_strmatch(pIoman->pPartition->PathCache[i].Path, path, (FF_T_UINT16)strlen(path))) {
#endif
						pIoman->pPartition->PathCache[i].Path[0] = '\0';
						pIoman->pPartition->PathCache[i].DirCluster = 0;
						FF_ReleaseSemaphore(pIoman->pSemaphore);
					}
				}
			}
			FF_ReleaseSemaphore(pIoman->pSemaphore);
#endif
			
			Error = FF_IncreaseFreeClusters(pIoman, pFile->iChainLength);
			if(Error) {
				FF_CleanupEntryFetch(pIoman, &FetchContext);
				FF_unlockDIR(pIoman);
				FF_Close(pFile);
				return Error;
			}

			FF_CleanupEntryFetch(pIoman, &FetchContext);

			Error = FF_FlushCache(pIoman);
			if(Error) {
				FF_unlockDIR(pIoman);
				FF_Close(pFile);
				return Error;
			}
		} else {
			RetVal = (FF_ERR_DIR_NOT_EMPTY | FF_RMDIR);
		}
	}
	FF_unlockDIR(pIoman);
	Error = FF_Close(pFile); // Free the file pointer resources

	if(Error) {
		return Error;
	}

	// File is now lost!
	return RetVal;
}

#ifdef FF_UNICODE_SUPPORT
FF_ERROR FF_RmFile(FF_IOMAN *pIoman, const FF_T_WCHAR *path) {
#else
FF_ERROR FF_RmFile(FF_IOMAN *pIoman, const FF_T_INT8 *path) {
#endif
	FF_FILE *pFile;
	FF_ERROR Error = FF_ERR_NONE;
	FF_T_UINT8 EntryBuffer[32];
	FF_FETCH_CONTEXT FetchContext;

	pFile = FF_Open(pIoman, path, FF_MODE_READ, &Error);

	if(!pFile) {
		return Error;	// File in use or File not found!
	}

	pFile->ValidFlags |= FF_VALID_FLAG_DELETED;//FF_TRUE;

	if(pFile->ObjectCluster) {	// Ensure there is actually a cluster chain to delete!
		FF_lockFAT(pIoman);	// Lock the FAT so its thread-safe.
		{
			Error = FF_UnlinkClusterChain(pIoman, pFile->ObjectCluster, 0);	// 0 to delete the entire chain!
		}
		FF_unlockFAT(pIoman);

		if(Error) {
			FF_Close(pFile);
			return Error;
		}
	}

	// Edit the Directory Entry! (So it appears as deleted);
	FF_lockDIR(pIoman);
	{
		Error = FF_InitEntryFetch(pIoman, pFile->DirCluster, &FetchContext);
		if(Error) {
			FF_unlockDIR(pIoman);
			FF_Close(pFile);
			return Error;
		}
		Error = FF_RmLFNs(pIoman, pFile->DirEntry, &FetchContext);
		if(Error) {
			FF_CleanupEntryFetch(pIoman, &FetchContext);
			FF_unlockDIR(pIoman);
			FF_Close(pFile);
			return Error;
		}
		Error = FF_FetchEntryWithContext(pIoman, pFile->DirEntry, &FetchContext, EntryBuffer);
		if(Error) {
			FF_CleanupEntryFetch(pIoman, &FetchContext);
			FF_unlockDIR(pIoman);
			FF_Close(pFile);
			return Error;
		}
		EntryBuffer[0] = 0xE5;
		
		Error = FF_PushEntryWithContext(pIoman, pFile->DirEntry, &FetchContext, EntryBuffer);
		if(Error) {
			FF_CleanupEntryFetch(pIoman, &FetchContext);
			FF_unlockDIR(pIoman);
			FF_Close(pFile);
			return Error;
		}

		FF_CleanupEntryFetch(pIoman, &FetchContext);
	}
	FF_unlockDIR(pIoman);

	Error = FF_FlushCache(pIoman);
	if(Error) {
		FF_Close(pFile);
		return Error;
	}
	
	Error = FF_Close(pFile); // Free the file pointer resources
	
	return Error;
}

/**
 *	@public
 *	@brief	Moves a file or directory from source to destination.
 *
 *	@param	pIoman				The FF_IOMAN object pointer.
 *	@param	szSourceFile		String of the source file to be moved or renamed.
 *	@param	szDestinationFile	String of the destination file to where the source should be moved or renamed.
 *
 *	@return	FF_ERR_NONE on success.
 *	@return FF_ERR_FILE_DESTINATION_EXISTS if the destination file exists.
 *	@return FF_ERR_FILE_COULD_NOT_CREATE_DIRENT if dirent creation failed (fatal error!).
 *	@return FF_ERR_FILE_DIR_NOT_FOUND if destination directory was not found.
 *	@return FF_ERR_FILE_SOURCE_NOT_FOUND if the source file was not found.
 *
 **/
#ifdef FF_UNICODE_SUPPORT
FF_ERROR FF_Move(FF_IOMAN *pIoman, const FF_T_WCHAR *szSourceFile, const FF_T_WCHAR *szDestinationFile) {
#else
FF_ERROR FF_Move(FF_IOMAN *pIoman, const FF_T_INT8 *szSourceFile, const FF_T_INT8 *szDestinationFile) {
#endif
	FF_ERROR	Error;
	FF_FILE		*pSrcFile, *pDestFile;
	FF_DIRENT	MyFile;
	FF_T_UINT8	EntryBuffer[32];
	FF_T_UINT16 i;
	FF_T_UINT32	DirCluster;
	FF_FETCH_CONTEXT	FetchContext;

	if(!pIoman) {
		return (FF_ERR_NULL_POINTER | FF_MOVE);
	}

	// Check destination file doesn't exist!
	pDestFile = FF_Open(pIoman, szDestinationFile, FF_MODE_READ, &Error);

	if(pDestFile || (FF_GETERROR(Error) == FF_ERR_FILE_OBJECT_IS_A_DIR)) {
		if (pDestFile) {
			FF_Close(pDestFile);
		}
		
		return (FF_ERR_FILE_DESTINATION_EXISTS | FF_MOVE);	// YES -- FAIL
	}

	pSrcFile = FF_Open(pIoman, szSourceFile, FF_MODE_READ, &Error);

	if(FF_GETERROR(Error) == FF_ERR_FILE_OBJECT_IS_A_DIR) {
		// Open a directory for moving!
		pSrcFile = FF_Open(pIoman, szSourceFile, FF_MODE_DIR, &Error);
	}

	if(!pSrcFile) {
		return Error;
	}

	// Create the new dirent.
	Error = FF_InitEntryFetch(pIoman, pSrcFile->DirCluster, &FetchContext);
	if(Error) {
		FF_Close(pSrcFile);
		return Error;
	}
	Error = FF_FetchEntryWithContext(pIoman, pSrcFile->DirEntry, &FetchContext, EntryBuffer);
	if(Error) {
		FF_Close(pSrcFile);
		FF_CleanupEntryFetch(pIoman, &FetchContext);
		return Error;
	}
	//FF_FetchEntry(pIoman, pSrcFile->DirCluster, pSrcFile->DirEntry, EntryBuffer);
	MyFile.Attrib			= FF_getChar(EntryBuffer,  (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB));
	MyFile.Filesize			= pSrcFile->Filesize;
	MyFile.ObjectCluster	= pSrcFile->ObjectCluster;
	MyFile.CurrentItem		= 0;

#ifdef FF_UNICODE_SUPPORT
	i = (FF_T_UINT16) wcslen(szDestinationFile);
#else
	i = (FF_T_UINT16) strlen(szDestinationFile);
#endif

	while(i != 0) {
		if(szDestinationFile[i] == '\\' || szDestinationFile[i] == '/') {
			break;
		}
		i--;
	}

#ifdef FF_UNICODE_SUPPORT
	wcsncpy(MyFile.FileName, (szDestinationFile + i + 1), FF_MAX_FILENAME);
#else
	strncpy(MyFile.FileName, (szDestinationFile + i + 1), FF_MAX_FILENAME);
#endif

	if(i == 0) {
		i = 1;
	}
	

	DirCluster = FF_FindDir(pIoman, szDestinationFile, i, &Error);
	if(Error) {
		FF_Close(pSrcFile);
		FF_CleanupEntryFetch(pIoman, &FetchContext);
		return Error;
	}
	
	if(DirCluster) {
		// HT: Cleaup because FF_CreateDirent might want to write the same sector
		FF_CleanupEntryFetch(pIoman, &FetchContext);
		// Destination Dir was found, we can now create the new entry.
		Error = FF_CreateDirent(pIoman, DirCluster, &MyFile);
		if(Error) {
			FF_Close(pSrcFile);
			FF_CleanupEntryFetch(pIoman, &FetchContext);
			return Error;	// FAILED
		}

		// Edit the Directory Entry! (So it appears as deleted);
		FF_lockDIR(pIoman);
		{

			Error = FF_RmLFNs(pIoman, pSrcFile->DirEntry, &FetchContext);
			if(Error) {
				FF_unlockDIR(pIoman);
				FF_Close(pSrcFile);
				FF_CleanupEntryFetch(pIoman, &FetchContext);
				return Error;
			}
			Error = FF_FetchEntryWithContext(pIoman, pSrcFile->DirEntry, &FetchContext, EntryBuffer);
			if(Error) {
				FF_unlockDIR(pIoman);
				FF_Close(pSrcFile);
				FF_CleanupEntryFetch(pIoman, &FetchContext);
				return Error;
			}
			EntryBuffer[0] = 0xE5;
			//FF_PushEntry(pIoman, pSrcFile->DirCluster, pSrcFile->DirEntry, EntryBuffer);
			Error = FF_PushEntryWithContext(pIoman, pSrcFile->DirEntry, &FetchContext, EntryBuffer);
			if(Error) {
				FF_unlockDIR(pIoman);
				FF_Close(pSrcFile);
				FF_CleanupEntryFetch(pIoman, &FetchContext);
				return Error;
			}
			FF_CleanupEntryFetch(pIoman, &FetchContext);
		}
		FF_unlockDIR(pIoman);
		FF_Close(pSrcFile);

		FF_FlushCache(pIoman);

		return FF_ERR_NONE;
	}

	return (FF_ERR_FILE_DIR_NOT_FOUND | FF_MOVE);
}


/**
 *	@public
 *	@brief	Get's the next Entry based on the data recorded in the FF_DIRENT object.
 *
 *	@param	pFile	FF_FILE object that was created by FF_Open().
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
 *	@brief	Checks the number of bytes left on a read handle
 *
 *	@param	pFile		An open file handle
 *
 *	@return	Less than zero: an error code
 *	@return	Number of bytes left to read from handle
 **/
FF_T_SINT32 FF_BytesLeft(FF_FILE *pFile) {
	if(!pFile) {
		return FF_ERR_NULL_POINTER | FF_BYTESLEFT;
	}
	if(!(pFile->Mode & FF_MODE_READ)) {
		return FF_ERR_FILE_NOT_OPENED_IN_READ_MODE | FF_BYTESLEFT;
	}

	if(pFile->FilePointer >= pFile->Filesize) {
		return 0;
	} else {
		return pFile->Filesize - pFile->FilePointer;
	}
}


static FF_T_UINT32 FF_GetSequentialClusters(FF_IOMAN *pIoman, FF_T_UINT32 StartCluster, FF_T_UINT32 Limit, FF_ERROR *pError) {
	FF_T_UINT32 CurrentCluster;
	FF_T_UINT32 NextCluster = StartCluster;
	FF_T_UINT32 i = 0;

	*pError = FF_ERR_NONE;

	do {
		CurrentCluster = NextCluster;
		NextCluster = FF_getFatEntry(pIoman, CurrentCluster, pError);
		if(*pError) {
			return 0;
		}
		if(NextCluster == (CurrentCluster + 1)) {
			i++;
		} else {
			break;
		}

		if(Limit) {
			if(i == Limit) {
				break;
			}
		}
	}while(NextCluster == (CurrentCluster + 1));

	return i;
}

static FF_ERROR FF_ReadClusters(FF_FILE *pFile, FF_T_UINT32 Count, FF_T_UINT8 *buffer) {
	FF_T_UINT32 ulSectors;
	FF_T_UINT32 SequentialClusters = 0;
	FF_T_UINT32 nItemLBA;
	FF_T_SINT32 slRetVal;
	FF_ERROR	Error;

	while(Count != 0) {
		if((Count - 1) > 0) {
			SequentialClusters = FF_GetSequentialClusters(pFile->pIoman, pFile->AddrCurrentCluster, (Count - 1), &Error);
			if(Error) {
				return Error;
			}
		}
		ulSectors = (SequentialClusters + 1) * pFile->pIoman->pPartition->SectorsPerCluster;
		nItemLBA = FF_Cluster2LBA(pFile->pIoman, pFile->AddrCurrentCluster);
		nItemLBA = FF_getRealLBA(pFile->pIoman, nItemLBA);

		slRetVal = FF_BlockRead(pFile->pIoman, nItemLBA, ulSectors, buffer, FF_FALSE);
		if(slRetVal < 0) {
			return slRetVal;
		}
		
		Count -= (SequentialClusters + 1);
		pFile->AddrCurrentCluster = FF_TraverseFAT(pFile->pIoman, pFile->AddrCurrentCluster, (SequentialClusters + 1), &Error);
		if(Error) {
			return Error;
		}
		pFile->CurrentCluster += (SequentialClusters + 1);
		buffer += ulSectors * pFile->pIoman->BlkSize;
		SequentialClusters = 0;
	}

	return FF_ERR_NONE;
}


static FF_ERROR FF_ExtendFile(FF_FILE *pFile, FF_T_UINT32 Size) {
	FF_IOMAN	*pIoman = pFile->pIoman;
	FF_T_UINT32 nBytesPerCluster = pIoman->pPartition->BlkSize * pIoman->pPartition->SectorsPerCluster;
	FF_T_UINT32 nTotalClustersNeeded = (Size + nBytesPerCluster-1) / nBytesPerCluster;
	FF_T_UINT32 nClusterToExtend; 
	FF_T_UINT32 CurrentCluster, NextCluster;
	FF_T_UINT32	i;
	FF_DIRENT	OriginalEntry;
	FF_ERROR	Error;

	if((pFile->Mode & FF_MODE_WRITE) != FF_MODE_WRITE) {
		return (FF_ERR_FILE_NOT_OPENED_IN_WRITE_MODE | FF_EXTENDFILE);
	}

	if(pFile->Filesize == 0 && pFile->ObjectCluster == 0) {	// No Allocated clusters.
		// Create a Cluster chain!
		pFile->AddrCurrentCluster = FF_CreateClusterChain(pFile->pIoman, &Error);

		if(Error) {
			return Error;
		}

		Error = FF_GetEntry(pIoman, pFile->DirEntry, pFile->DirCluster, &OriginalEntry);
		
		if(!Error) {
			OriginalEntry.ObjectCluster = pFile->AddrCurrentCluster;
			Error = FF_PutEntry(pIoman, pFile->DirEntry, pFile->DirCluster, &OriginalEntry);
		}

		if(Error) {
			return Error;
		}

		pFile->ObjectCluster = pFile->AddrCurrentCluster;
		pFile->iChainLength = 1;
		pFile->CurrentCluster = 0;
		pFile->iEndOfChain = pFile->AddrCurrentCluster;
	}
	
	if(pFile->iChainLength == 0) {	// First extension requiring the chain length, 
		pFile->iChainLength = FF_GetChainLength(pIoman, pFile->ObjectCluster, &pFile->iEndOfChain, &Error);
		if(Error) {
			return Error;
		}
	}

	nClusterToExtend = (nTotalClustersNeeded - pFile->iChainLength);

	if(nTotalClustersNeeded > pFile->iChainLength) {

		NextCluster = pFile->AddrCurrentCluster;
		FF_lockFAT(pIoman);
		{
			// HT This "<=" issue is now solved by asing for 1 extra byte
			// Thus not always asking for 1 extra cluster
			for(i = 0; i < nClusterToExtend; i++) {
				CurrentCluster = FF_FindEndOfChain(pIoman, NextCluster, &Error);
				if(Error) {
					FF_unlockFAT(pIoman);
					FF_DecreaseFreeClusters(pIoman, i);
					return Error;
				}
				NextCluster = FF_FindFreeCluster(pIoman, &Error);
				if(!Error && !NextCluster) {
					Error = FF_ERR_FAT_NO_FREE_CLUSTERS | FF_EXTENDFILE;
				}
				if(Error) {
					FF_unlockFAT(pIoman);
					FF_DecreaseFreeClusters(pIoman, i);
					return Error;
				}
				
				Error = FF_putFatEntry(pIoman, CurrentCluster, NextCluster);
				if(Error) {
					FF_unlockFAT(pIoman);
					FF_DecreaseFreeClusters(pIoman, i);
					return Error;
				}
				Error = FF_putFatEntry(pIoman, NextCluster, 0xFFFFFFFF);
				if(Error) {
					FF_unlockFAT(pIoman);
					FF_DecreaseFreeClusters(pIoman, i);
					return Error;
				}
			}
			
			pFile->iEndOfChain = FF_FindEndOfChain(pIoman, NextCluster, &Error);
			if(Error) {
				FF_unlockFAT(pIoman);
				FF_DecreaseFreeClusters(pIoman, i);
				return Error;
			}
		}
		FF_unlockFAT(pIoman);
		
		pFile->iChainLength += i;
		Error = FF_DecreaseFreeClusters(pIoman, i);	// Keep Tab of Numbers for fast FreeSize()
		if(Error) {
			return Error;
		}
	}

	return FF_ERR_NONE;
}

static FF_ERROR FF_WriteClusters(FF_FILE *pFile, FF_T_UINT32 Count, FF_T_UINT8 *buffer) {
	FF_T_UINT32 ulSectors;
	FF_T_UINT32 SequentialClusters = 0;
	FF_T_UINT32 nItemLBA;
	FF_T_SINT32 slRetVal;
	FF_ERROR	Error;

	while(Count != 0) {
		if((Count - 1) > 0) {
			SequentialClusters = FF_GetSequentialClusters(pFile->pIoman, pFile->AddrCurrentCluster, (Count - 1), &Error);
			if(Error) {
				return Error;
			}
		}
		ulSectors = (SequentialClusters + 1) * pFile->pIoman->pPartition->SectorsPerCluster;
		nItemLBA = FF_Cluster2LBA(pFile->pIoman, pFile->AddrCurrentCluster);
		nItemLBA = FF_getRealLBA(pFile->pIoman, nItemLBA);

		slRetVal = FF_BlockWrite(pFile->pIoman, nItemLBA, ulSectors, buffer, FF_FALSE);

		if(slRetVal < 0) {
			return slRetVal;
		}
		
		Count -= (SequentialClusters + 1);
		pFile->AddrCurrentCluster = FF_TraverseFAT(pFile->pIoman, pFile->AddrCurrentCluster, (SequentialClusters + 1), &Error);
		if(Error) {
			return Error;
		}
		pFile->CurrentCluster += (SequentialClusters + 1);
		buffer += ulSectors * pFile->pIoman->BlkSize;
		SequentialClusters = 0;
	}

	return 0;
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
FF_T_SINT32 FF_Read(FF_FILE *pFile, FF_T_UINT32 ElementSize, FF_T_UINT32 Count, FF_T_UINT8 *buffer) {
	FF_T_UINT32 nBytes = ElementSize * Count;
	FF_T_UINT32	nBytesRead = 0;
	FF_T_UINT32 nBytesToRead;
	FF_IOMAN	*pIoman;
	FF_BUFFER	*pBuffer;
	FF_T_UINT32 nRelBlockPos;
	FF_T_UINT32	nItemLBA;
	FF_T_SINT32	RetVal = 0;
	FF_T_UINT16	sSectors;
	FF_T_UINT32 nRelClusterPos;
	FF_T_UINT32 nBytesPerCluster;
	FF_T_UINT32	nClusterDiff;
	FF_ERROR	Error;

	if(!pFile) {
		return (FF_ERR_NULL_POINTER | FF_READ);
	}
	Error = FF_CheckValid (pFile);
	if (Error)
		return Error;

	if(!(pFile->Mode & FF_MODE_READ)) {
		return (FF_ERR_FILE_NOT_OPENED_IN_READ_MODE | FF_READ);
	}

	pIoman = pFile->pIoman;

	if(pFile->FilePointer >= pFile->Filesize) {
		return 0;
	}

	if((pFile->FilePointer + nBytes) > pFile->Filesize) {
		nBytes = pFile->Filesize - pFile->FilePointer;
	}
	
	nClusterDiff = FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1) - pFile->CurrentCluster;
	if(nClusterDiff) {
		if(pFile->CurrentCluster < FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1)) {
			pFile->AddrCurrentCluster = FF_TraverseFAT(pIoman, pFile->AddrCurrentCluster, nClusterDiff, &Error);
			if(Error) {
				return Error;
			}
			pFile->CurrentCluster += nClusterDiff;
		}
	}

	nRelBlockPos = FF_getMinorBlockEntry(pIoman, pFile->FilePointer, 1); // Get the position within a block.
	
	nItemLBA = FF_Cluster2LBA(pIoman, pFile->AddrCurrentCluster);
	nItemLBA = FF_getRealLBA(pIoman, nItemLBA + FF_getMajorBlockNumber(pIoman, pFile->FilePointer, 1)) + FF_getMinorBlockNumber(pIoman, pFile->FilePointer, 1);

	if((nRelBlockPos + nBytes) < pIoman->BlkSize) {	// Bytes to read are within a block and less than a block size.
		pBuffer = FF_GetBuffer(pIoman, nItemLBA, FF_MODE_READ);
		{
			if(!pBuffer) {
				return (FF_ERR_DEVICE_DRIVER_FAILED | FF_READ);
			}
			memcpy(buffer, (pBuffer->pBuffer + nRelBlockPos), nBytes);
		}
		FF_ReleaseBuffer(pIoman, pBuffer);

		pFile->FilePointer += nBytes;
		
		return nBytes;		// Return the number of bytes read.

	} else {

		//---------- Read (memcpy) to a Sector Boundary
		if(nRelBlockPos != 0) {	// Not on a sector boundary, at this point the LBA is known.
			nBytesToRead = pIoman->BlkSize - nRelBlockPos;
			pBuffer = FF_GetBuffer(pIoman, nItemLBA, FF_MODE_READ);
			{
				if(!pBuffer) {
					return (FF_ERR_DEVICE_DRIVER_FAILED | FF_READ);
				}
				// Here we copy to the sector boudary.
				memcpy(buffer, (pBuffer->pBuffer + nRelBlockPos), nBytesToRead);
			}
			FF_ReleaseBuffer(pIoman, pBuffer);

			nBytes				-= nBytesToRead;
			nBytesRead			+= nBytesToRead;
			pFile->FilePointer	+= nBytesToRead;
			buffer				+= nBytesToRead;
			
		}

		//---------- Read to a Cluster Boundary
		
		nRelClusterPos = FF_getClusterPosition(pIoman, pFile->FilePointer, 1);
		nBytesPerCluster = (pIoman->pPartition->SectorsPerCluster * pIoman->BlkSize);
		if(nRelClusterPos != 0 && nBytes >= nBytesPerCluster) { // Need to get to cluster boundary
			
			nClusterDiff = FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1) - pFile->CurrentCluster;
			if(nClusterDiff) {
				if(pFile->CurrentCluster < FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1)) {
					pFile->AddrCurrentCluster = FF_TraverseFAT(pIoman, pFile->AddrCurrentCluster, nClusterDiff, &Error);
					if(Error) {
						return Error;	// Returning an error, ensuring we are signed (error flag).
					}
					pFile->CurrentCluster += nClusterDiff;
				}
			}
		
			nItemLBA = FF_Cluster2LBA(pIoman, pFile->AddrCurrentCluster);
			nItemLBA = FF_getRealLBA(pIoman, nItemLBA + FF_getMajorBlockNumber(pIoman, pFile->FilePointer, 1)) + FF_getMinorBlockNumber(pIoman, pFile->FilePointer, 1);

			sSectors = (FF_T_UINT16) (pIoman->pPartition->SectorsPerCluster - (nRelClusterPos / pIoman->BlkSize));
			
			RetVal = FF_BlockRead(pIoman, nItemLBA, (FF_T_UINT32) sSectors, buffer, FF_FALSE);
			if(RetVal < 0) {
				return RetVal;
			}
			
			nBytesToRead		 = sSectors * pIoman->BlkSize;
			nBytes				-= nBytesToRead;
			buffer				+= nBytesToRead;
			nBytesRead			+= nBytesToRead;
			pFile->FilePointer	+= nBytesToRead;

		}

		//---------- Read Clusters
		if(nBytes >= nBytesPerCluster) {
			//----- Thanks to Christopher Clark of DigiPen Institute of Technology in Redmond, US adding this traversal check.
			nClusterDiff = FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1) - pFile->CurrentCluster;
			if(nClusterDiff) {
				if(pFile->CurrentCluster < FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1)) {
					pFile->AddrCurrentCluster = FF_TraverseFAT(pIoman, pFile->AddrCurrentCluster, nClusterDiff, &Error);
					if(Error) {
						return Error;
					}
					pFile->CurrentCluster += nClusterDiff;
				}
			}
			//----- End of Contributor fix.

			RetVal = FF_ReadClusters(pFile, (nBytes / nBytesPerCluster), buffer);
			if(RetVal < 0) {
				return RetVal;
			}
			nBytesToRead = (nBytesPerCluster *  (nBytes / nBytesPerCluster));

			pFile->FilePointer	+= nBytesToRead;

			nBytes			-= nBytesToRead;
			buffer			+= nBytesToRead;
			nBytesRead		+= nBytesToRead;
		}

		//---------- Read Remaining Blocks
		if(nBytes >= pIoman->BlkSize) {
			sSectors = (FF_T_UINT16) (nBytes / pIoman->BlkSize);
			
			nClusterDiff = FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1) - pFile->CurrentCluster;
			if(nClusterDiff) {
				if(pFile->CurrentCluster < FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1)) {
					pFile->AddrCurrentCluster = FF_TraverseFAT(pIoman, pFile->AddrCurrentCluster, nClusterDiff, &Error);
					if(Error) {
						return Error;
					}
					pFile->CurrentCluster += nClusterDiff;
				}
			}
			
			nItemLBA = FF_Cluster2LBA(pIoman, pFile->AddrCurrentCluster);
			nItemLBA = FF_getRealLBA(pIoman, nItemLBA + FF_getMajorBlockNumber(pIoman, pFile->FilePointer, 1)) + FF_getMinorBlockNumber(pIoman, pFile->FilePointer, 1);

			RetVal = FF_BlockRead(pIoman, nItemLBA, (FF_T_UINT32) sSectors, buffer, FF_FALSE);

			if(RetVal < 0) {
				return RetVal;
			}
			
			nBytesToRead = sSectors * pIoman->BlkSize;
			pFile->FilePointer	+= nBytesToRead;
			nBytes				-= nBytesToRead;
			buffer				+= nBytesToRead;
			nBytesRead			+= nBytesToRead;
		}

		//---------- Read (memcpy) Remaining Bytes
		if(nBytes > 0) {
			
			nClusterDiff = FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1) - pFile->CurrentCluster;
			if(nClusterDiff) {
				if(pFile->CurrentCluster < FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1)) {
					pFile->AddrCurrentCluster = FF_TraverseFAT(pIoman, pFile->AddrCurrentCluster, nClusterDiff, &Error);
					if(Error) {
						return Error;
					}
					pFile->CurrentCluster += nClusterDiff;
				}
			}
			
			nItemLBA = FF_Cluster2LBA(pIoman, pFile->AddrCurrentCluster);
			nItemLBA = FF_getRealLBA(pIoman, nItemLBA + FF_getMajorBlockNumber(pIoman, pFile->FilePointer, 1)) + FF_getMinorBlockNumber(pIoman, pFile->FilePointer, 1);
			pBuffer = FF_GetBuffer(pIoman, nItemLBA, FF_MODE_READ);
			{
				if(!pBuffer) {
					return (FF_ERR_DEVICE_DRIVER_FAILED | FF_READ);
				}
				memcpy(buffer, pBuffer->pBuffer, nBytes);
			}
			FF_ReleaseBuffer(pIoman, pBuffer);

			nBytesToRead = nBytes;
			pFile->FilePointer	+= nBytesToRead;
			nBytes				-= nBytesToRead;
			buffer				+= nBytesToRead;
			nBytesRead			+= nBytesToRead;

		}
	}

	return nBytesRead;
}




/**
 *	@public
 *	@brief	Equivalent to fgetc()
 *
 *	@param	pFile		FF_FILE object that was created by FF_Open().
 *
 *	@return The character that was read (cast as a 32-bit interger). -1 on EOF.
 *	@return FF_ERROR code. (Check with if(FF_isERR(RetVal)) {}).
 *	@return -1 EOF (end of file).
 *
 **/
FF_T_SINT32 FF_GetC(FF_FILE *pFile) {
	FF_T_UINT32		fileLBA;
	FF_BUFFER		*pBuffer;
	FF_T_UINT8		retChar;
	FF_T_UINT32		relMinorBlockPos;
	FF_T_UINT32     clusterNum;
	FF_T_UINT32		nClusterDiff;
	FF_ERROR		Error;
	
	
	if(!pFile) {
		return (FF_ERR_NULL_POINTER | FF_GETC);	// Ensure this is a signed error.
	}

	if(!(pFile->Mode & FF_MODE_READ)) {
		return (FF_ERR_FILE_NOT_OPENED_IN_READ_MODE | FF_GETC);
	}
	
	if(pFile->FilePointer >= pFile->Filesize) {
		return -1; // EOF!	
	}

	relMinorBlockPos	= FF_getMinorBlockEntry(pFile->pIoman, pFile->FilePointer, 1);
	clusterNum			= FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1);

	nClusterDiff = FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1) - pFile->CurrentCluster;
	if(nClusterDiff) {
		if(pFile->CurrentCluster < FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1)) {
			pFile->AddrCurrentCluster = FF_TraverseFAT(pFile->pIoman, pFile->AddrCurrentCluster, nClusterDiff, &Error);
			if(Error) {
				return Error;
			}
			pFile->CurrentCluster += nClusterDiff;
		}
	}
	

	fileLBA = FF_Cluster2LBA(pFile->pIoman, pFile->AddrCurrentCluster)	+ FF_getMajorBlockNumber(pFile->pIoman, pFile->FilePointer, (FF_T_UINT16) 1);
	fileLBA = FF_getRealLBA (pFile->pIoman, fileLBA)		+ FF_getMinorBlockNumber(pFile->pIoman, pFile->FilePointer, (FF_T_UINT16) 1);
	
	pBuffer = FF_GetBuffer(pFile->pIoman, fileLBA, FF_MODE_READ);
	{
		if(!pBuffer) {
			return (FF_ERR_DEVICE_DRIVER_FAILED | FF_GETC);
		}
		retChar = pBuffer->pBuffer[relMinorBlockPos];
	}
	FF_ReleaseBuffer(pFile->pIoman, pBuffer);

	pFile->FilePointer += 1;

	return (FF_T_SINT32) retChar;
}


/**
 * @public
 * @brief	Gets a Line from a Text File, but no more than ulLimit charachters. The line will be NULL terminated.
 *
 *			The behaviour of this function is undefined when called on a binary file.
 *			It should just read in ulLimit bytes of binary, and ZERO terminate the line.
 *
 *			This function works for both UNIX line feeds, and Windows CRLF type files.
 *
 * @param	pFile	The FF_FILE object pointer.
 * @param	szLine	The charachter buffer where the line should be stored.
 * @param	ulLimit	This should be the max number of charachters that szLine can hold.
 *
 * @return	The number of charachters read from the line, on success.
 * @return	0 when no more lines are available, or when ulLimit is 0.
 * @return	FF_ERR_NULL_POINTER if pFile or szLine are NULL;
 *
 **/
FF_T_SINT32 FF_GetLine(FF_FILE *pFile, FF_T_INT8 *szLine, FF_T_UINT32 ulLimit) {
	FF_T_SINT32 c = 0;
	FF_T_UINT32 i;

	if(!pFile || !szLine) {
		return (FF_ERR_NULL_POINTER | FF_GETLINE);
	}

	for(i = 0; i < (ulLimit - 1) && (c=FF_GetC(pFile)) >= 0 && c != '\n'; ++i) {
		if(c == '\r') {
			i--;
		} else {
			szLine[i] = (FF_T_INT8) c;
		}
	}

	szLine[i] = '\0';	// Always do this before sending the err, we don't know what the user will
						// do with this buffer if they don't see the error.
	
	if(FF_isERR(c)) {
		return c;		// Return 'c' as an error code.
	}

	return i;
}

// HT made inline
/*FF_T_UINT32 FF_Tell(FF_FILE *pFile) {
	if(pFile) {
		return pFile->FilePointer;
	}

	return 0;
}*/


/**
 *	@public
 *	@brief	Writes data to a File.
 *
 *	@param	pFile			FILE Pointer.
 *	@param	ElementSize		Size of an Element of Data to be copied. (in bytes). 
 *	@param	Count			Number of Elements of Data to be copied. (ElementSize * Count must not exceed ((2^31)-1) bytes. (2GB). For best performance, multiples of 512 bytes or Cluster sizes are best.
 *	@param	buffer			Byte-wise buffer containing the data to be written.
 *
 *	@return
 **/
FF_T_SINT32 FF_Write(FF_FILE *pFile, FF_T_UINT32 ElementSize, FF_T_UINT32 Count, FF_T_UINT8 *buffer) {
	FF_T_UINT32 nBytes = ElementSize * Count;
	FF_T_UINT32	nBytesWritten = 0;
	FF_T_UINT32 nBytesToWrite;
	FF_IOMAN	*pIoman;
	FF_BUFFER	*pBuffer;
	FF_T_UINT32 nRelBlockPos;
	FF_T_UINT32	nItemLBA;
	FF_T_SINT32	slRetVal = 0;
	FF_T_UINT16	sSectors;
	FF_T_UINT32 nRelClusterPos;
	FF_T_UINT32 nBytesPerCluster, nClusterDiff, nClusters;
	FF_ERROR	Error;

	if(!pFile) {
		return (FF_ERR_NULL_POINTER | FF_WRITE);
	}

	Error = FF_CheckValid (pFile);
	if (Error)
		return Error;

	if(!(pFile->Mode & FF_MODE_WRITE)) {
		return (FF_ERR_FILE_NOT_OPENED_IN_WRITE_MODE | FF_WRITE);
	}

	// Make sure a write is after the append point.
	if((pFile->Mode & FF_MODE_APPEND)) {
		if(pFile->FilePointer < pFile->Filesize) {
			FF_Seek(pFile, 0, FF_SEEK_END);
		}
	}

	pIoman = pFile->pIoman;

	nBytesPerCluster = (pIoman->pPartition->SectorsPerCluster * pIoman->BlkSize);

	// Extend File for atleast nBytes!
	// Handle file-space allocation

	// HT: + 1 byte because the code assumes there is always a next cluster
	Error = FF_ExtendFile(pFile, pFile->FilePointer + nBytes + 1);

	if(Error) {
		return Error;	
	}

	nRelBlockPos = FF_getMinorBlockEntry(pIoman, pFile->FilePointer, 1); // Get the position within a block.
	
	nClusterDiff = FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1) - pFile->CurrentCluster;
	if(nClusterDiff) {
		if(pFile->CurrentCluster != FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1)) {
			pFile->AddrCurrentCluster = FF_TraverseFAT(pIoman, pFile->AddrCurrentCluster, nClusterDiff, &Error);
			if(Error) {
				return Error;
			}
			pFile->CurrentCluster += nClusterDiff;
		}
	}
	
	nItemLBA = FF_Cluster2LBA(pIoman, pFile->AddrCurrentCluster);
	nItemLBA = FF_getRealLBA(pIoman, nItemLBA + FF_getMajorBlockNumber(pIoman, pFile->FilePointer, 1)) + FF_getMinorBlockNumber(pIoman, pFile->FilePointer, 1);

	if((nRelBlockPos + nBytes) < pIoman->BlkSize) {	// Bytes to read are within a block and less than a block size.
		pBuffer = FF_GetBuffer(pIoman, nItemLBA, FF_MODE_WRITE);
		{
			if(!pBuffer) {
				return (FF_ERR_DEVICE_DRIVER_FAILED | FF_WRITE);
			}
			memcpy((pBuffer->pBuffer + nRelBlockPos), buffer, nBytes);
		}
		FF_ReleaseBuffer(pIoman, pBuffer);

		pFile->FilePointer += nBytes;
		nBytesWritten = nBytes;
		//return nBytes;		// Return the number of bytes read.

	} else {

		//---------- Write (memcpy) to a Sector Boundary
		if(nRelBlockPos != 0) {	// Not on a sector boundary, at this point the LBA is known.
			nBytesToWrite = pIoman->BlkSize - nRelBlockPos;
			pBuffer = FF_GetBuffer(pIoman, nItemLBA, FF_MODE_WRITE);
			{
				if(!pBuffer) {
					return (FF_ERR_DEVICE_DRIVER_FAILED | FF_WRITE);
				}
				// Here we copy to the sector boudary.
				memcpy((pBuffer->pBuffer + nRelBlockPos), buffer, nBytesToWrite);
			}
			FF_ReleaseBuffer(pIoman, pBuffer);

			nBytes				-= nBytesToWrite;
			nBytesWritten		+= nBytesToWrite;
			pFile->FilePointer	+= nBytesToWrite;
			buffer				+= nBytesToWrite;
		}

		//---------- Write to a Cluster Boundary
		
		nRelClusterPos = FF_getClusterPosition(pIoman, pFile->FilePointer, 1);
		if(nRelClusterPos != 0 && nBytes >= nBytesPerCluster) { // Need to get to cluster boundary
			
			nClusterDiff = FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1) - pFile->CurrentCluster;
			if(nClusterDiff) {
				if(pFile->CurrentCluster < FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1)) {
					pFile->AddrCurrentCluster = FF_TraverseFAT(pIoman, pFile->AddrCurrentCluster, nClusterDiff, &Error);
					if(Error) {
						return Error;
					}
					pFile->CurrentCluster += nClusterDiff;
				}
			}
		
			nItemLBA = FF_Cluster2LBA(pIoman, pFile->AddrCurrentCluster);
			nItemLBA = FF_getRealLBA(pIoman, nItemLBA + FF_getMajorBlockNumber(pIoman, pFile->FilePointer, 1)) + FF_getMinorBlockNumber(pIoman, pFile->FilePointer, 1);

			sSectors = (FF_T_UINT16) (pIoman->pPartition->SectorsPerCluster - (nRelClusterPos / pIoman->BlkSize));

			slRetVal = FF_BlockWrite(pFile->pIoman, nItemLBA, sSectors, buffer, FF_FALSE);
			if(slRetVal < 0) {
				return slRetVal;
			}
			
			nBytesToWrite		 = sSectors * pIoman->BlkSize;
			nBytes				-= nBytesToWrite;
			buffer				+= nBytesToWrite;
			nBytesWritten		+= nBytesToWrite;
			pFile->FilePointer	+= nBytesToWrite;

		}

		//---------- Write Clusters
		if(nBytes >= nBytesPerCluster) {
			//----- Thanks to Christopher Clark of DigiPen Institute of Technology in Redmond, US adding this traversal check.
			nClusterDiff = FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1) - pFile->CurrentCluster;
			if(nClusterDiff) {
				if(pFile->CurrentCluster < FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1)) {
					pFile->AddrCurrentCluster = FF_TraverseFAT(pIoman, pFile->AddrCurrentCluster, nClusterDiff, &Error);
					if(Error) {
						return Error;
					}
					pFile->CurrentCluster += nClusterDiff;
				}
			}
			//----- End of Contributor fix.

			nClusters = (nBytes / nBytesPerCluster);
			
			slRetVal = FF_WriteClusters(pFile, nClusters, buffer);
			if(slRetVal < 0) {
				return slRetVal;
			}
			
			nBytesToWrite = (nBytesPerCluster *  nClusters);
			
			pFile->FilePointer	+= nBytesToWrite;

			nBytes				-= nBytesToWrite;
			buffer				+= nBytesToWrite;
			nBytesWritten		+= nBytesToWrite;
		}

		//---------- Write Remaining Blocks
		if(nBytes >= pIoman->BlkSize) {
			sSectors = (FF_T_UINT16) (nBytes / pIoman->BlkSize);
			
			nClusterDiff = FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1) - pFile->CurrentCluster;
			if(nClusterDiff) {
				if(pFile->CurrentCluster < FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1)) {
					pFile->AddrCurrentCluster = FF_TraverseFAT(pIoman, pFile->AddrCurrentCluster, nClusterDiff, &Error);
					if(Error) {
						return Error;
					}
					pFile->CurrentCluster += nClusterDiff;
				}
			}			
			
			nItemLBA = FF_Cluster2LBA(pIoman, pFile->AddrCurrentCluster);
			nItemLBA = FF_getRealLBA(pIoman, nItemLBA + FF_getMajorBlockNumber(pIoman, pFile->FilePointer, 1)) + FF_getMinorBlockNumber(pIoman, pFile->FilePointer, 1);
			
			slRetVal = FF_BlockWrite(pFile->pIoman, nItemLBA, sSectors, buffer, FF_FALSE);
			if(slRetVal < 0) {
				return slRetVal;
			}
			
			nBytesToWrite = sSectors * pIoman->BlkSize;
			pFile->FilePointer	+= nBytesToWrite;
			nBytes				-= nBytesToWrite;
			buffer				+= nBytesToWrite;
			nBytesWritten		+= nBytesToWrite;
		}

		//---------- Write (memcpy) Remaining Bytes
		if(nBytes > 0) {
			
			nClusterDiff = FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1) - pFile->CurrentCluster;
			if(nClusterDiff) {
				if(pFile->CurrentCluster < FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1)) {
					pFile->AddrCurrentCluster = FF_TraverseFAT(pIoman, pFile->AddrCurrentCluster, nClusterDiff, &Error);
					if(Error) {
						return Error;
					}
					pFile->CurrentCluster += nClusterDiff;
				}
			}
			
			nItemLBA = FF_Cluster2LBA(pIoman, pFile->AddrCurrentCluster);
			nItemLBA = FF_getRealLBA(pIoman, nItemLBA + FF_getMajorBlockNumber(pIoman, pFile->FilePointer, 1)) + FF_getMinorBlockNumber(pIoman, pFile->FilePointer, 1);
			pBuffer = FF_GetBuffer(pIoman, nItemLBA, FF_MODE_WRITE);
			{
				if(!pBuffer) {
					return (FF_ERR_DEVICE_DRIVER_FAILED | FF_WRITE);
				}
				memcpy(pBuffer->pBuffer, buffer, nBytes);
			}
			FF_ReleaseBuffer(pIoman, pBuffer);

			nBytesToWrite = nBytes;
			pFile->FilePointer	+= nBytesToWrite;
			nBytes				-= nBytesToWrite;
			buffer				+= nBytesToWrite;
			nBytesWritten			+= nBytesToWrite;

		}
	}

	if(pFile->FilePointer > pFile->Filesize) {
		pFile->Filesize = pFile->FilePointer;
	}

	return nBytesWritten;
}


/**
 *	@public
 *	@brief	Writes a char to a FILE.
 *
 *	@param	pFile		FILE Pointer.
 *	@param	pa_cValue	Char to be placed in the file.
 *
 *	@return	Returns the value written to the file, or a value less than 0.
 *
 **/
FF_T_SINT32 FF_PutC(FF_FILE *pFile, FF_T_UINT8 pa_cValue) {
	FF_BUFFER	*pBuffer;
	FF_T_UINT32 iItemLBA;
	FF_T_UINT32 iRelPos;
	FF_T_UINT32 nClusterDiff;
	FF_ERROR	Error;
	
	if(!pFile) {	// Ensure we don't have a Null file pointer on a Public interface.
		return (FF_ERR_NULL_POINTER | FF_PUTC);
	}

	if(!(pFile->Mode & FF_MODE_WRITE)) {
		return (FF_ERR_FILE_NOT_OPENED_IN_WRITE_MODE | FF_PUTC);
	}

	// Make sure a write is after the append point.
	if((pFile->Mode & FF_MODE_APPEND)) {
		if(pFile->FilePointer < pFile->Filesize) {
			Error = FF_Seek(pFile, 0, FF_SEEK_END);
			if(Error) {
				return Error;
			}
		}
	}

	iRelPos = FF_getMinorBlockEntry(pFile->pIoman, pFile->FilePointer, 1);
	
	// Handle File Space Allocation.
	Error = FF_ExtendFile(pFile, pFile->FilePointer + 1);
	if(Error) {
		return Error;
	}
	
	nClusterDiff = FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1) - pFile->CurrentCluster;
	if(nClusterDiff) {
		if(pFile->CurrentCluster < FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1)) {
			pFile->AddrCurrentCluster = FF_TraverseFAT(pFile->pIoman, pFile->AddrCurrentCluster, nClusterDiff, &Error);
			if(Error) {
				return Error;
			}
			pFile->CurrentCluster += nClusterDiff;
		}
	}

	iItemLBA = FF_Cluster2LBA(pFile->pIoman, pFile->AddrCurrentCluster) + FF_getMajorBlockNumber(pFile->pIoman, pFile->FilePointer, (FF_T_UINT16) 1);
	iItemLBA = FF_getRealLBA (pFile->pIoman, iItemLBA)					+ FF_getMinorBlockNumber(pFile->pIoman, pFile->FilePointer, (FF_T_UINT16) 1);
	
	pBuffer = FF_GetBuffer(pFile->pIoman, iItemLBA, FF_MODE_WRITE);
	{
		if(!pBuffer) {
			return (FF_ERR_DEVICE_DRIVER_FAILED | FF_PUTC);
		}
		FF_putChar(pBuffer->pBuffer, (FF_T_UINT16) iRelPos, pa_cValue);
	}
	FF_ReleaseBuffer(pFile->pIoman, pBuffer);

	pFile->FilePointer += 1;
	if(pFile->Filesize < (pFile->FilePointer)) {
		pFile->Filesize += 1;
	}
	return pa_cValue;
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
 *	@return FF_ERR_NULL_POINTER if a FF_FILE pointer was not recieved.
 *	@return -3 if an invalid origin was provided.
 *	
 **/
FF_ERROR FF_Seek(FF_FILE *pFile, FF_T_SINT32 Offset, FF_T_INT8 Origin) {
	
	FF_ERROR	Error;

	if(!pFile) {
		return (FF_ERR_NULL_POINTER | FF_SEEK);
	}

	Error = FF_FlushCache(pFile->pIoman);
	if(Error) {
		return Error;
	}

	switch(Origin) {
		case FF_SEEK_SET:
			if((FF_T_UINT32) Offset <= pFile->Filesize && Offset >= 0) {
				pFile->FilePointer = Offset;
				pFile->CurrentCluster = FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1);
				pFile->AddrCurrentCluster = FF_TraverseFAT(pFile->pIoman, pFile->ObjectCluster, pFile->CurrentCluster, &Error);
				if(Error) {
					return Error;
				}
			} else {
				return -2;
			}
			break;

		case FF_SEEK_CUR:
			if((Offset + pFile->FilePointer) <= pFile->Filesize && (Offset + (FF_T_SINT32) pFile->FilePointer) >= 0) {
				pFile->FilePointer = Offset + pFile->FilePointer;
				pFile->CurrentCluster = FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1);
				pFile->AddrCurrentCluster = FF_TraverseFAT(pFile->pIoman, pFile->ObjectCluster, pFile->CurrentCluster, &Error);
				if(Error) {
					return Error;
				}
			} else {
				return -2;
			}
			break;
	
		case FF_SEEK_END:
			if((Offset + (FF_T_SINT32) pFile->Filesize) >= 0 && (Offset + pFile->Filesize) <= pFile->Filesize) {
				pFile->FilePointer = Offset + pFile->Filesize;
				pFile->CurrentCluster = FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1);
				pFile->AddrCurrentCluster = FF_TraverseFAT(pFile->pIoman, pFile->ObjectCluster, pFile->CurrentCluster, &Error);
				if(Error) {
					return Error;
				}
			} else {
				return -2;
			}
			break;

		default:
			return -3;
		
	}

	return 0;
}

#ifdef FF_REMOVABLE_MEDIA
/**
 *	@public
 *	@brief	Invalidate all file handles belonging to pIoman
 *
 *	@param	pIoMan		FF_IOMAN object that was created by FF_CreateIOMAN().
 *
 *	@return 0 if no handles were open
 *	@return >0 the amount of handles that were invalidated
 *	@return <0 probably an invalid FF_IOMAN pointer
 *
 **/
FF_T_SINT32	 FF_Invalidate (FF_IOMAN *pIoman) ///< Invalidate all handles belonging to pIoman
{
	FF_T_SINT32	 Result;
	FF_FILE *pFileChain;
	if (!pIoman) {
		return (FF_ERR_NULL_POINTER | FF_INVALIDATE);
	}
	Result = 0;
	FF_PendSemaphore(pIoman->pSemaphore);
	{	// Semaphore is required, or linked list might change

		pFileChain = (FF_FILE *) pIoman->FirstFile;
		if (pFileChain) {
			// Count elements in FirstFile
			do {
				pFileChain->ValidFlags |= FF_VALID_FLAG_INVALID;
				Result++;
				pFileChain = pFileChain->Next;
			} while (pFileChain);
		}
	}
	FF_ReleaseSemaphore(pIoman->pSemaphore);
	return Result;
}
#endif	// FF_REMOVABLE_MEDIA

/**
 *	@public
 *	@brief	Check validity of file handle
 *
 *	@param	pFile		FF_FILE object that was created by FF_Open().
 *
 *	@return 0 on sucess.
 *	@return FF_ERR_NULL_POINTER       if a null pointer was provided.
 *	@return FF_ERR_FILE_BAD_HANDLE    if handle is not recognized
 *	@return FF_ERR_FILE_MEDIA_REMOVED please call FF_Close
 *
 **/
FF_ERROR FF_CheckValid (FF_FILE *pFile)
{
	FF_FILE *pFileChain;

	if (!pFile || !pFile->pIoman) {
		return (FF_ERR_NULL_POINTER | FF_CHECKVALID);
	}

	pFileChain = (FF_FILE *)pFile->pIoman->FirstFile;
	while (pFileChain) {
		if (pFileChain == pFile) {
#ifdef FF_REMOVABLE_MEDIA
			if (pFileChain->ValidFlags & FF_VALID_FLAG_INVALID) {
				return (FF_ERR_FILE_MEDIA_REMOVED | FF_CHECKVALID);
			}
#endif
			return FF_ERR_NONE;
		}
		pFileChain = pFileChain->Next;
	}
	return (FF_ERR_FILE_BAD_HANDLE | FF_CHECKVALID);
}

#ifdef FF_TIME_SUPPORT
/**
 *	@public
 *	@brief	Set the time-stamp(s) of a file entry
 *
 *	@param	pFile		FF_FILE object that was created by FF_Open().
 *	@param	pTime       FF_SYSTEMTIME the time stamp
 *	@param	aWhat       FF_T_UINT a combination of enum ETimeMask
 *
 *	@return 0 or FF_ERROR
 *
 **/
FF_ERROR FF_SetFileTime(FF_FILE *pFile, FF_SYSTEMTIME *pTime, FF_T_UINT aWhat) {
	FF_DIRENT	OriginalEntry;
	FF_ERROR	Error;
	Error = FF_CheckValid (pFile);
	if (Error)
		return Error;

	if (pFile->ValidFlags & FF_VALID_FLAG_DELETED) { //if (pFile->FileDeleted)
		return FF_ERR_FILE_NOT_FOUND | FF_SETFILETIME;
	}
	if (!(pFile->Mode & (FF_MODE_WRITE |FF_MODE_APPEND))) {
		return FF_ERR_FILE_NOT_OPENED_IN_WRITE_MODE | FF_SETFILETIME;
	}
	// Update the Dirent!
	Error = FF_GetEntry(pFile->pIoman, pFile->DirEntry, pFile->DirCluster, &OriginalEntry);
	if (!Error) {
		if (aWhat & ETimeCreate) OriginalEntry.CreateTime = *pTime;		///< Date and Time Created.
		if (aWhat & ETimeMod)    OriginalEntry.ModifiedTime = *pTime;	///< Date and Time Modified.
		if (aWhat & ETimeAccess) OriginalEntry.AccessedTime = *pTime;	///< Date of Last Access.
		Error = FF_PutEntry(pFile->pIoman, pFile->DirEntry, pFile->DirCluster, &OriginalEntry);
	}
	if (!Error)
		Error = FF_FlushCache(pFile->pIoman);		// Ensure all modfied blocks are flushed to disk!
	return Error;
}

/**
 *	@public
 *	@brief	Set the time-stamp(s) of a file entry (by name)
 *
 *	@param	pIoman		FF_IOMAN device handle
 *	@param	path		FF_T_INT8/FF_T_WCHAR name of the file
 *	@param	pTime       FF_SYSTEMTIME the time stamp
 *	@param	aWhat       FF_T_UINT a combination of enum ETimeMask
 *
 *	@return 0 or FF_ERROR
 *
 **/
#ifdef FF_UNICODE_SUPPORT
FF_ERROR FF_SetTime(FF_IOMAN *pIoman, const FF_T_WCHAR *path, FF_SYSTEMTIME *pTime, FF_T_UINT aWhat)
#else
FF_ERROR FF_SetTime(FF_IOMAN *pIoman, const FF_T_INT8 *path, FF_SYSTEMTIME *pTime, FF_T_UINT aWhat)
#endif
{
	FF_DIRENT	OriginalEntry;
	FF_ERROR	Error;
	FF_T_UINT32     DirCluster;
	FF_T_UINT32     FileCluster;
	FF_T_INT        i;
#ifdef FF_UNICODE_SUPPORT
	FF_T_WCHAR	filename[FF_MAX_FILENAME];
#else
	FF_T_INT8	filename[FF_MAX_FILENAME];
#endif

#ifdef FF_UNICODE_SUPPORT
	i = (FF_T_UINT16) wcslen(path);
#else 
	i = (FF_T_UINT16) strlen(path);
#endif

	while(i != 0) {
		if(path[i] == '\\' || path[i] == '/') {
			break;
		}
		i--;
	}
#ifdef FF_UNICODE_SUPPORT
	wcsncpy(filename, (path + i + 1), FF_MAX_FILENAME);
#else
	strncpy(filename, (path + i + 1), FF_MAX_FILENAME);
#endif

	if(i == 0) {
		i = 1;
	}
	
	DirCluster = FF_FindDir(pIoman, path, i, &Error);
	if (Error)
		return Error;

	if (!DirCluster) {
		return FF_ERR_FILE_NOT_FOUND | FF_SETTIME;
	}
	FileCluster = FF_FindEntryInDir(pIoman, DirCluster, filename, 0, &OriginalEntry, &Error);
	if (Error)
		return Error;

	if (!FileCluster) {
		//logPrintf ("FF_SetTime: Can not find '%s'\n", filename);
		return FF_ERR_FILE_NOT_FOUND | FF_SETTIME;
	}

	// Update the Dirent!
	if (aWhat & ETimeCreate) OriginalEntry.CreateTime = *pTime;		///< Date and Time Created.
	if (aWhat & ETimeMod)    OriginalEntry.ModifiedTime = *pTime;	///< Date and Time Modified.
	if (aWhat & ETimeAccess) OriginalEntry.AccessedTime = *pTime;	///< Date of Last Access.
	Error = FF_PutEntry(pIoman, OriginalEntry.CurrentItem-1, DirCluster, &OriginalEntry);

	if (!Error)
		Error = FF_FlushCache(pIoman);		// Ensure all modfied blocks are flushed to disk!
	return Error;
}

#endif // FF_TIME_SUPPORT

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
FF_ERROR FF_Close(FF_FILE *pFile) {

	FF_FILE		*pFileChain;
	FF_DIRENT	OriginalEntry;
	FF_ERROR	Error;

	if(!pFile) {
		return (FF_ERR_NULL_POINTER | FF_CLOSE);
	}

	/*
     * HT thinks:
	 * It is important to check that:
	 * user doesn't supply invalid handle
	 * or a handle invalid because of "media removed"
	 */
	Error = FF_CheckValid (pFile);
#ifdef FF_REMOVABLE_MEDIA
	if (FF_GETERROR(Error) == FF_ERR_FILE_MEDIA_REMOVED) {
		FF_PendSemaphore(pFile->pIoman->pSemaphore);
		{
			pFileChain = (FF_FILE *) pFile->pIoman->FirstFile;
			if(pFileChain == pFile) {
				pFile->pIoman->FirstFile = pFile->Next;
			} else {
				while (pFileChain) {
					if (pFileChain->Next == pFile) {
						pFileChain->Next = pFile->Next;
						break;
					}
					pFileChain = pFileChain->Next;	// Forgot this one
				}
			}
		}	// Semaphore released, linked list was shortened!
		FF_ReleaseSemaphore(pFile->pIoman->pSemaphore);
		FF_FREE(pFile);  // So at least we have freed the pointer.
		return FF_ERR_NONE;
	}
#endif
	if (Error != FF_ERR_NONE) {
		return Error; // FF_ERR_FILE_BAD_HANDLE or FF_ERR_NULL_POINTER
	}

	/*
     * So here we have a normal valid file handle
	 */

	
	// UpDate Dirent if File-size has changed?
	if(!(pFile->ValidFlags & FF_VALID_FLAG_DELETED) && (pFile->Mode & (FF_MODE_WRITE |FF_MODE_APPEND))) {
		// Update the Dirent!
		Error = FF_GetEntry(pFile->pIoman, pFile->DirEntry, pFile->DirCluster, &OriginalEntry);
		// Error might be non-zero, but don't forget to remove handle from list
		// and to free the pFile pointer
		if (!Error && pFile->Filesize != OriginalEntry.Filesize) {
			OriginalEntry.Filesize = pFile->Filesize;
			Error = FF_PutEntry(pFile->pIoman, pFile->DirEntry, pFile->DirCluster, &OriginalEntry);
		}
	}
	if (!FF_GETERROR (Error)) {
		Error = FF_FlushCache(pFile->pIoman);		// Ensure all modfied blocks are flushed to disk!
	}

	// Handle Linked list!
	FF_PendSemaphore(pFile->pIoman->pSemaphore);
	{	// Semaphore is required, or linked list could become corrupted.
		pFileChain = (FF_FILE *) pFile->pIoman->FirstFile;
		if(pFileChain == pFile) {
			pFile->pIoman->FirstFile = pFile->Next;
		} else {
			while (pFileChain) {
				if (pFileChain->Next == pFile) {
					pFileChain->Next = pFile->Next;
					break;
				}
				pFileChain = pFileChain->Next;	// Forgot this one
			}
		}
	}	// Semaphore released, linked list was shortened!
	FF_ReleaseSemaphore(pFile->pIoman->pSemaphore);

	// If file written, flush to disk
	FF_FREE(pFile);
	// Simply free the pointer!
	return Error;
}
