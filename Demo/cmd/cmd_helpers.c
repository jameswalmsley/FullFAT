/*
	Provides a library to help command lines!
*/

#include <stdio.h>
#include <string.h>
#include "cmd_helpers.h"
#include "../../../ffterm/src/ffterm.h"
#include "../../src/ff_config.h"

#ifdef FF_UNICODE_SUPPORT
#include <wchar.h>
#endif

/*
	Meaningful and Pretty Error printing:
*/

void PrintError(FF_ERROR Error) {
	 printf("Error    : %08X encountered.\n", (unsigned int) Error);
	 printf("Message  : %s.\n", FF_GetErrMessage(Error));
	 printf("Module   : %s.\n", FF_GetErrModule(Error));
	 printf("Function : %s()\n", FF_GetErrFunction(Error));
}


/*
	Gets the last token of a path.
	String must be a fully qualified path e.g.:
*/
#ifdef FF_UNICODE_SUPPORT
const wchar_t *wcsGetWildcard(const wchar_t *String) {
	int i = wcslen(String);
		while(String[i] != '\\' && String[i] != '/') {
		i--;
	}

	return &String[i+1];
}
#endif
const char *GetWildcard(const char *String) {
	int i = strlen(String);

	while(String[i] != '\\' && String[i] != '/') {
		i--;
	}

	return &String[i+1];
}


/*
	Replaces
*/
#ifdef FF_UNICODE_SUPPORT
int	wcsAppendFilename(wchar_t *path, wchar_t *filename) {
	int i = wcslen(path);
	while(path[i] != '\\' && path[i] != '/') {
		i--;
		if(!i) {
			break;
		}
	}

	if(path[i] == '\\' || path[i] == '/') {
		wcscpy(&path[i+1], filename);
	} else {
		wcscpy(&path[i], filename);
	}

	return 0;
}

#endif
int	AppendFilename(char *path, char *filename) {
	int i = strlen(path);

	while(path[i] != '\\' && path[i] != '/') {
		i--;
		if(!i) {
			break;
		}
	}

	if(path[i] == '\\' || path[i] == '/') {
		strcpy(&path[i+1], filename);
	} else {
		strcpy(&path[i], filename);
	}

	return 0;
}

/**
 *	@public
 *	@brief	FullFAT Path Processor
 *
 *			Processes a path, and determines if it is relative or absolute.
 *			An absolute path is copied to dest as-is.
 *			A relative path is converted into an absolute path based on the working directory
 *			that is provided via the pEnv (FF_ENVIRONMENT object) pointer.
 *
 *	@param	dest	Where to write the processed string.
 *	@param	src		Path to be processed.
 *	@param	pEnv	Pointer to an FF_ENVIRONMENT object.
 *
 **/
#ifdef FF_UNICODE_SUPPORT
void ProcessPath(wchar_t *dest, const wchar_t *src, FF_ENVIRONMENT *pEnv) {
	if(src[0] != '\\' && src[0] != '/') {
		if(wcslen(pEnv->WorkingDir) == 1) {
			swprintf(dest, FF_MAX_PATH, L"\\%ls\0", src);
		} else {
			swprintf(dest, FF_MAX_PATH, L"%ls\\%ls\0", pEnv->WorkingDir, src);
		}
	} else {
		swprintf(dest, FF_MAX_PATH, L"%ls", src);
	}
	wcsExpandPath(dest);
}
#else
void ProcessPath(char *dest, const char *src, FF_ENVIRONMENT *pEnv) {
	if(src[0] != '\\' && src[0] != '/') {
		if(strlen(pEnv->WorkingDir) == 1) {
			sprintf(dest, "\\%s", src);
		} else {
			sprintf(dest, "%s\\%s", pEnv->WorkingDir, src);
		}
	} else {
		sprintf(dest, "%s", src);
	}

	ExpandPath(dest);	// Remove all relativity from the path.
}
#endif


/**
 *	@public
 *	@brief	A Standardised Dirent Print for FullFAT
 *
 *	Prints a Directory Entry, using a mix of styles found in Windows and Linux.
 *
 *	@param	pDirent		A pointer to an FF_DIRENT object as populated by the FF_FindFirst() or FF_FindNext() functions.
 **/
