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

#ifdef FF_UNICODE_SUPPORT
static int md5_getHash(const wchar_t *path, char *szpHash, FF_ENVIRONMENT *pEnv);
static int md5_checkHash(const wchar_t *path, FF_ENVIRONMENT *pEnv);
#else
static int md5_getHash(const char *path, char *szpHash, FF_ENVIRONMENT *pEnv);
static int md5_checkHash(const char *path, FF_ENVIRONMENT *pEnv);
#endif

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
	FF_T_BOOL			bCheck = FF_FALSE, bVerify = FF_FALSE;
	FF_DIRENT			findData;
	FF_FILE				*pfOut = NULL;

	FF_ERROR			RetVal, ffError;
#ifdef FF_UNICODE_SUPPORT
	wchar_t				path[FF_MAX_PATH];
	wchar_t				outfilepath[FF_MAX_PATH];
	wchar_t				argvwc[FF_MAX_PATH];
#else
	char				path[FF_MAX_PATH];
	char				outfilepath[FF_MAX_PATH];
#endif

	const char			*szargPath;
	const char			*szargOutputFile = NULL;
	const char			*verifysum = NULL;
	
	
	char				hash[33];

	int i;

	memset(&optionContext, 0, sizeof(FFT_GETOPT_CONTEXT));

	option = FFTerm_getopt(argc, (const char **) argv, "bctwo:v:s:", &optionContext);

	if(option != EOF) {
		do {
			switch(option) {
				case 'b':
					//bBinary = FF_TRUE;
					break;
				case 'c':
					bCheck = FF_TRUE;
					break;

				case 't':
					//bBinary = FF_FALSE;
					break;

				case 'w':
//					bWarn = FF_TRUE;
					break;

				case 'o':
					szargOutputFile = optionContext.optarg;
					break;

			case 'v':
				verifysum = optionContext.optarg;
				bVerify = FF_TRUE;
				break;
			case 's':
				//string = optionContext.optarg;
				break;
			}

			option = FFTerm_getopt(argc, (const char ** ) argv, "bctwo:v:s:", &optionContext);
		} while(option != EOF);
	}

	szargPath = FFTerm_getarg(argc, (const char **) argv, 0, &optionContext);	// Get the available non-option arguments remaining.
		
	if(szargPath) {

#ifdef FF_UNICODE_SUPPORT
		FF_cstrtowcs(argvwc, szargPath);
		ProcessPath(path, argvwc, pEnv);
#else
		ProcessPath(path, szargPath, pEnv);
#endif

		RetVal = FF_FindFirst(pEnv->pIoman, &findData, path);

		if(RetVal) {
			printf("%s: %s: No such file or directory.\n", argv[0], szargPath);
			return 0;
		}

		if(szargOutputFile) {
#ifdef FF_UNICODE_SUPPORT
			FF_cstrtowcs(argvwc, szargOutputFile);
			ProcessPath(outfilepath, argvwc, pEnv);
#else
			ProcessPath(outfilepath, szargOutputFile, pEnv);
#endif
			pfOut = FF_Open(pEnv->pIoman, outfilepath, FF_MODE_WRITE | FF_MODE_TRUNCATE | FF_MODE_CREATE, &ffError);
			if(!pfOut) {
				printf("%s: Error opening %s for writing: %s\n", argv[0], szargOutputFile, FF_GetErrMessage(ffError));
				return 0;
			}
		}

		do {
			if(findData.Attrib & FF_FAT_ATTR_DIR) {
#ifdef FF_UNICODE_SUPPORT
				printf("%s: %ls: Is a directory\n", argv[0], findData.FileName);
#else
				printf("%s: %s: Is a directory\n", argv[0], findData.FileName);
#endif
			} else {
#ifdef FF_UNICODE_SUPPORT
				wcsAppendFilename(path, findData.FileName);
#else
				AppendFilename(path, findData.FileName);
#endif

				if(!bCheck && !bVerify) {
					if(!md5_getHash(path, hash, pEnv)) {
#ifdef FF_UNICODE_SUPPORT
						printf("%s  %ls\n", hash, findData.FileName);
#else
						printf("%s  %s\n", hash, findData.FileName);
#endif
					}
#ifdef FF_UNICODE_SUPPORT
					if(szargOutputFile) {
						FF_cstrtowcs(argvwc, szargOutputFile);
					}
					if(szargOutputFile && wcscmp(argvwc, findData.FileName)) {
#else
					if(szargOutputFile && strcmp(szargOutputFile, findData.FileName)) {
#endif
						for(i = 0; i < 32; i++) {
							FF_PutC(pfOut, hash[i]);
						}
						FF_PutC(pfOut, ' ');
						FF_PutC(pfOut, ' ');
						for(i = 0; findData.FileName[i] != '\0'; i++) {
							FF_PutC(pfOut, (FF_T_UINT8) findData.FileName[i]);
						}
						FF_PutC(pfOut, '\n'); // Nextline!
					}
				} else if(bVerify && !pfOut) {
						if(!md5_getHash(path, hash, pEnv)) {
							//printf("%s : %s  $s", hash, verifysum, findData.FileName);
							if(!strcmp(hash, verifysum)) {
								return 0;
							} else {
								return 1;
							}
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


#ifdef FF_UNICODE_SUPPORT
static int md5_getHash(const wchar_t *path, char *szpHash, FF_ENVIRONMENT *pEnv) {
#else
static int md5_getHash(const char *path, char *szpHash, FF_ENVIRONMENT *pEnv) {
#endif
	FF_T_UINT8			readBuf[FF_CMD_BUFFER_SIZE];
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

#ifdef FF_UNICODE_SUPPORT
static int md5_checkHash(const wchar_t *path, FF_ENVIRONMENT *pEnv) {
#else
static int md5_checkHash(const char *path, FF_ENVIRONMENT *pEnv) {
#endif

	FF_FILE		*pFile;
	FF_ERROR	ffError;
#ifdef FF_UNICODE_SUPPORT
	wchar_t		filePath[FF_MAX_PATH];
	wchar_t		wctemp[FF_MAX_PATH];
#else
	char		filePath[FF_MAX_PATH];
#endif
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

#ifdef FF_UNICODE_SUPPORT
				FF_cstrtowcs(wctemp, chkFilename);
				ProcessPath(filePath, wctemp, pEnv);
#else
				ProcessPath(filePath, chkFilename, pEnv);
#endif

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

