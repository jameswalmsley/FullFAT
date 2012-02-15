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
#include "md5sum_win_cmd.h"
#include <windows.h>
/*
	This provides an md5sum for Windows, that works consistently
	as the FullFAT version does.
*/

static int md5_getHash(const char *path, char *szpHash);
static int md5_checkHash(const char *path);

/**
 *	@public
 *	@brief	MD5 Data Hashing function.
 *
 *	Generates and displays an MD5 hash of a file. This is really useful when
 *	verify files for their integrity. We used MD5 extensively while stabilising
 *	the read and write functionality of FullFAT.
 *
 **/
int md5sum_win_cmd(int argc, char **argv) {
	int option;
	FFT_GETOPT_CONTEXT	optionContext;
	FF_T_BOOL			bBinary = FF_FALSE, bCheck = FF_FALSE, bWarn = FF_FALSE;
	WIN32_FIND_DATAA	findData;
	HANDLE				hSearch;
	FILE				*pfOut;

	//FF_ERROR			RetVal, ffError;
	BOOL				RetVal;
	char				path[FF_MAX_PATH];
	char				outfilepath[FF_MAX_PATH];
	const char			*szargPath;
	char				hash[33];

	const char			*szargOutputFile = NULL;
	int i;

	memset(&optionContext, 0, sizeof(FFT_GETOPT_CONTEXT));

	option = FFTerm_getopt(argc, argv, "bctwo:", &optionContext);

	if(option != EOF) {
		do {
			switch(option) {
				case 'b':
					bBinary = FF_TRUE;
					break;
				case 'c':
					bCheck = FF_TRUE;
					break;

				case 't':
					bBinary = FF_FALSE;
					break;

				case 'w':
					bWarn = FF_TRUE;
					break;

				case 'o':
					szargOutputFile = optionContext.optarg;
					break;
			}

			option = FFTerm_getopt(argc, argv, "bctw", &optionContext);
		} while(option != EOF);
	}

	szargPath = FFTerm_getarg(argc, argv, 0, &optionContext);	// Get the available non-option arguments remaining.
		
	if(szargPath) {
		//ProcessLinuxPath(path, szargPath);
		
		//RetVal = FF_FindFirst(pEnv->pIoman, &findData, path);
		

		if(szargOutputFile) {
			//ProcessLinuxPath(outfilepath, szargOutputFile);
			//pfOut = FF_Open(pEnv->pIoman, outfilepath, FF_MODE_WRITE | FF_MODE_TRUNCATE | FF_MODE_CREATE, &ffError);
			strcpy(outfilepath, szargOutputFile);
			pfOut = fopen(outfilepath, "w");
			if(!pfOut) {
				printf("%s: Error opening %s for writing.\n", argv[0], szargOutputFile);
				return 0;
			}
		}

		strcpy(path, szargPath);
		if(path[strlen(path) - 1] == '\\' || path[strlen(path) - 1] == '\\') {
			strcat(path, "*");
		}
		hSearch = FindFirstFileA(path, &findData);

		if(hSearch == INVALID_HANDLE_VALUE) {
			printf("%s: %s: No such file or directory.\n", argv[0], szargPath);
			return 0;
		}

		do {
			if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				printf("%s: %s: Is a directory\n", argv[0], findData.cFileName);
			} else {
				AppendFilename(path, findData.cFileName);
				if(!bCheck) {
					if(!md5_getHash(path, hash)) {
						printf("%s  %s\n", hash, findData.cFileName);
					}

					if(szargOutputFile && strcmp(szargOutputFile, findData.cFileName)) {
						for(i = 0; i < 32; i++) {
							//FF_PutC(pfOut, hash[i]);
							fputc(hash[i], pfOut);
						}
						//FF_PutC(pfOut, ' ');
						//FF_PutC(pfOut, ' ');
						fputc(' ', pfOut);
						fputc(' ', pfOut);
						for(i = 0; findData.cFileName[i] != '\0'; i++) {
							//FF_PutC(pfOut, findData.FileName[i]);
							fputc(findData.cFileName[i], pfOut);
						}
						//FF_PutC(pfOut, '\n'); // Nextline!
						fputc('\n', pfOut);
					}

				} else {
					md5_checkHash(path);
				}
			}
			//RetVal = FF_FindNext(pEnv->pIoman, &findData);
			RetVal = FindNextFileA(hSearch, &findData);
			//RetVal = FindNextFile(pDir, &findData);
		} while(RetVal);

		if(szargOutputFile) {
			//FF_Close(pfOut);
			fclose(pfOut);
		}

		if(hSearch) {
			FindClose(hSearch);
		}

		return 0;
	}
	
	printf("See usage! -- Please!\n");	

	return 0;
}
const FFT_ERR_TABLE md5sum_win_Info[] =
{	
	{"Unknown or Generic Error",		-1},							// Generic Error (always the first entry).
	{"Calculates an MD5 checksum for the specified file.",			FFT_COMMAND_DESCRIPTION},
	{ NULL }
};



