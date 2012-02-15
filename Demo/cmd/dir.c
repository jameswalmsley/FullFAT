#include <stdlib.h>
#include <string.h>
#include "dir.h"
#include "../../src/ff_config.h"

#ifdef WIN32
#define stricmp	_stricmp
#define wcsicmp _wcsicmp
#else
#define	stricmp strcasecmp
#define wcsicmp wcscasecmp
#endif

struct _SD_DIR {
	unsigned long	ulTotalItems;
	unsigned long	ulMaxNameLength;
	unsigned long	ulTotalMemory;
	SD_DIRENT		*pFirstEntry;
};

SD_DIR SD_CreateDir() {
	SD_DIR dir = (SD_DIR) malloc(sizeof(struct _SD_DIR));
	
	if(dir) {
		memset(dir, 0, sizeof(struct _SD_DIR));
		return dir;
	}

	return NULL;
}

SD_ERROR SD_CleanupDir(SD_DIR Dir) {

	SD_DIRENT *pDirent = Dir->pFirstEntry;

	if(pDirent) {

		do {
			Dir->pFirstEntry = pDirent->pNextEntry;
			free(pDirent->szFileName);
			free(pDirent);
			pDirent = Dir->pFirstEntry;
		} while(pDirent);

	}

	free(Dir);

	return 0;
}

/*
SD_ERROR SD_SortDirents(SD_DIR Dir) {
	SD_DIRENT	*pDirent; //= Dir->pFirstEntry;
	SD_DIRENT	*pDir1, *pDir2;
	int			iSwaps;
	int			iResult;

	do {
		iSwaps = 0;
		pDirent = Dir->pFirstEntry;
		while(pDirent && pDirent->pNextEntry) {
			iResult = strcmp(pDirent->szFileName, pDirent->pNextEntry->szFileName);
			if(iResult == 1) {	// Put pCommand after the next command.
				iSwaps++;
				pDir1 = Dir->pFirstEntry;
				if(pDir1 == pDirent) {
					pDir2 = pDirent->pNextEntry;
					Dir->pFirstEntry = pDir2;
					pDirent->pNextEntry = pDir2->pNextEntry;
					pDir2->pNextEntry = pDirent;
				} else {
					while(pDir1 && pDir1->pNextEntry) {
						if(pDir1->pNextEntry == pDirent) {
							pDir2 = pDirent->pNextEntry;
							pDir1->pNextEntry = pDir2;
							pDirent->pNextEntry = pDir2->pNextEntry;
							pDir2->pNextEntry = pDirent;
							break;
						}
						pDir1 = pDir1->pNextEntry;
					}
				}

			}
			if(iResult == -1) {	// Leave current string where it is.
				

			}
			pDirent = pDirent->pNextEntry;
		}
	} while(iSwaps);
	
	return 0;
}*/

SD_ERROR SD_AddDirent(SD_DIR Dir, SD_DIRENT *pDirent) {
	SD_DIRENT *myDirent = (SD_DIRENT *) malloc(sizeof(SD_DIRENT));
	SD_DIRENT *pSortDirent;

	int i;

	if(!myDirent) {
		return 1; // Return error
	}

	memcpy(myDirent, pDirent, sizeof(SD_DIRENT));

	// Handle Filename!
#ifdef FF_UNICODE_SUPPORT
	i = wcslen(myDirent->szFileName);
	myDirent->szFileName = (wchar_t *) malloc((i + 1)*sizeof(wchar_t));
	wcscpy(myDirent->szFileName, pDirent->szFileName);
#else
	i = strlen(myDirent->szFileName);
	myDirent->szFileName = (char *) malloc(i + 1);
	strcpy(myDirent->szFileName, pDirent->szFileName);
#endif
	

	if(!Dir->pFirstEntry) {
		Dir->pFirstEntry = myDirent;
		myDirent->pNextEntry = NULL;
	} else {
		pSortDirent = Dir->pFirstEntry;
#ifdef FF_UNICODE_SUPPORT
		while(pSortDirent->pNextEntry && wcsicmp(pSortDirent->szFileName, myDirent->szFileName) < 0) {
#else
		while(pSortDirent->pNextEntry && stricmp(pSortDirent->szFileName, myDirent->szFileName) < 0) {
#endif

			pSortDirent = pSortDirent->pNextEntry;
		}

		if(pSortDirent == Dir->pFirstEntry) {
			// Sort out a first entry swap?
#ifdef FF_UNICODE_SUPPORT
			if(wcsicmp(myDirent->szFileName, pSortDirent->szFileName) < 0) {
#else
			if(stricmp(myDirent->szFileName, pSortDirent->szFileName) < 0) {
#endif
				myDirent->pNextEntry = pSortDirent;
				Dir->pFirstEntry = myDirent;
			} else {
				myDirent->pNextEntry = pSortDirent->pNextEntry;
				pSortDirent->pNextEntry = myDirent;
			}
		} else {
			myDirent->pNextEntry = pSortDirent->pNextEntry;
			pSortDirent->pNextEntry = myDirent;
		}

		
	}


	Dir->ulTotalItems += 1;
	Dir->ulTotalMemory += sizeof(SD_DIRENT) + i + 1;
	
	if((unsigned) i > Dir->ulMaxNameLength) {
		Dir->ulMaxNameLength = i;
	}

	//SD_SortDirents(Dir);

	return 0;
}


SD_ERROR SD_FindFirst(SD_DIR Dir, SD_DIRENT *pFindData) {
	if(Dir->pFirstEntry) {
		memcpy(pFindData, Dir->pFirstEntry, sizeof(SD_DIRENT));
		return 0;
	}

	return 1;
}

SD_ERROR SD_FindNext(SD_DIR Dir, SD_DIRENT *pFindData) {
	Dir = NULL;
	
	if(pFindData->pNextEntry) {
		memcpy(pFindData, pFindData->pNextEntry, sizeof(SD_DIRENT));
		return 0;
	}

	return 1;
}

unsigned long SD_GetMaxFileName(SD_DIR Dir) {
	return Dir->ulMaxNameLength;
}

unsigned long SD_GetTotalItems(SD_DIR Dir) {
	return Dir->ulTotalItems;
}
