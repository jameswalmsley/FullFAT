/*****************************************************************************
 *  FullFAT - High Performance, Thread-Safe Embedded FAT File-System         *
 *                                                                           *
 *  Copyright(C) 2009  James Walmsley  (james@fullfat-fs.co.uk)              *
 *  Many Thanks to     Hein Tibosch    (hein_tibosch@yahoo.es)               *
 *                                                                           *
 *  See RESTRICTIONS.TXT for extra restrictions on the use of FullFAT.       *
 *                                                                           *
 *                FULLFAT IS NOT FREE FOR COMMERCIAL USE                     *
 *                                                                           *
 *  Removing this notice is illegal and will invalidate this license.        *
 *****************************************************************************
 *  See http://www.fullfat-fs.co.uk/ for more information.                   *
 *  Or  http://fullfat.googlecode.com/ for latest releases and the wiki.     *
 *****************************************************************************
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
 *****************************************************************************/

#include "ls_cmd.h"

//static void transferdatetime(FF_DIRENT *pSource, SD_DIRENT *pDest);
#ifdef WIN32

#else
#define wcsicmp wcscasecmp
#endif

typedef struct {
	 int bShowHidden, bList, bRecursive, bHumanReadable;
} LS_OPTIONS;

static void ls_printDirent(SD_DIRENT *pDirent, LS_OPTIONS *poOptions, FF_ENVIRONMENT *pEnv);

#ifdef FF_UNICODE_SUPPORT
static int ls_dir(const wchar_t *szPath, LS_OPTIONS *poOptions, FF_ENVIRONMENT *pEnv);
#else
static int ls_dir(const char *szPath, LS_OPTIONS *poOptions, FF_ENVIRONMENT *pEnv);
#endif

int ls_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
#ifdef FF_UNICODE_SUPPORT
	FF_T_WCHAR			path[FF_MAX_PATH];
	FF_T_WCHAR			wcArgv[FF_MAX_PATH];
#else
	FF_T_INT8			path[FF_MAX_PATH];
#endif
	
	int RetVal;
	int option;

	const char 			*szPath;
	FFT_GETOPT_CONTEXT	optionContext;
	LS_OPTIONS			oOptions;

	oOptions.bRecursive	 	= 0;		// Initialise option flags with default values.
	oOptions.bList				= 0;
	oOptions.bShowHidden		= 0;
	oOptions.bHumanReadable = 0;

	memset(&optionContext, 0, sizeof(FFT_GETOPT_CONTEXT));

	// Process command line arguments

	option = FFTerm_getopt(argc, (const char **) argv, "rRlLaAhH", &optionContext);
	
	if(option != EOF) {
		do {
			switch(option) {
				case 'r':
				case 'R':
					oOptions.bRecursive = FF_TRUE;
					break;

				case 'l':
				case 'L':
					oOptions.bList = FF_TRUE;
					break;

				case 'a':
				case 'A':
					oOptions.bShowHidden = FF_TRUE;
					break;

				case 'h':
				case 'H':
					oOptions.bHumanReadable = FF_TRUE;
					break;

				default:
					break;
			}

			option = FFTerm_getopt(argc, (const char **) argv, "rRlLaAhH", &optionContext);
		} while(option != EOF);
	}

	szPath = FFTerm_getarg(argc, (const char **) argv, 0, &optionContext);

	if(szPath) {
#ifdef FF_UNICODE_SUPPORT
		FF_cstrtowcs(wcArgv, szPath);
		ProcessPath(path, wcArgv, pEnv);
#else
		ProcessPath(path, szPath, pEnv);
#endif

		RetVal = ls_dir(path, &oOptions, pEnv);
	} else {

		 RetVal = ls_dir(pEnv->WorkingDir, &oOptions, pEnv);

		 if(RetVal == -5) { // Not Found!
			  if(szPath) {
					printf("%s: cannot access %s: no such file or directory\n", argv[0], szPath);
			  }
		 }
	}
	return 0;
}
const FFT_ERR_TABLE lsInfo[] =
{
	{"Unknown or Generic Error",		-1},							// Generic Error (always the first entry).
	{"Lists the contents of the current working directory.",			FFT_COMMAND_DESCRIPTION},
	{ NULL }
};