static int md5_getHash(const char *path, char *szpHash) {
	FF_T_UINT8			readBuf[8192];
	FILE				*pFile;
	//FF_ERROR			ffError;

	int					len;
	md5_state_t 		state;
	md5_byte_t			digest[16];
	char				temp[3];
	int					di;

	//pFile = FF_Open(pEnv->pIoman, path, FF_MODE_READ, &ffError);
	pFile = fopen(path, "rb");

	if(pFile) {
		md5_init(&state);
		do {
			//len = FF_Read(pFile, 1, 8192, readBuf);
			len = fread(readBuf, 1, 8192, pFile);
			md5_append(&state, (const md5_byte_t *)readBuf, len);
		} while(len);
		
		md5_finish(&state, digest);

		strcpy(szpHash, "");

		for (di = 0; di < 16; ++di) {
			sprintf(temp, "%02x", digest[di]);
			strcat(szpHash, temp);
		}

		//FF_Close(pFile);
		fclose(pFile);

		return 0;
	}

	return -1;
}

static int fgetline(FILE *pFile, char *szLine, int ulLimit) {
	int c;
	int i;

	if(!pFile || !szLine) {
		return -1;
	}

	for(i = 0; i < (ulLimit - 1) && (c=fgetc(pFile)) >= 0 && c != '\n'; ++i) {
		if(c == '\r') {
			i--;
		} else {
			szLine[i] = (FF_T_INT8) c;
		}
	}

	szLine[i] = '\0';
	return i;
}

static int md5_checkHash(const char *path) {
	FILE		*pFile;
	//FF_ERROR	ffError;
	char		filePath[FF_MAX_PATH];
	char		chkHash[33], fileHash[33];
	char		chkFilename[FF_MAX_FILENAME];
	char		chkLineBuffer[33 + 2 + FF_MAX_FILENAME + 2];
	char		*c, *d;
	
	//pFile = FF_Open(pEnv->pIoman, path, FF_MODE_READ, &ffError);
	pFile = fopen(path, "r");

	if(pFile) {
		while(fgetline(pFile, chkLineBuffer, 33 + 2 + FF_MAX_FILENAME + 2)) {
			memcpy(chkHash, chkLineBuffer, 32);
			chkHash[32] = '\0';

			if(chkLineBuffer[32] != ' ' || chkLineBuffer[33] != ' ') {
				printf("Not a properly formatted md5sum file\n");
			} else {

				c = &chkLineBuffer[34];
				d = chkFilename;
				
				while(*c) {
					*d++ = *c++;			
				}

				*d = '\0';

				//ProcessLinuxPath(filePath, chkFilename);
				strcpy(filePath, chkFilename);
				if(!md5_getHash(filePath, fileHash)) {

					if(!strcmp(chkHash, fileHash)) {
						printf("%s: OK\n", chkFilename);
					} else {
						printf("%s: FAILED\n", chkFilename);
					}
				} else {
					printf("%s: %s: No such file or directory\n", "md5sum", chkFilename);
				}
			}
		}

		//FF_Close(pFile);
		fclose(pFile);
	}

	return 0;
}