void FF_PrintDir(FF_DIRENT *pDirent) {
	unsigned char attr[5] = { '-','-','-','-', '\0' };	// String of Attribute Flags.
	if(pDirent->Attrib & FF_FAT_ATTR_READONLY)
		attr[0] = 'R';
	if(pDirent->Attrib & FF_FAT_ATTR_HIDDEN)
		attr[1] = 'H';
	if(pDirent->Attrib & FF_FAT_ATTR_SYSTEM)
		attr[2] = 'S';
	if(pDirent->Attrib & FF_FAT_ATTR_DIR)
		attr[3] = 'D';
#ifdef FF_TIME_SUPPORT	// Different Print formats dependent on if Time support is built-in.
#ifdef FF_UNICODE_SUPPORT
	printf("%02d.%02d.%02d  %02d:%02d  %s  %12lu  %ls\n", pDirent->CreateTime.Day, pDirent->CreateTime.Month, pDirent->CreateTime.Year, pDirent->CreateTime.Hour, pDirent->CreateTime.Minute, attr, pDirent->Filesize, pDirent->FileName);
#else
	printf("%02d.%02d.%02d  %02d:%02d  %s  %12lu  %s\n", pDirent->CreateTime.Day, pDirent->CreateTime.Month, pDirent->CreateTime.Year, pDirent->CreateTime.Hour, pDirent->CreateTime.Minute, attr, pDirent->Filesize, pDirent->FileName);
#endif
#else
#ifdef FF_UNICODE_SUPPORT
	printf(" %s %12lu    %ls\n", attr, pDirent->Filesize, pDirent->FileName);
#else
	printf(" %s %12lu    %s\n", attr, pDirent->Filesize, pDirent->FileName);
#endif
#endif
}

void SD_PrintDirent(SD_DIRENT *pDirent, SD_SIZEUNIT eUnit, char cForcedBytes, FF_ENVIRONMENT *pEnv) {
	unsigned char attr[5] = { '-','-','-','-', '\0' };	// String of Attribute Flags.
	if(pDirent->ulAttributes & SD_ATTRIBUTE_RDONLY)
		attr[0] = 'R';
	if(pDirent->ulAttributes & SD_ATTRIBUTE_HIDDEN)
		attr[1] = 'H';
	if(pDirent->ulAttributes & SD_ATTRIBUTE_SYSTEM)
		attr[2] = 'S';
	if(pDirent->ulAttributes & SD_ATTRIBUTE_DIR) {
		attr[3] = 'D';
	}

	
	printf("%02d.%02d.%02d  %02d:%02d  %s ", pDirent->tmCreated.cDay, pDirent->tmCreated.cMonth, pDirent->tmCreated.iYear, pDirent->tmCreated.cHour, pDirent->tmCreated.cMinute, attr /*, pDirent->ulFileSize*/);

	switch(eUnit) {
		case SD_TERABYTES:
			printf("%7llu ", (unsigned long long)((unsigned long long)pDirent->ulFileSize / (unsigned long long)((unsigned long long)1024*1024*1024*1024)));
			printf("TB");
			break;

		case SD_GIGABYTES:
			printf("%7.2f ", (float)((float)pDirent->ulFileSize / (1024.0*1024.0*1024.0)));
			printf("GB");
			break;

		case SD_MEGABYTES:
			printf("%7.2f ", (float)((float)pDirent->ulFileSize / (1024.0*1024.0)));
			printf("MB");
			break;

		case SD_KILOBYTES:
			printf("%7.2f ", (float)((float)pDirent->ulFileSize / (1024.0)));
			printf("KB");
			break;

	case SD_BYTES:
		printf("bytes\n");
		default:
			if(cForcedBytes) {
				printf("%12lu ", (unsigned long) pDirent->ulFileSize);
			} else {
				printf("%7lu ", (unsigned long) pDirent->ulFileSize);
				printf("B ");
			}
			break;
	}
	
	if(pDirent->ulAttributes & SD_ATTRIBUTE_DIR) {
		FFTerm_SetConsoleColour(pEnv->pConsole, DIR_COLOUR | FFT_FOREGROUND_INTENSITY);
	}
	
#ifdef FF_UNICODE_SUPPORT
	printf("  %ls\n",  pDirent->szFileName);
#else
	printf("  %s\n",  pDirent->szFileName);
#endif
	
	FFTerm_SetConsoleColour(pEnv->pConsole, FFT_FOREGROUND_GREY);
}