/*
	This function copies time and date info from FullFAT dirents, to 
	the Sorted Dir dirents. (Simply it allows the LS command to provide a sorted
	list).
*/
static void transferdatetime(FF_DIRENT *pSource, SD_DIRENT *pDest) {
#ifdef FF_TIME_SUPPORT
	pDest->tmCreated.cDay = (unsigned char) pSource->CreateTime.Day;
	pDest->tmCreated.cMonth = (unsigned char) pSource->CreateTime.Month;
	pDest->tmCreated.iYear = (int) pSource->CreateTime.Year;
	pDest->tmCreated.cHour = (unsigned char) pSource->CreateTime.Hour;
	pDest->tmCreated.cMinute = (unsigned char) pSource->CreateTime.Minute;
	pDest->tmCreated.cSecond = (unsigned char) pSource->CreateTime.Second;

	pDest->tmLastAccessed.cDay = (unsigned char) pSource->AccessedTime.Day;
	pDest->tmLastAccessed.cMonth = (unsigned char)pSource->AccessedTime.Month;
	pDest->tmLastAccessed.iYear = pSource->AccessedTime.Year;
	pDest->tmLastAccessed.cHour = (unsigned char) pSource->AccessedTime.Hour;
	pDest->tmLastAccessed.cMinute = (unsigned char) pSource->AccessedTime.Minute;
	pDest->tmLastAccessed.cSecond = (unsigned char) pSource->AccessedTime.Second;

	pDest->tmCreated.cDay = (unsigned char) pSource->CreateTime.Day;
	pDest->tmCreated.cMonth = (unsigned char) pSource->CreateTime.Month;
	pDest->tmCreated.iYear = pSource->CreateTime.Year;
	pDest->tmCreated.cHour = (unsigned char) pSource->CreateTime.Hour;
	pDest->tmCreated.cMinute = (unsigned char) pSource->CreateTime.Minute;
	pDest->tmCreated.cSecond = (unsigned char) pSource->CreateTime.Second;
#else

#endif
}

