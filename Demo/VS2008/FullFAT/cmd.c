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
#include <windows.h>

#define COPY_BUFFER_SIZE 8192

/*
	This is a standardised DIRENT print for FullFAT.
	It mixes styles used in Windows and Linux.
*/
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
#ifdef FF_TIME_SUPPORT
	printf("%0.2d.%0.2d.%0.2d  %0.2d:%0.2d  %s  %12lu  %s\n", pDirent->CreateTime.Day, pDirent->CreateTime.Month, pDirent->CreateTime.Year, pDirent->CreateTime.Hour, pDirent->CreateTime.Minute, attr, pDirent->Filesize, pDirent->FileName);
#else
	printf("%s %12lu %s\n", attr, pDirent->Filesize, pDirent->FileName);
#endif
}

/*
	Makes a path absolute.
*/
static void ProcessPath(char *dest, char *src, FF_ENVIRONMENT *pEnv) {
	if(src[0] != '\\' && src[0] != '/') {
		if(strlen(pEnv->WorkingDir) == 1) {
			sprintf(dest, "\\%s", src);
		} else {
			sprintf(dest, "%s\\%s", pEnv->WorkingDir, src);
		}

	} else {
		sprintf(dest, "%s", src);
	}
}


/*
	This routine removes all relative ..\ from a path.
	It's probably not the best, but it works. 

	Its based on some old code I wrote a long time ago.
*/
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



/**
 *	This command acts as the command prompt.
 **/
int cmd_prompt(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	if(argc) {
		printf("This is the command prompt application. \nIt cannot be executed directly from the command line.\n");
		printf("For more information about FullFAT or the FFTerm software, see:\nwww.worm.me.uk/fullfat/\n");
	} else {
		printf("FullFAT%s>", pEnv->WorkingDir);
	}
	if(argv) {

	}
	return 0;
}
const FFT_ERR_TABLE promptInfo[] =
{
	"The command prompt!",			FFT_COMMAND_DESCRIPTION,
	NULL
};

int pwd_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	if(argc == 1) {
		printf("Current directory:\n");
		printf("%s\n", pEnv->WorkingDir);
	} else {
		printf("Usage: %s\n", argv[0]);
	}
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

	printf("Type \"ls ?\" for an attribute legend.\n");
	
	if(argc == 1) {
		tester = FF_FindFirst(pIoman, &mydir, pEnv->WorkingDir);
		printf("Directory Listing of: %s\n", pEnv->WorkingDir);
	} else {

		if(argv[1][0] == '?') {
			printf("ATTR Info:\n");
			printf("D:\tDirectory\n");
			printf("H:\tHidden\n");
			printf("S:\tSystem\n");
			printf("R:\tRead-only\n");
			return 0;
		}

		if(!FF_FindDir(pIoman, argv[1], (FF_T_UINT16) strlen(argv[1]))) {
			printf("Path %s Not Found!\n\n", argv[1]);
			return 0;
		}
		tester = FF_FindFirst(pIoman, &mydir, argv[1]);
		printf("Directory Listing of: %s\n", argv[1]);
	}

	printf("\n");

#ifdef FF_TIME_SUPPORT
	printf("   DATE   | TIME | ATTR |  FILESIZE  |  FILENAME         \n");
	printf("---------------------------------------------------------\n");
#else

#endif

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
	FF_T_INT8	path[FF_MAX_PATH];

	if(argc == 2) {
		ProcessPath(path, argv[1], pEnv);

		ExpandPath(path);
		
		if(FF_FindDir(pIoman, path, (FF_T_UINT16) strlen(path))) {
			sprintf(pEnv->WorkingDir, path);
		} else {
			printf("Path \"%s\" not found.\n", path);
		}
	} else {
		printf("Usage: %s [path]\n", argv[0]);
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
	FF_T_INT8 path[FF_MAX_PATH];
	FF_T_UINT8	readBuf[8192];
	FF_FILE *fSource;
	FF_ERROR Error;

	int len;
	md5_state_t state;
	md5_byte_t digest[16];
	int di;
	
	if(argc == 2) {
		
		ProcessPath(path, argv[1], pEnv);

		fSource = FF_Open(pIoman, path, "rb", &Error);

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
		printf("Usage: %s [filename]\n", argv[0]);
	}

	return 0;
}