/*
	This routine removes all relative ..\ from a path.
	It's probably not the best, but it works. 

	Its based on some old code I wrote a long time ago.
*/

#ifdef FF_UNICODE_SUPPORT
void wcsExpandPath(wchar_t *acPath) {
	wchar_t *pRel 		= 0;
	wchar_t	*pRelStart 	= 0;
	wchar_t *pRelEnd 	= 0;

	int		charRef 	= 0;
	int 	lenPath 	= 0;
	int		lenRel		= 0;
	int 	i 			= 0;
	int 	remain 		= 0;

	lenPath = wcslen(acPath);
	pRel = wcsstr(acPath, L"..");

	while(pRel) {	// Loop removal of Relativity
		charRef = pRel - acPath;

		/*
			We have found some relativity in the Path, 
		*/

		// Where it ends:
		
		if(pRel[2] == '\\' || pRel[2] == '/') {
			pRelEnd = pRel + 3;
		} else {
			pRelEnd = pRel + 2;	
		}
		
		// Where it Starts:
		
		if(charRef == 1) {	// Relative Path comes after the root /
			return;	// Fixed, returns false appropriately, as in the TODO: above!
		} else {
			for(i = (charRef - 2); i >= 0; i--) {
				if(acPath[i] == '\\' || acPath[i] == '/') {
					pRelStart = (acPath + (i + 1));
					break;
				}
			}
		}
		
		// The length of the relativity
		lenRel = pRelEnd - pRelStart;
		
		remain = lenPath - (pRelEnd - acPath);	// Remaining Chars on the end of the path
		
		if(lenRel) {
			wcsncpy(pRelStart, pRelEnd, remain);
			pRelStart[remain] = '\0';
		}
		
		lenPath -= lenRel;
		pRel = wcsstr(acPath, L"..");
	}
}
#endif

void ExpandPath(char *acPath) {

	char 	*pRel 		= 0;
	char	*pRelStart 	= 0;
	char 	*pRelEnd 	= 0;

	int		charRef 	= 0;
	int 	lenPath 	= 0;
	int		lenRel		= 0;
	int 	i 			= 0;
	int 	remain 		= 0;
	

	lenPath = strlen(acPath);
	pRel = strstr(acPath, "..");

	while(pRel) {	// Loop removal of Relativity
		charRef = pRel - acPath;

		/*
			We have found some relativity in the Path, 
		*/

		// Where it ends:
		
		if(pRel[2] == '\\' || pRel[2] == '/') {
			pRelEnd = pRel + 3;
		} else {
			pRelEnd = pRel + 2;	
		}
		
		// Where it Starts:
		
		if(charRef == 1) {	// Relative Path comes after the root /
			return;	// Fixed, returns false appropriately, as in the TODO: above!
		} else {
			for(i = (charRef - 2); i >= 0; i--) {
				if(acPath[i] == '\\' || acPath[i] == '/') {
					pRelStart = (acPath + (i + 1));
					break;
				}
			}
		}
		
		// The length of the relativity
		lenRel = pRelEnd - pRelStart;
		
		remain = lenPath - (pRelEnd - acPath);	// Remaining Chars on the end of the path
		
		if(lenRel) {

			strncpy(pRelStart, pRelEnd, remain);
			pRelStart[remain] = '\0';
		}
		
		lenPath -= lenRel;
		pRel = strstr(acPath, "..");
	}
}



/*
	FindFirstFile() / FindNextFile() wrapper for Linux opendir() etc.
*/

#ifndef WIN32
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <limits.h>

