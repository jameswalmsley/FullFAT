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
#include "ls_cmd.h"

static void transferdatetime(FF_DIRENT *pSource, SD_DIRENT *pDest);
static int ls_dir(const char *szPath, FF_T_BOOL bList, FF_T_BOOL bRecursive, FF_T_BOOL bShowHidden, FF_ENVIRONMENT *pEnv);

int ls_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	FF_T_INT8			path[FF_MAX_PATH];
	const char 			*szPath;
	FF_T_BOOL			bRecursive = FF_FALSE, bList = FF_FALSE, bShowHidden = FF_FALSE;
	FFT_GETOPT_CONTEXT	optionContext;

	int RetVal;

	int option;

	memset(&optionContext, 0, sizeof(FFT_GETOPT_CONTEXT));

	// Process command line arguments

	option = FFTerm_getopt(argc, argv, "rRlLaA", &optionContext);

	if(option != EOF) {
		do {
			switch(option) {
				case 'r':
				case 'R':
					bRecursive = FF_TRUE;
					break;

				case 'l':
				case 'L':
					bList = FF_TRUE;
					break;

				case 'a':
				case 'A':
					bShowHidden = FF_TRUE;
					break;

				default:
					break;
			}

			option = FFTerm_getopt(argc, argv, "rRlLaA", &optionContext);
		} while(option != EOF);
	}

	szPath = FFTerm_getarg(argc, argv, 0, &optionContext);

	if(szPath) {
		ProcessPath(path, szPath, pEnv);
		RetVal = ls_dir(path, bList, bRecursive, bShowHidden, pEnv);
	} else {
		RetVal = ls_dir(pEnv->WorkingDir, bList, bRecursive, bShowHidden, pEnv);
	}

	if(RetVal == -5) { // Not FounD!
		if(szPath) {
			printf("%s: cannot access %s: no such file or directory\n", argv[0], szPath);
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
}

/*
	This function simply lists an entire directory, with specified wildCard.
*/
static int ls_dir(const char *szPath, FF_T_BOOL bList, FF_T_BOOL bRecursive, FF_T_BOOL bShowHidden, FF_ENVIRONMENT *pEnv) {
	FF_DIRENT	findData;
	FF_ERROR	Result;
	SD_ERROR	RetVal;
	SD_DIR		Dir;
	SD_DIRENT	Dirent;
	char		path[FF_MAX_PATH];
	char		name[FF_MAX_FILENAME];
	FF_T_WCHAR	wcPath[FF_MAX_PATH];
	FF_T_WCHAR	wcName[FF_MAX_FILENAME];
	char		recursivePath[FF_MAX_PATH];

	const char	*szpWildCard;
	
	int columns, columnWidth;
	int i;
	
	strcpy(path, szPath);	// Place szPath into a modifiable buffer so we can correctly format it.

	// First Pass to calculate column widths!
	FF_cstrtowcs(wcPath, path);
	Result = FF_FindFirst(pEnv->pIoman, &findData, wcPath);

	if(Result) {
		return -5; // No dirs;
	}

	szpWildCard = getWildcard(szPath);

	// A directory should be opened with /path/to/dir/ or /path/to/dir/*, not /path/to/dir
	// Check if entry is a dir, and the wildCard specified a specific dir.
	// If so we should open that dir for iteration.
#ifdef FF_UNICODE_SUPPORT
	FF_cstrtowcs(wcName, szpWildCard);
	if(!wcsicmp(findData.FileName, wcName) && (findData.Attrib & FF_FAT_ATTR_DIR)) {
#else
	if(!stricmp(findData.FileName, szpWildCard) && (findData.Attrib & FF_FAT_ATTR_DIR)) {
#endif
		//strcpy(recursivePath, szPath);	// Copy szPath, to recursivePath so \* can be added.
		strcat(path, "\\*");	// Add a backslash to the end!
		Result = FF_FindFirst(pEnv->pIoman, &findData, wcPath);
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

	columns = (FFTerm_GetConsoleWidth()-1) / (SD_GetMaxFileName(Dir) + 1);

	if(columns > 5) {
		columns = 5;
	}

	columnWidth = (FFTerm_GetConsoleWidth()-1)/ columns;
	
	if(!columns) {
		columns++;
	}

	RetVal = SD_FindFirst(Dir, &Dirent);

	if(!bList) {
		do {
			for(i = 0; i < columns; i++) {
				if(bShowHidden) {
					if(Dirent.ulAttributes & SD_ATTRIBUTE_DIR) {
						FFTerm_SetConsoleColour(DIR_COLOUR | FFT_FOREGROUND_INTENSITY);
					}
#ifdef FF_UNICODE_SUPPORT
					wprintf(L"%-*s", columnWidth, Dirent.szFileName);
#else
					printf("%-*s", columnWidth, Dirent.szFileName);
#endif
				} else {
					if(Dirent.ulAttributes & SD_ATTRIBUTE_DIR) {
						FFTerm_SetConsoleColour(DIR_COLOUR | FFT_FOREGROUND_INTENSITY);
					}
					if(Dirent.szFileName[0] != '.' && !(Dirent.ulAttributes & SD_ATTRIBUTE_HIDDEN)) {
#ifdef FF_UNICODE_SUPPORT
						wprintf(L"%-*s", columnWidth, Dirent.szFileName);
#else
						printf("%-*s", columnWidth, Dirent.szFileName);
#endif
					} else {
						i--;	// Make sure hidden items don't cause unequal wrapping!
					}
				}
				
				FFTerm_SetConsoleColour(FFT_FOREGROUND_GREY);
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
			if(bShowHidden) {
				if(!(bRecursive && (Dirent.ulAttributes & SD_ATTRIBUTE_DIR))) {
					SD_PrintDirent(&Dirent);
				}
			} else {
									
				if(Dirent.szFileName[0] != '.' && !(Dirent.ulAttributes & SD_ATTRIBUTE_HIDDEN)) {
					if(!(bRecursive && (Dirent.ulAttributes & SD_ATTRIBUTE_DIR))) {
						SD_PrintDirent(&Dirent);
					}
				}
			}
			RetVal = SD_FindNext(Dir, &Dirent);
		} while(!RetVal);
	}
	printf("\n");
	
	printf("%lu items.\n", SD_GetTotalItems(Dir));

	if(bRecursive) {
		RetVal = SD_FindFirst(Dir, &Dirent);

		do {
			if(Dirent.ulAttributes & SD_ATTRIBUTE_DIR) {
				if(!(Dirent.szFileName[0] == '.' && (Dirent.szFileName[1] == '.' || Dirent.szFileName[1] == '\0'))) {
										
					strcpy(recursivePath, path);

					append_filename(recursivePath, Dirent.szFileName);

					if(recursivePath[strlen(recursivePath) - 1] != '\\' && recursivePath[strlen(recursivePath) - 1] != '/') {
						strcat(recursivePath, "\\");
					}
					
					printf(".%s:\n", recursivePath);
					ls_dir(recursivePath, bList, bRecursive, bShowHidden, pEnv);
				}
			}
			
			RetVal = SD_FindNext(Dir, &Dirent);
		} while(!RetVal);
	}
	
	SD_CleanupDir(Dir);	// Cleanup the directory!

	return 0;
}