const FFT_ERR_TABLE md5Info[] =
{
	"Calculates an MD5 checksum for the specified file.",			FFT_COMMAND_DESCRIPTION,
	NULL
};


int cp_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	FF_IOMAN *pIoman = pEnv->pIoman;
	FF_FILE *fSource, *fDest;
	FF_ERROR Error;

	FF_T_INT8 path[2600];
	FF_T_UINT8 copybuf[COPY_BUFFER_SIZE];

	FF_T_SINT32	BytesRead;

	LARGE_INTEGER ticksPerSecond;
	LARGE_INTEGER start_ticks, end_ticks, cputime;
	float time, transferRate;

	QueryPerformanceFrequency(&ticksPerSecond);
	
	if(argc == 3) {
		ProcessPath(path, argv[1], pEnv);
		fSource = FF_Open(pIoman, path, "rb", &Error);
		if(fSource) {
			ProcessPath(path, argv[2], pEnv);
			fDest = FF_Open(pIoman, path, "wb", &Error);
			if(fDest) {
				// Do the copy
				QueryPerformanceCounter(&start_ticks);  
				do{
					BytesRead = FF_Read(fSource, COPY_BUFFER_SIZE, 1, (FF_T_UINT8 *)copybuf);
					FF_Write(fDest, BytesRead, 1, (FF_T_UINT8 *) copybuf);
					QueryPerformanceCounter(&end_ticks); 
					cputime.QuadPart = end_ticks.QuadPart - start_ticks.QuadPart;
					time = ((float)cputime.QuadPart/(float)ticksPerSecond.QuadPart);
					transferRate = (fSource->FilePointer / time) / 1024;
					printf("%3.0f%% - %10d Bytes Copied, %7.2f Kb/S\r", ((float)((float)fSource->FilePointer/(float)fSource->Filesize) * 100), fSource->FilePointer, transferRate);
				}while(BytesRead > 0);
				printf("%3.0f%% - %10d Bytes Copied, %7.2f Kb/S\n", ((float)((float)fSource->FilePointer/(float)fSource->Filesize) * 100), fSource->FilePointer, transferRate);		

				FF_Close(fSource);
				FF_Close(fDest);
			} else {
				FF_Close(fSource);
				printf("Could not open destination file - %s\n", FF_GetErrMessage(Error));
			}

		} else {
			printf("Could not open source file - %s\n", FF_GetErrMessage(Error));
		}
		
	} else {
		printf("Usage: %s [source file] [destination file]\n", argv[0]);
	}
	return 0;
}

const FFT_ERR_TABLE cpInfo[] =
{
	"Copies the specified file to the specified location.",			FFT_COMMAND_DESCRIPTION,
	NULL
};

