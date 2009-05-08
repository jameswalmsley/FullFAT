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

#include "cmd.h"
#include "md5.h"
#include <stdio.h>

static void FF_PrintDir(FF_DIRENT *pDirent) {
	unsigned char attr[5] = { '-','-','-','-', '\0' };
	if(pDirent->Attrib & FF_FAT_ATTR_READONLY)
			attr[0] = 'R';
	if(pDirent->Attrib & FF_FAT_ATTR_HIDDEN)
			attr[1] = 'H';
	if(pDirent->Attrib & FF_FAT_ATTR_SYSTEM)
			attr[2] = 'S';
	if(pDirent->Attrib & FF_FAT_ATTR_DIR)
			attr[3] = 'D';

	printf("%s %12lu %s\n", attr, pDirent->Filesize, pDirent->FileName);
}


int pwd_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	printf("%s\n", pEnv->WorkingDir);
	return 0;	
}

const FFT_ERR_TABLE pwdInfo[] =
{
	"Types the current working directory to the screen.",			FFT_COMMAND_DESCRIPTION,
	NULL
};

int ls_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	FF_IOMAN *pIoman = pEnv->pIoman;
	FF_DIRENT mydir;

	int  i = 0;
	char tester = 0;
	
	tester = FF_FindFirst(pIoman, &mydir, pEnv->WorkingDir);
	while(tester == 0) {
		FF_PrintDir(&mydir);
		i++;
		tester = FF_FindNext(pIoman, &mydir);
	}
	
	printf("\n%d Items\n", i);
	putchar('\n');

	return 0;
}
const FFT_ERR_TABLE lsInfo[] =
{
	"Lists the contents of the current working directory.",			FFT_COMMAND_DESCRIPTION,
	NULL
};

int cd_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	FF_IOMAN *pIoman = pEnv->pIoman;
	FF_T_INT8	buffer[2600];

	if(argc == 2) {
		if(argv[1][0] != '\\' && argv[1][0] != '/') {
			if(strlen(pEnv->WorkingDir) == 1) {
				sprintf(buffer, "\\%s", argv[1]);
			} else {
				sprintf(buffer, "%s\\%s", pEnv->WorkingDir, argv[1]);
			}

		} else {
			sprintf(buffer, "%s", argv[1]);
		}
		
		if(FF_FindDir(pIoman, buffer, (FF_T_UINT16) strlen(buffer))) {
			sprintf(pEnv->WorkingDir, buffer);
		} else {
			printf("Path \"%s\" not found.\n", argv[1]);
		}
	} else {
		printf("Usage: cd [path]\n");
	}
	return 0;
}

const FFT_ERR_TABLE cdInfo[] =
{
	"Changes the current working directory to the specified path.",			FFT_COMMAND_DESCRIPTION,
	NULL
};



int md5_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	FF_IOMAN *pIoman = pEnv->pIoman;
	FF_T_INT8 buffer[2600];
	FF_T_UINT8	readBuf[8192];
	FF_FILE *fSource;
	FF_T_SINT8 Error;

	int len;
	md5_state_t state;
	md5_byte_t digest[16];
	int di;
	
	if(argc == 2) {
		if(argv[1][0] != '\\' && argv[1][0] != '/') {
			if(strlen(pEnv->WorkingDir) == 1) {
				sprintf(buffer, "\\%s", argv[1]);
			} else {
				sprintf(buffer, "%s\\%s", pEnv->WorkingDir, argv[1]);
			}

		} else {
			sprintf(buffer, "%s", argv[1]);
		}

		fSource = FF_Open(pIoman, buffer, FF_MODE_READ, &Error);

		if(fSource) {
			md5_init(&state);
			do {
				len = FF_Read(fSource, 1, 8192, readBuf);
				md5_append(&state, (const md5_byte_t *)readBuf, len);
			} while(len);
			
			md5_finish(&state, digest);

			for (di = 0; di < 16; ++di)
				printf("%02x", digest[di]);

			printf ("\n");

			FF_Close(fSource);
		} else {
			printf("Could not open file - %s\n", FF_GetErrMessage(Error));
		}
	} else {
		printf("Usage: md5 [filename]\n");
	}

	return 0;
}

const FFT_ERR_TABLE md5Info[] =
{
	"Calculates an MD5 checksum for the specified file.",			FFT_COMMAND_DESCRIPTION,
	NULL
};