void ProcessLinuxPath(char *dest, const char *src) {
	
	char path[FF_MAX_PATH + 1];

	getcwd(path, FF_MAX_PATH);
	
	if(src[0] != '\\' && src[0] != '/') {
		if(strlen(path) == 1) {
			sprintf(dest, "/%s", src);
		} else {
			sprintf(dest, "%s/%s", path, src);
		}
	} else {
		sprintf(dest, "%s", src);
	}
}

static int wildcompare(const char *pszWildCard, const char *pszString) {
    register const char *pszWc 	= NULL;
	register const char *pszStr 	= NULL;	// Encourage the string pointers to be placed in memory.
    do {
        if ( *pszWildCard == '*' ) {
			while(*(1 + pszWildCard++) == '*'); // Eat up multiple '*''s
			pszWc = (pszWildCard - 1);
            pszStr = pszString;
        }
		if (*pszWildCard == '?' && !*pszString) {
			return 0;	// False when the string is ended, yet a ? charachter is demanded.
		}
//#ifdef FF_WILDCARD_CASE_INSENSITIVE
        if (*pszWildCard != '?' && tolower(*pszWildCard) != tolower(*pszString)) {
//#else
//		if (*pszWildCard != '?' && *pszWildCard != *pszString) {
//#endif
			if (pszWc == NULL) {
				return 0;
			}
            pszWildCard = pszWc;
            pszString = pszStr++;
        }
    } while ( *pszWildCard++ && *pszString++ );

	while(*pszWildCard == '*') {
		pszWildCard++;
	}

	if(!*(pszWildCard - 1)) {	// WildCard is at the end. (Terminated)
		return 1;	// Therefore this must be a match.
	}

	return 0;	// If not, then return FF_FALSE!
}

DIR	*FindFirstFile(const char *szpPath, DIRENT *pFindData) {
	DIR 			*pDir;
	struct dirent 	*pDirent;
	const char		*szpWildCard;
	char			path[FF_MAX_PATH];

	strcpy(pFindData->szWildCard, "");
	szpWildCard = GetWildcard(szpPath);
	strcpy(pFindData->szWildCard, szpWildCard);

	strncpy(path, szpPath, (szpWildCard - szpPath));
	path[(szpWildCard - szpPath)] = '\0';

	pDir = opendir(path);

	if(!pDir) {
		return NULL;
	}

	pDirent = readdir(pDir);

	strcpy(pFindData->szItemPath, path);
	
	while(pDirent) {
		if(!strcmp(pFindData->szWildCard, "")) {
			AppendFilename(pFindData->szItemPath, pDirent->d_name);
			lstat(pFindData->szItemPath, &pFindData->itemInfo);
			memcpy(&pFindData->dir, pDirent, sizeof(struct dirent));
			return pDir;
		}
		
		if(pDirent->d_name[0] == 'P') {
			printf(" ");
		}

		if(wildcompare(szpWildCard, pDirent->d_name)) {
			AppendFilename(pFindData->szItemPath, pDirent->d_name);
			lstat(pFindData->szItemPath, &pFindData->itemInfo);
			memcpy(&pFindData->dir, pDirent, sizeof(struct dirent));
			return pDir;			
		}
		pDirent = readdir(pDir);
	}

	closedir(pDir);

	return NULL;
}

int	FindNextFile(DIR *pDir, DIRENT *pFindData) {
	struct dirent *pDirent = readdir(pDir);

	while(pDirent) {
		if(!strcmp(pFindData->szWildCard, "")) {
			AppendFilename(pFindData->szItemPath, pDirent->d_name);
			lstat(pFindData->szItemPath, &pFindData->itemInfo);
			memcpy(&pFindData->dir, pDirent, sizeof(struct dirent));
			return 1;
		}		
		if(wildcompare(pFindData->szWildCard, pDirent->d_name)) {
			AppendFilename(pFindData->szItemPath, pDirent->d_name);
			lstat(pFindData->szItemPath, &pFindData->itemInfo);
			memcpy(&pFindData->dir, pDirent, sizeof(struct dirent));
			return 1;			
		}
		pDirent = readdir(pDir);
	}

	return 0;
}

int FindClose(DIR *pDir) {
	return closedir(pDir);
}

#endif