int xcp_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	FF_IOMAN	*pIoman = pEnv->pIoman;
	FF_FILE		*fSource;
	FILE		*fDest;
	FF_ERROR	Error;

	FF_T_INT8	path[2600];
	FF_T_UINT8	copybuf[COPY_BUFFER_SIZE];

	FF_T_SINT32	BytesRead;

	LARGE_INTEGER ticksPerSecond;
	LARGE_INTEGER start_ticks, end_ticks, cputime;
	float time, transferRate;

	QueryPerformanceFrequency(&ticksPerSecond);
	
	if(argc == 3) {
		ProcessPath(path, argv[1], pEnv);
		fSource = FF_Open(pIoman, path, "rb", &Error);
		if(fSource) {
			fDest = fopen(argv[2], "rb+");
			if(fDest) {
				// Do the copy
				QueryPerformanceCounter(&start_ticks);  
				do{
					BytesRead = FF_Read(fSource, COPY_BUFFER_SIZE, 1, (FF_T_UINT8 *)copybuf);
					fwrite(copybuf, BytesRead, 1, fDest);
					QueryPerformanceCounter(&end_ticks); 
					cputime.QuadPart = end_ticks.QuadPart - start_ticks.QuadPart;
					time = ((float)cputime.QuadPart/(float)ticksPerSecond.QuadPart);
					transferRate = (fSource->FilePointer / time) / 1024;
					printf("%3.0f%% - %10d Bytes Copied, %7.2f Kb/S\r", ((float)((float)fSource->FilePointer/(float)fSource->Filesize) * 100), fSource->FilePointer, transferRate);
				}while(BytesRead > 0);
				printf("%3.0f%% - %10d Bytes Copied, %7.2f Kb/S\n", ((float)((float)fSource->FilePointer/(float)fSource->Filesize) * 100), fSource->FilePointer, transferRate);		

				FF_Close(fSource);
				fclose(fDest);
			} else {
				FF_Close(fSource);
				printf("Could not open destination file.\n");
			}

		} else {
			printf("Could not open source file - %s\n", FF_GetErrMessage(Error));
		}
		
	} else {
		printf("Usage: %s [source file] [destination file]\n", argv[0]);
	}
	return 0;
}
const FFT_ERR_TABLE xcpInfo[] =
{
	"Copies a FullFAT file to the Hard-disk drive.",			FFT_COMMAND_DESCRIPTION,
	NULL
};

int icp_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	FF_IOMAN *pIoman = pEnv->pIoman;
	FF_FILE *fDest;
	FILE	*fSource;
	FF_ERROR Error;

	FF_T_INT8 path[2600];
	FF_T_UINT8 copybuf[COPY_BUFFER_SIZE];

	FF_T_SINT32	BytesRead;
	FF_T_UINT32	SourceSize;

	LARGE_INTEGER ticksPerSecond;
	LARGE_INTEGER start_ticks, end_ticks, cputime;
	float time, transferRate;

	QueryPerformanceFrequency(&ticksPerSecond);
	
	if(argc == 3) {
		fSource = fopen(argv[1], "rb");
		if(fSource) {
			fseek(fSource, 0, SEEK_END);
			SourceSize = ftell(fSource);
			fseek(fSource, 0, SEEK_SET);
			ProcessPath(path, argv[2], pEnv);
			fDest = FF_Open(pIoman, path, "w", &Error);
			if(fDest) {
				// Do the copy
				QueryPerformanceCounter(&start_ticks);  
				do{
					BytesRead = fread(copybuf, 1, COPY_BUFFER_SIZE, fSource);
					FF_Write(fDest, BytesRead, 1, (FF_T_UINT8 *) copybuf);
					QueryPerformanceCounter(&end_ticks); 
					cputime.QuadPart = end_ticks.QuadPart - start_ticks.QuadPart;
					time = ((float)cputime.QuadPart/(float)ticksPerSecond.QuadPart);
					transferRate = (ftell(fSource) / time) / 1024;
					printf("%3.0f%% - %10d Bytes Copied, %7.2f Kb/S\r", ((float)((float)ftell(fSource)/(float)SourceSize) * 100), ftell(fSource), transferRate);
				}while(BytesRead > 0);
				printf("%3.0f%% - %10d Bytes Copied, %7.2f Kb/S\n", ((float)((float)ftell(fSource)/(float)SourceSize) * 100), ftell(fSource), transferRate);	

				fclose(fSource);
				FF_Close(fDest);
			} else {
				fclose(fSource);
				printf("Could not open destination file - %s\n", FF_GetErrMessage(Error));
			}

		} else {
			printf("Could not open source file.\n");
		}
		
	} else {
		printf("Usage: %s [source file] [destination file]\n", argv[0]);
	}
	return 0;
}

const FFT_ERR_TABLE icpInfo[] =
{
	"Copies a file from the Hard-disk to the FullFAT partition.",	FFT_COMMAND_DESCRIPTION,
	NULL
};