/*
	This function simply lists an entire directory, with specified wildCard.
*/
#ifdef FF_UNICODE_SUPPORT
static int ls_dir(const wchar_t *szPath, LS_OPTIONS *poOptions, FF_ENVIRONMENT *pEnv) {
#else
static int ls_dir(const char *szPath, LS_OPTIONS *poOptions, FF_ENVIRONMENT *pEnv) {
#endif
	FF_DIRENT	findData;
	FF_ERROR	Result;
	SD_ERROR	RetVal; 
	SD_DIR		Dir;
	SD_DIRENT	Dirent;
#ifdef FF_UNICODE_SUPPORT
	wchar_t			path[FF_MAX_PATH];
	//wchar_t			name[FF_MAX_FILENAME];
	wchar_t			recursivePath[FF_MAX_PATH];
	const wchar_t	*szpWildCard;
#else
	char		path[FF_MAX_PATH];
//	char		name[FF_MAX_FILENAME];
	char		recursivePath[FF_MAX_PATH];
	const char	*szpWildCard;
#endif
	
	int columns, columnWidth;
	int i;

#ifdef FF_UNICODE_SUPPORT
	wcscpy(path, szPath);	// Place szPath into a modifiable buffer so we can correctly format it.
#else
	strcpy(path, szPath);	// Place szPath into a modifiable buffer so we can correctly format it.
#endif

	// First Pass to calculate column widths!
	Result = FF_FindFirst(pEnv->pIoman, &findData, path);

	if(Result) {
		return -5; // No dirs;
	}

#ifdef FF_UNICODE_SUPPORT	
	szpWildCard = wcsGetWildcard(szPath);
#else

	szpWildCard = GetWildcard(szPath);

#endif

	// A directory should be opened with /path/to/dir/ or /path/to/dir/*, not /path/to/dir
	// Check if entry is a dir, and the wildCard specified a specific dir.
	// If so we should open that dir for iteration.


#ifdef FF_UNICODE_SUPPORT

	if(!wcsicmp(findData.FileName, szpWildCard) && (findData.Attrib & FF_FAT_ATTR_DIR)) {
		wcscat(path, L"\\*");	// Add a backslash to the end!
#else
	if(!FF_stricmp(findData.FileName, szpWildCard) && (findData.Attrib & FF_FAT_ATTR_DIR)) {
		strcat(path, "\\*");	// Add a backslash to the end!
#endif
		//strcpy(recursivePath, szPath);	// Copy szPath, to recursivePath so \* can be added.

		Result = FF_FindFirst(pEnv->pIoman, &findData, path);
		if(Result) {
			return -5;
		}
	}

	// Create the Directory object.
	Dir = SD_CreateDir();

	// Feed in the directory data!

	do {
		Dirent.szFileName = findData.FileName;
		Dirent.ulFileSize = findData.Filesize;
		
		// Created Time Stamp!
		transferdatetime(&findData, &Dirent);
		
		Dirent.ulAttributes = 0;

		// Process the attributes!
		if(findData.Attrib & FF_FAT_ATTR_READONLY)
			Dirent.ulAttributes |= SD_ATTRIBUTE_RDONLY;
		if(findData.Attrib & FF_FAT_ATTR_HIDDEN)
			Dirent.ulAttributes |= SD_ATTRIBUTE_HIDDEN;
		if(findData.Attrib & FF_FAT_ATTR_SYSTEM)
			Dirent.ulAttributes |= SD_ATTRIBUTE_SYSTEM;
		if(findData.Attrib & FF_FAT_ATTR_DIR)
			Dirent.ulAttributes |= SD_ATTRIBUTE_DIR;
		if(findData.Attrib & FF_FAT_ATTR_ARCHIVE)
			Dirent.ulAttributes |= SD_ATTRIBUTE_ARCHIVE;

		SD_AddDirent(Dir, &Dirent);

		Result = FF_FindNext(pEnv->pIoman, &findData);
	} while(!Result);

	// Second Pass to print the columns nicely.

	columns = (FFTerm_GetConsoleWidth(pEnv->pConsole)-1) / (SD_GetMaxFileName(Dir) + 1);

	if(columns > 5) {
		columns = 5;
	}

	if(!columns) {
		columns = 1;
	}

	columnWidth = (FFTerm_GetConsoleWidth(pEnv->pConsole)-1)/ columns;
	
	if(!columns) {
		columns++;
	}

	RetVal = SD_FindFirst(Dir, &Dirent);

	if(!poOptions->bList) {
		do {
			for(i = 0; i < columns; i++) {
				if(poOptions->bShowHidden) {
					if(Dirent.ulAttributes & SD_ATTRIBUTE_DIR) {
						FFTerm_SetConsoleColour(pEnv->pConsole, DIR_COLOUR | FFT_FOREGROUND_INTENSITY);
					}
#ifdef FF_UNICODE_SUPPORT
					printf("%-*ls", columnWidth, Dirent.szFileName);
#else
					printf("%-*s", columnWidth, Dirent.szFileName);
#endif
				} else {
					if(Dirent.ulAttributes & SD_ATTRIBUTE_DIR) {
						FFTerm_SetConsoleColour(pEnv->pConsole, DIR_COLOUR | FFT_FOREGROUND_INTENSITY);
					}
					if(Dirent.szFileName[0] != '.' && !(Dirent.ulAttributes & SD_ATTRIBUTE_HIDDEN)) {
#ifdef FF_UNICODE_SUPPORT
						printf("%-*ls", columnWidth, Dirent.szFileName);
#else
						printf("%-*s", columnWidth, Dirent.szFileName);
#endif
					} else {
						i--;	// Make sure hidden items don't cause unequal wrapping!
					}
				}
				
				FFTerm_SetConsoleColour(pEnv->pConsole, FFT_FOREGROUND_GREY);
				RetVal = SD_FindNext(Dir, &Dirent);
				if(RetVal) {
					break;
				}
			}
			if(!RetVal) {
				printf("\n");
			}
		} while(!RetVal);

	} else {

		do {
			if(poOptions->bShowHidden) {
				if(!(poOptions->bRecursive && (Dirent.ulAttributes & SD_ATTRIBUTE_DIR))) {
					ls_printDirent(&Dirent, poOptions, pEnv);
				}
			} else {
									
				if(Dirent.szFileName[0] != '.' && !(Dirent.ulAttributes & SD_ATTRIBUTE_HIDDEN)) {
					if(!(poOptions->bRecursive && (Dirent.ulAttributes & SD_ATTRIBUTE_DIR))) {
						 ls_printDirent(&Dirent, poOptions, pEnv);
					}
				}
			}
			RetVal = SD_FindNext(Dir, &Dirent);
		} while(!RetVal);
	}
	printf("\n");
	
	printf("%lu items.\n", SD_GetTotalItems(Dir));

	if(poOptions->bRecursive) {
		RetVal = SD_FindFirst(Dir, &Dirent);

		do {
			if(Dirent.ulAttributes & SD_ATTRIBUTE_DIR) {
				if(!(Dirent.szFileName[0] == '.' && (Dirent.szFileName[1] == '.' || Dirent.szFileName[1] == '\0'))) {
										
#ifdef FF_UNICODE_SUPPORT
					wcscpy(recursivePath, path);
					wcsAppendFilename(recursivePath, Dirent.szFileName);

#else
					strcpy(recursivePath, path);
					AppendFilename(recursivePath, Dirent.szFileName);
#endif


#ifdef FF_UNICODE_SUPPORT
					if(recursivePath[wcslen(recursivePath) - 1] != '\\' && recursivePath[wcslen(recursivePath) - 1] != '/') {
						wcscat(recursivePath, L"\\");
					}
					
					printf(".%ls:\n", recursivePath);
#else
					if(recursivePath[strlen(recursivePath) - 1] != '\\' && recursivePath[strlen(recursivePath) - 1] != '/') {
						strcat(recursivePath, "\\");
					}
					
					printf(".%s:\n", recursivePath);
#endif
					ls_dir(recursivePath, poOptions, pEnv);
				}
			}
			
			RetVal = SD_FindNext(Dir, &Dirent);
		} while(!RetVal);
	}
	
	SD_CleanupDir(Dir);	// Cleanup the directory!

	return 0;
}

void ls_printDirent(SD_DIRENT *pDirent, LS_OPTIONS *poOptions, FF_ENVIRONMENT *pEnv) {
	
	SD_SIZEUNIT eUnit = SD_BYTES;	// Default to bytes.

	if(pDirent->ulFileSize >= 1024) {	// Decide which size unit to print?
		eUnit = SD_KILOBYTES;
	}
	
	if(pDirent->ulFileSize >= (1024*1024)) { // 1MB
		eUnit = SD_MEGABYTES;
	}

	if(pDirent->ulFileSize >= (1024*1024*1024)) {
		eUnit = SD_GIGABYTES;
	}

	if((unsigned long long) pDirent->ulFileSize >= (unsigned long long)((unsigned long long)1024*1024*1024*1024)) {
		eUnit = SD_TERABYTES;
	}

	if(!poOptions->bHumanReadable) {
		eUnit = SD_BYTES;
	}

	SD_PrintDirent(pDirent, eUnit, !(poOptions->bHumanReadable), pEnv);
}
