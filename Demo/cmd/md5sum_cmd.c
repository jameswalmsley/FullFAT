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
#include "md5sum_cmd.h"

static int md5_getHash(const char *path, char *szpHash, FF_ENVIRONMENT *pEnv);
static int md5_checkHash(const char *path, FF_ENVIRONMENT *pEnv);

/**
 *	@public
 *	@brief	MD5 Data Hashing function.
 *
 *	Generates and displays an MD5 hash of a file. This is really useful when
 *	verify files for their integrity. We used MD5 extensively while stabilising
 *	the read and write functionality of FullFAT.
 *
 **/
int md5sum_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	int option;
	FFT_GETOPT_CONTEXT	optionContext;
	FF_T_BOOL			bBinary = FF_FALSE, bCheck = FF_FALSE, bWarn = FF_FALSE;
	FF_DIRENT			findData;
	FF_FILE				*pfOut;

	FF_ERROR			RetVal, ffError;
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
		ProcessPath(path, szargPath, pEnv);
		
		RetVal = FF_FindFirst(pEnv->pIoman, &findData, path);

		if(RetVal) {
			printf("%s: %s: No such file or directory.\n", argv[0], szargPath);
			return 0;
		}

		if(szargOutputFile) {
			ProcessPath(outfilepath, szargOutputFile, pEnv);
			pfOut = FF_Open(pEnv->pIoman, outfilepath, FF_MODE_WRITE | FF_MODE_TRUNCATE | FF_MODE_CREATE, &ffError);
			if(!pfOut) {
				printf("%s: Error opening %s for writing: %s\n", argv[0], szargOutputFile, FF_GetErrMessage(ffError));
				return 0;
			}
		}

		do {
			if(findData.Attrib & FF_FAT_ATTR_DIR) {
				printf("%s: %s: Is a directory\n", argv[0], findData.FileName);
			} else {
				AppendFilename(path, findData.FileName);
				if(!bCheck) {
					if(!md5_getHash(path, hash, pEnv)) {
						printf("%s  %s\n", hash, findData.FileName);
					}

					if(szargOutputFile && strcmp(szargOutputFile, findData.FileName)) {
						for(i = 0; i < 32; i++) {
							FF_PutC(pfOut, hash[i]);
						}
						FF_PutC(pfOut, ' ');
						FF_PutC(pfOut, ' ');
						for(i = 0; findData.FileName[i] != '\0'; i++) {
							FF_PutC(pfOut, findData.FileName[i]);
						}
						FF_PutC(pfOut, '\n'); // Nextline!
					}

				} else {
					md5_checkHash(path, pEnv);
				}
			}
			RetVal = FF_FindNext(pEnv->pIoman, &findData);

		} while(!RetVal);

		if(szargOutputFile) {
			FF_Close(pfOut);
		}

		return 0;
	}
	
	printf("See usage!\n");	

	return 0;
}
const FFT_ERR_TABLE md5sumInfo[] =
{	
	{"Unknown or Generic Error",		-1},							// Generic Error (always the first entry).
	{"Calculates an MD5 checksum for the specified file.",			FFT_COMMAND_DESCRIPTION},
	{ NULL }
};



static int md5_getHash(const char *path, char *szpHash, FF_ENVIRONMENT *pEnv) {
	FF_T_UINT8			readBuf[8192];
	FF_FILE				*pFile;
	FF_ERROR			ffError;

	int					len;
	md5_state_t 		state;
	md5_byte_t			digest[16];
	char				temp[3];
	int					di;

	pFile = FF_Open(pEnv->pIoman, path, FF_MODE_READ, &ffError);

	if(pFile) {
		md5_init(&state);
		do {
			len = FF_Read(pFile, 1, 8192, readBuf);
			md5_append(&state, (const md5_byte_t *)readBuf, len);
		} while(len);
		
		md5_finish(&state, digest);

		strcpy(szpHash, "");

		for (di = 0; di < 16; ++di) {
			sprintf(temp, "%02x", digest[di]);
			strcat(szpHash, temp);
		}

		FF_Close(pFile);

		return 0;
	}

	return -1;
}

static int md5_checkHash(const char *path, FF_ENVIRONMENT *pEnv) {
	FF_FILE		*pFile;
	FF_ERROR	ffError;
	char		filePath[FF_MAX_PATH];
	char		chkHash[33], fileHash[33];
	char		chkFilename[FF_MAX_FILENAME];
	char		chkLineBuffer[33 + 2 + FF_MAX_FILENAME + 2];
	char		*c, *d;
	
	pFile = FF_Open(pEnv->pIoman, path, FF_MODE_READ, &ffError);

	if(pFile) {
		while(FF_GetLine(pFile, chkLineBuffer, 33 + 2 + FF_MAX_FILENAME + 2)) {
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

				ProcessPath(filePath, chkFilename, pEnv);

				if(!md5_getHash(filePath, fileHash, pEnv)) {

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

		FF_Close(pFile);
	}

	return 0;
}