int mkdir_cmd(int argc, char **argv, FF_ENVIRONMENT *pEv) {
	
	FF_T_INT8	path[FF_MAX_PATH];
	FF_T_SINT8	Error;

	if(argc == 2) {
		ProcessPath(path, argv[1], pEv);
		Error = FF_MkDir(pEv->pIoman, path);
		if(Error) {
			printf("Could not mkdir - %s\n", FF_GetErrMessage(Error));
		}
	} else {
		printf("Usage: %s [path]\n", argv[0]);
	}
	return 0;
}

const FFT_ERR_TABLE mkdirInfo[] =
{
	"Creates directories.",	FFT_COMMAND_DESCRIPTION,
	NULL
};

int info_cmd(int argc, char **argv, FF_ENVIRONMENT *pEv) {
	FF_IOMAN		*pIoman = pEv->pIoman;
	FF_PARTITION	*pPart	= pEv->pIoman->pPartition;
	
	if(argc == 1) {
		switch(pPart->Type) {
			case FF_T_FAT32:
				printf("FAT32 Formatted\n"); break;
			case FF_T_FAT16:
				printf("FAT16 Formatted\n"); break;
			case FF_T_FAT12:
				printf("FAT12 Formatted\n"); break;
		}
		printf("FullFAT Driver Blocksize: \t%d\n", pIoman->BlkSize);
		printf("Partition Blocksize: \t\t%d\n", pPart->BlkSize);
		printf("Cluster Size: \t\t\t%dKb\n", (pPart->BlkSize * pPart->SectorsPerCluster) / 1024);
		printf("Volume Size: \t\t\t%llu (%d MB)\n", FF_GetVolumeSize(pIoman), (unsigned int) (FF_GetVolumeSize(pIoman) / 1048576));
		printf("Volume Free: \t\t\t%llu (%d MB)\n", FF_GetFreeSize(pIoman), (unsigned int) (FF_GetFreeSize(pIoman) / 1048576));
	} else {
		printf("Usage: %s\n", argv[0]);
	}
	return 0;
}
const FFT_ERR_TABLE infoInfo[] =
{
	"Displays information about the currently mounted partition.",	FFT_COMMAND_DESCRIPTION,
	NULL
};


/*int mount_cmd(int argc, char **argv, FF_ENVIRONMENT *pEv) {
	return -4;
}

const FFT_ERR_TABLE mountInfo[] =
{
	"Used to mount various partitions and volumes.",	FFT_COMMAND_DESCRIPTION,
	"Command Not Yet Implemented",						-4,
	NULL
};

*/

/*
	A View command to type out the contents of a file using FullFAT.
*/
int view_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	
	FF_FILE		*f;
	FF_ERROR	Error;
	FF_T_SINT32	c;
	FF_T_INT8	path[FF_MAX_PATH];

	if(argc == 2) {

		ProcessPath(path, argv[1], pEnv);

		f = FF_Open(pEnv->pIoman, path, "rb", &Error);
		if(f) {
			printf("//---------- START OF FILE\n");
			while(!FF_isEOF(f)) {
				c = FF_GetC(f);
				if(c >= 0) {
					printf("%c", c);
				} else {
					printf("Error while reading file: %s\n", FF_GetErrMessage(c));
					FF_Close(f);
					return -3;
				}
			}
			printf("\n//---------- END OF FILE\n");

			FF_Close(f);
		} else {
			printf("Could not open file: %s\n", FF_GetErrMessage(Error));
			return -2;
		}
	} else {
		printf("Usage: %s [filename]\n", argv[0]);
	}
	return 0;
}
const FFT_ERR_TABLE viewInfo[] =
{											// This demonstrates how FFTerm can provide useful information about specific command failure codes.
	"Unknown or Generic Error",				-1,	// Generic Error must always be the first in the table.
	"Types out the specified file.",		FFT_COMMAND_DESCRIPTION,
	"File open failed. (File not found?)",	-2,
	"Error while reading from device!",		-3,
	NULL
};