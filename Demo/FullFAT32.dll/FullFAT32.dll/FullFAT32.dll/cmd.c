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
#include <stdio.h>
#include <windows.h>


/**
 *	The functions and commands provided in this file, serve as a reference to creating
 *	some common and standard functions with FullFAT. They may be reproduced or copied in your
 *	own software. Although no warranty is claimed for the code below, they do follow
 *	the recommended usage of FullFAT to create such functionality.
 *
 *	FFTerm (FullFAT Terminal) as used in this DEMO is also completely free under the terms of 
 *	the GNU GPL License. For alternative licensing regarding FFTerm, please contact the author.
 *
 *	Look at the MinOS version (FullFAT DEMO) to see the equivalent using a stdio and POSIX interface.
 *	Note: MinOS integrates FullFAT and exposes standard and POSIX interfaces to the developer.
 **/

#define COPY_BUFFER_SIZE 8192

static FF_T_BOOL wildCompare(const char * pszWildCard, const char * pszString);
static void ExpandPath(char *acPath);


/**
 *	@public
 *	@brief	A Standardised Dirent Print for FullFAT
 *
 *	Prints a Directory Entry, using a mix of styles found in Windows and Linux.
 *
 *	@param	pDirent		A pointer to an FF_DIRENT object as populated by the FF_FindFirst() or FF_FindNext() functions.
 **/
static void FF_PrintDir(FF_DIRENT *pDirent) {
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
	printf("%02d.%02d.%02d  %02d:%02d  %s  %12lu  %s\n", pDirent->CreateTime.Day, pDirent->CreateTime.Month, pDirent->CreateTime.Year, pDirent->CreateTime.Hour, pDirent->CreateTime.Minute, attr, pDirent->Filesize, pDirent->FileName);
#else
	printf(" %s %12lu    %s\n", attr, pDirent->Filesize, pDirent->FileName);
#endif
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
void ProcessPath(char *dest, char *src, FF_ENVIRONMENT *pEnv) {
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


/**
 *	@public
 *	@brief	A Special Command Prompt command for FFTerm
 *
 *	FFTerm allows dynamically generated command prompts when a command named
 *	"prompt" is hooked. This is that command, allowing the prompt to include the
 *	the current working directory.
 *
 *	@param	argc	The number of argument strings provided on the command line.
 *	@param	argc	An array of pointers, pointing to strings representing the command line arguments.
 *	@param	pEnv	Pointer to an FF_ENVIRONMENT object.
 *
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
/**
 *	You will see many of these tables after each command. They are optional.
 *	These tables simply provide descriptions of the commands to FFTerm.
 *	The first entry is always a generic error code (-1). The table is always NULL terminated.
 *
 *	The strings are used to provide meaningful messages to the user for various error codes.
 *	FFT_COMMAND_DESCRIPTION defines a command description, and is displayed via the help command.
 **/
const FFT_ERR_TABLE promptInfo[] =
{
	{"Unknown or Generic Error",						-1},						// Generic Error (always the first entry).
	{"This message is displayed when -2 is returned",	-2},						// Error Message for a -2 return code.
	{"The command prompt!",								FFT_COMMAND_DESCRIPTION},	// Command Description.
	{ NULL }				// Always terminated with NULL. 
};


/**
 *	@brief	Prints the current working directory.
 *
 *	@param	argc	The number of argument strings provided on the command line.
 *	@param	argc	An array of pointers, pointing to strings representing the command line arguments.
 *	@param	pEnv	Pointer to an FF_ENVIRONMENT object.
 *
 **/
int pwd_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	if(argc == 1) {
		printf("Current directory:\n");							// Simply print WorkingDir from pEnv object.
		printf("%s\n", pEnv->WorkingDir);
	} else {
		printf("Usage: %s\n", argv[0]);							// argv[0] is always the name of the command.
	}
	return 0;	
}
const FFT_ERR_TABLE pwdInfo[] =
{
	{"Unknown or Generic Error",		-1},					// Generic Error (always the first entry).
	{"Types the current working directory to the screen.",		FFT_COMMAND_DESCRIPTION},
	{ NULL }
};




/**
 *	@public
 *	@brief	List Directory Command
 *
 *	Lists the working directory or a specified directory.
 *
 *	PARAMS are explained above.
 **/
int ls_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	FF_IOMAN *pIoman = pEnv->pIoman;	// FF_IOMAN object from the Environment
	FF_DIRENT mydir;					// DIRENT object.
	int  i = 0;
	FF_ERROR tester = 0;

	printf("Type \"%s ?\" for an attribute legend.\n", argv[0]);
	
	if(argc == 1) {	// Default, open the working directory.
		tester = FF_FindFirst(pIoman, &mydir, pEnv->WorkingDir);
		printf("Directory Listing of: %s\n", pEnv->WorkingDir);
	} else if(argc == 2){

		if(argv[1][0] == '?') {			// If ? is first argument, print a legend.
			printf("ATTR Info:\n");
			printf("D:\tDirectory\n");
			printf("H:\tHidden\n");
			printf("S:\tSystem\n");
			printf("R:\tRead-only\n");
			return 0;
		}

										// Otherwise try to open a provided path.
		if(!FF_FindDir(pIoman, argv[1], (FF_T_UINT16) strlen(argv[1]))) {
			printf("Path %s Not Found!\n\n", argv[1]);	// Dir not found!
			return 0;
		}
		tester = FF_FindFirst(pIoman, &mydir, argv[1]);	// Find first Object.
		printf("Directory Listing of: %s\n", argv[1]);
	} else {
		printf("Usage: %s\n", argv[0]);
		printf("Or: %s ? \t// Displays a legend!\n", argv[0]);
		printf("Or: %s [path]\t// Lists a specified path.\n", argv[0]);
		return 0;
	}

	printf("\n");

#ifdef FF_TIME_SUPPORT
	printf("   DATE   | TIME | ATTR |  FILESIZE  |  FILENAME         \n");
	printf("---------------------------------------------------------\n");
#else
	printf(" ATTR |  FILESIZE  |  FILENAME         \n");
	printf("---------------------------------------------------------\n");
#endif

	while(tester == 0) {		// Loop printing each Dirent until no more are available.
		FF_PrintDir(&mydir);	// FF_FindNext() returns 0 on success.
		i++;
		tester = FF_FindNext(pIoman, &mydir);
	}
	
	printf("\n%d Items\n", i);
	printf("\n");

	return 0;
}
const FFT_ERR_TABLE lsInfo[] =
{
	{"Unknown or Generic Error",		-1},							// Generic Error (always the first entry).
	{"Lists the contents of the current working directory.",			FFT_COMMAND_DESCRIPTION},
	{ NULL }
};


/**
 *	@public
 *	@brief	Changes the Current Working Directory
 *
 *	To change the directory, we simply check the provided path, if it exists,
 *	then we change the environment's working directory to that path.
 *
 **/
int cd_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	FF_IOMAN *pIoman = pEnv->pIoman;
	FF_T_INT8	path[FF_MAX_PATH];

	int i;

	if(argc == 2) {
		ProcessPath(path, argv[1], pEnv);	// Make path absolute if relative.

		ExpandPath(path);	// Remove any relativity from the path (../ or ..\).
		
		if(FF_FindDir(pIoman, path, (FF_T_UINT16) strlen(path))) {	// Check if path is valid.
			i = strlen(path) - 1;	// Path found, change the directory.

			if(i) {
				if(path[i] == '\\' || path[i] == '/') {
					path[i] = '\0';
				}
			}
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
	{"Unknown or Generic Error",		-1},							// Generic Error (always the first entry).
	{"Changes the current working directory to the specified path.",			FFT_COMMAND_DESCRIPTION},
	{ NULL }
};


/**
 *	@public
 *	@brief	MD5 Data Hashing function.
 *
 *	Generates and displays an MD5 hash of a file. This is really useful when
 *	verify files for their integrity. We used MD5 extensively while stabilising
 *	the read and write functionality of FullFAT.
 *
 **/
/*int md5_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	FF_IOMAN	*pIoman = pEnv->pIoman;
	FF_T_INT8	path[FF_MAX_PATH];
	FF_T_UINT8	readBuf[8192];
	FF_FILE		*fSource;
	FF_ERROR	Error;

	int			len;
	md5_state_t state;
	md5_byte_t	digest[16];
	int			di;
	
	if(argc == 2) {
		
		ProcessPath(path, argv[1], pEnv);

		fSource = FF_Open(pIoman, path, FF_MODE_READ, &Error);

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
	{"Unknown or Generic Error",		-1},							// Generic Error (always the first entry).
	{"Calculates an MD5 checksum for the specified file.",			FFT_COMMAND_DESCRIPTION},
	{ NULL }
};
*/

/**
 *	@public
 *	@brief	MD5 Data Hashing function. (For Windows Files).
 *
 *	Generates and displays an MD5 hash of a file. This is really useful when
 *	verify files for their integrity. We used MD5 extensively while stabilising
 *	the read and write functionality of FullFAT.
 *
 **/
/*
int md5win_cmd(int argc, char **argv) {

	FF_T_UINT8	readBuf[8192];
	FILE		*fSource;

	int			len;
	md5_state_t state;
	md5_byte_t	digest[16];
	int			di;
	
	if(argc == 2) {
		
		fSource = fopen(argv[1], "rb");

		if(fSource) {
			md5_init(&state);
			do {
				len = fread(readBuf, 1, 8192, fSource);
				md5_append(&state, (const md5_byte_t *)readBuf, len);
			} while(len);
			
			md5_finish(&state, digest);

			for (di = 0; di < 16; ++di)
				printf("%02x", digest[di]);

			printf ("\n");

			fclose(fSource);
		} else {
			printf("Could not open file.\n");
		}
	} else {
		printf("Usage: %s [filename]\n", argv[0]);
	}

	return 0;
}
const FFT_ERR_TABLE md5winInfo[] =
{	
	{"Unknown or Generic Error",		-1},							// Generic Error (always the first entry).
	{"Calculates an MD5 checksum for the specified Windows file.",			FFT_COMMAND_DESCRIPTION},
	{ NULL }
};
*/

/*
	This isn't a command, but rather a simple copy function designed to aid
	wildCard copying.
*/
int filecopy(const char *src, const char *dest, FF_ENVIRONMENT *pEnv) {
	
	FF_IOMAN *pIoman = pEnv->pIoman;
	FF_ERROR Error;

	FF_FILE *fSource, *fDest;

	FF_T_UINT8 copybuf[COPY_BUFFER_SIZE];

	FF_T_SINT32	BytesRead;

	LARGE_INTEGER ticksPerSecond;
	LARGE_INTEGER start_ticks, end_ticks, cputime, ostart, oend;
	float time, transferRate;
	int ticks;

	QueryPerformanceFrequency(&ticksPerSecond);
	
	QueryPerformanceCounter(&ostart); 
	fSource = FF_Open(pIoman, src, FF_MODE_READ, &Error);
	QueryPerformanceCounter(&oend);
	ticks = (int) (oend.QuadPart - ostart.QuadPart);
	//printf("Source: Open took %d\n", ticks);
	if(fSource) {
		QueryPerformanceCounter(&ostart);
		fDest = FF_Open(pIoman, dest, FF_GetModeBits("w"), &Error);
		QueryPerformanceCounter(&oend);
		ticks = (int) (oend.QuadPart - ostart.QuadPart);
		printf("Dest: Open took %d\n", ticks);
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
				printf("%3.0f%% - %10ld Bytes Copied, %7.2f Kb/S\r", ((float)((float)fSource->FilePointer/(float)fSource->Filesize) * 100), fSource->FilePointer, transferRate);
			}while(BytesRead > 0);
			printf("%3.0f%% - %10ld Bytes Copied, %7.2f Kb/S\n", ((float)((float)fSource->FilePointer/(float)fSource->Filesize) * 100), fSource->FilePointer, transferRate);		

			FF_Close(fSource);
			FF_Close(fDest);
		} else {
			FF_Close(fSource);
			printf("Could not open destination file - %s\n", FF_GetErrMessage(Error));
		}

	} else {
		if(Error == FF_ERR_FILE_OBJECT_IS_A_DIR) {
			return FF_ERR_FILE_OBJECT_IS_A_DIR;
		}
		printf("Could not open source file - %s\n", FF_GetErrMessage(Error));
	}
	return 0;
}


/*
	Copies with wild-cards!
	cp_cmd redirects here if it detects a wildCard in the source or destination paths.
*/
int wildcopy(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	
	FF_T_INT8	pathsrc[FF_MAX_PATH];
	FF_T_INT8	pathdest[FF_MAX_PATH];
	FF_T_INT8	tmpsrc[FF_MAX_PATH];
	FF_T_INT8	tmpdest[FF_MAX_PATH];
	FF_T_INT8	srcWild[FF_MAX_PATH];
	FF_T_INT8	destWild[FF_MAX_PATH];
	FF_DIRENT	mydir;
	FF_ERROR	Tester;
	FF_T_INT8	*p;

	if(argc != 3) {
		printf("Copy command is invalid\n");
		return 0;
	}

	ProcessPath(pathsrc, argv[1], pEnv);
	ProcessPath(pathdest, argv[2], pEnv);

	p =  strstr(pathdest, "*");
	if(!p) {
		printf("Missing Wildcard!\n");
		return 0;
	}
	strcpy(destWild, p);
	*p = '\0';

	p =  strstr(pathsrc, "*");
	if(!p) {
		printf("Missing Wildcard!\n");
		return 0;
	}
	strcpy(srcWild, p);
	*p = '\0';

	if(!FF_strmatch(srcWild, destWild, 0)) {
		printf("Source and Destination Wildcards do not match!\n");
		return 0;
	}
	
	Tester = FF_FindFirst(pEnv->pIoman, &mydir, pathsrc);

	while(!Tester) {
		if(wildCompare(srcWild, mydir.FileName)) {
			// Do Copy!
			if(!FF_strmatch(mydir.FileName, ".", 0) && !FF_strmatch(mydir.FileName, "..", 0)) {
				printf("Copying file %s\n", mydir.FileName);
				strcpy(tmpsrc, pathsrc);
				strcat(tmpsrc, mydir.FileName);
				strcpy(tmpdest, pathdest);
				strcat(tmpdest, mydir.FileName);
				if(filecopy(tmpsrc, tmpdest, pEnv) == FF_ERR_FILE_OBJECT_IS_A_DIR) {
					// Recurse through a dir copy with the same wildcards.
					// Make the DIR etc.
					FF_MkDir(pEnv->pIoman, tmpdest);
				}
			}
		}

		Tester = FF_FindNext(pEnv->pIoman, &mydir);
	}

	return 0;
}

int cp_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	FF_IOMAN *pIoman = pEnv->pIoman;
	FF_FILE *fSource, *fDest;
	FF_ERROR Error;

	FF_T_INT8 path[FF_MAX_PATH];
	FF_T_UINT8 copybuf[COPY_BUFFER_SIZE];

	FF_T_SINT32	BytesRead;

	LARGE_INTEGER ticksPerSecond;
	LARGE_INTEGER start_ticks, end_ticks, cputime;
	float time, transferRate;

	QueryPerformanceFrequency(&ticksPerSecond);
	
	if(argc == 3) {
		if(strstr(argv[1], "*") || strstr(argv[2], "*")) {
			return wildcopy(argc, argv, pEnv);
		}
		ProcessPath(path, argv[1], pEnv);

		fSource = FF_Open(pIoman, path, FF_MODE_READ, &Error);

		if(fSource) {
			ProcessPath(path, argv[2], pEnv);

			fDest = FF_Open(pIoman, path, FF_GetModeBits("w"), &Error);
			if(fDest) {
				// Do the copy
				QueryPerformanceCounter(&start_ticks);   
				QueryPerformanceCounter(&end_ticks);   
				do{
					BytesRead = FF_Read(fSource, COPY_BUFFER_SIZE, 1, (FF_T_UINT8 *)copybuf);
					FF_Write(fDest, BytesRead, 1, (FF_T_UINT8 *) copybuf);
					QueryPerformanceCounter(&end_ticks); 
					cputime.QuadPart = end_ticks.QuadPart - start_ticks.QuadPart;
					time = ((float)cputime.QuadPart/(float)ticksPerSecond.QuadPart);
					transferRate = (fSource->FilePointer / time) / 1024;
					printf("%3.0f%% - %10ld Bytes Copied, %7.2f Kb/S\r", ((float)((float)fSource->FilePointer/(float)fSource->Filesize) * 100), fSource->FilePointer, transferRate);
				}while(BytesRead > 0);
				printf("%3.0f%% - %10ld Bytes Copied, %7.2f Kb/S\n", ((float)((float)fSource->FilePointer/(float)fSource->Filesize) * 100), fSource->FilePointer, transferRate);		

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
	{"Generic or Unknown Error",										-1},
	{"Copies the specified file to the specified location.",			FFT_COMMAND_DESCRIPTION},
	{ NULL }
};


/**
 *	@brief	Export copy command, (Copies files from FullFAT to Windows)
 **/
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
		fSource = FF_Open(pIoman, path, FF_MODE_READ, &Error);
		if(fSource) {
			fDest = fopen(argv[2], "wb");
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
					printf("%3.0f%% - %10ld Bytes Copied, %7.2f Kb/S\r", ((float)((float)fSource->FilePointer/(float)fSource->Filesize) * 100), fSource->FilePointer, transferRate);
				}while(BytesRead > 0);
				printf("%3.0f%% - %10ld Bytes Copied, %7.2f Kb/S\n", ((float)((float)fSource->FilePointer/(float)fSource->Filesize) * 100), fSource->FilePointer, transferRate);		

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
	{"Generic or Unknown Error",							-1},
	{"Copies a FullFAT file to the Hard-disk drive.",		FFT_COMMAND_DESCRIPTION},
	{ NULL }
};


/**
 *	@brief	Import copy command, copies a windows file into FullFAT
 **/
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
			fDest = FF_Open(pIoman, path, FF_MODE_WRITE | FF_MODE_CREATE | FF_MODE_TRUNCATE, &Error);
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
					printf("%3.0f%% - %10ld Bytes Copied, %7.2f Kb/S\r", ((float)((float)ftell(fSource)/(float)SourceSize) * 100), ftell(fSource), transferRate);
				}while(BytesRead > 0);
				printf("%3.0f%% - %10ld Bytes Copied, %7.2f Kb/S\n", ((float)((float)ftell(fSource)/(float)SourceSize) * 100), ftell(fSource), transferRate);	

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
	{"Generic or Unknown Error",							-1},
	{"Copies a file from the Hard-disk to the FullFAT partition.",	FFT_COMMAND_DESCRIPTION},
	{ NULL }
};

/**
 *	@brief	A simple command for making dirs.
 **/
int mkdir_cmd(int argc, char **argv, FF_ENVIRONMENT *pEv) {
	
	FF_T_INT8	path[FF_MAX_PATH];
	FF_ERROR	Error;

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
	{"Generic or Unknown Error",							-1},
	{"Creates directories.",	FFT_COMMAND_DESCRIPTION},
	{ NULL }
};


/**
 *	@brief	A simple command for displaying some info.
 *			Displays some information about the currently mounted parttion.
 *
 *	The accesing of the FF_PARTITION object is not recommended as this object
 *	is subject to change in future versions.
 *
 **/
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
#ifdef FF_64_NUM_SUPPORT
		printf("Volume Size: \t\t\t%llu (%d MB)\n", FF_GetVolumeSize(pIoman), (unsigned int) (FF_GetVolumeSize(pIoman) / 1048576));
		printf("Volume Free: \t\t\t%llu (%d MB)\n", FF_GetFreeSize(pIoman), (unsigned int) (FF_GetFreeSize(pIoman) / 1048576));
#else
		printf("Volume Size: \t\t\t%lu (%d MB)\n", FF_GetVolumeSize(pIoman), (unsigned int) (FF_GetVolumeSize(pIoman) / 1048576));
		printf("Volume Free: \t\t\t%lu (%d MB)\n", FF_GetFreeSize(pIoman), (unsigned int) (FF_GetFreeSize(pIoman) / 1048576));
#endif
	} else {
		printf("Usage: %s\n", argv[0]);
	}
	return 0;
}
const FFT_ERR_TABLE infoInfo[] =
{
	{"Generic or Unknown Error",							-1},
	{"Displays information about the currently mounted partition.",	FFT_COMMAND_DESCRIPTION},
	{ NULL }
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

		f = FF_Open(pEnv->pIoman, path, FF_MODE_READ, &Error);
		if(f) {
			printf("//---------- START OF FILE\n");
			while(!FF_isEOF(f)) {
				c = FF_GetC(f);
				if(c >= 0) {
					printf("%c", (FF_T_INT8) c);
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
	{"Unknown or Generic Error",				-1},	// Generic Error must always be the first in the table.
	{"Types out the specified file.",			FFT_COMMAND_DESCRIPTION},
	{"File open failed. (File not found?)",		-2},
	{"Error while reading from device!",		-3},
	{ NULL }
};


/*
	A View command to type out the contents of a file using FullFAT.
*/
int rm_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	
	FF_ERROR	Error;
	FF_DIRENT	mydir;	// Used to detect if its a file or folder.
	FF_T_INT8	path[FF_MAX_PATH];
	FF_T_INT8	realpath[FF_MAX_PATH];
	FF_T_INT8	*pName;
	int i;

	if(argc == 2) {

		ProcessPath(path, argv[1], pEnv);

		for(i = strlen(path); ; i--) {
			if(path[i] == '\\' || path[i] == '/') {
				pName = &path[i + 1];
				break;
			}
		}

		memcpy(realpath, path, i + 1);
		realpath[i+1] = '\0';

		Error = FF_FindFirst(pEnv->pIoman, &mydir, realpath);

		while(!FF_strmatch(mydir.FileName, pName, 0)) {
			Error = FF_FindNext(pEnv->pIoman, &mydir);
			if(Error) {	// File
				printf("File or Folder not found!\n");
				return 0;
			}
		}

		if(mydir.Attrib & FF_FAT_ATTR_DIR) {
			Error = FF_RmDir(pEnv->pIoman, path);
		} else {
			Error = FF_RmFile(pEnv->pIoman, path);
		}

		if(Error) {
			printf("Could not remove file or folder: %s\n", FF_GetErrMessage(Error));
		}


	} else {
		printf("Usage: %s [filename]\n", argv[0]);
	}
	return 0;
}
const FFT_ERR_TABLE rmInfo[] =
{											// This demonstrates how FFTerm can provide useful information about specific command failure codes.
	{"Unknown or Generic Error",				-1},	// Generic Error must always be the first in the table.
	{"Deletes the specified file or folder.",	FFT_COMMAND_DESCRIPTION},
	{ NULL }
};


int move_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	FF_T_INT8	src[FF_MAX_PATH];
	FF_T_INT8	dest[FF_MAX_PATH];
	FF_ERROR	Error;

	if(argc == 3) {
		ProcessPath(src, argv[1], pEnv);
		ProcessPath(dest, argv[2], pEnv);

		Error = FF_Move(pEnv->pIoman, src, dest);
		if(Error) {
			printf("Error: %s\n", FF_GetErrMessage(Error));
		}
	} else {
		printf("Usage: %s [Source Path] [Destination Path]\n", argv[0]);
	}

	return 0;
}
const FFT_ERR_TABLE moveInfo[] =
{														// This demonstrates how FFTerm can provide useful information about specific command failure codes.
	{"Unknown or Generic Error",				-1},	// Generic Error must always be the first in the table.
	{"Moves a specified file or folder.",		FFT_COMMAND_DESCRIPTION},
	{ NULL }
};


int mkimg_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	
	FILE		*fDest;
	FF_T_UINT8	*buf;
	FF_T_UINT32	BS = 2048;

	FF_T_UINT32	read,x,i = 0;

	if(argc == 2) {
		fDest = fopen(argv[1], "wb");
		if(fDest) {

			buf = (FF_T_UINT8 *) malloc(pEnv->pIoman->pPartition->BlkSize * BS);

			if(!buf) {
				return 0;
			}

			while(i < pEnv->pIoman->pPartition->TotalSectors) {
				if(pEnv->pIoman->pPartition->TotalSectors - i < BS) {
					BS = pEnv->pIoman->pPartition->TotalSectors - i;
				}
				read = pEnv->pIoman->pBlkDevice->fnReadBlocks(buf, i, BS, pEnv->pIoman->pBlkDevice->pParam);
				i += read;
				x = fwrite(buf, pEnv->pIoman->pPartition->BlkSize, read, fDest);

				if(!read) {	// not reading anymore!
					break;
				}

				printf("%d%% Complete. (%ld of %ld Sectors read)\r", (int)(((float)i / (float)pEnv->pIoman->pPartition->TotalSectors) * (float)100.0), i,  pEnv->pIoman->pPartition->TotalSectors);
			}

			printf("\n");

			fclose(fDest);
			free(buf);

		} else {
			printf("Couldn't open the destination file!\n");
		}
	} else {
		printf("Usage: %s [filename.img]\n", argv[0]);
	}
	
	return 0;
}
const FFT_ERR_TABLE mkimgInfo[] =
{
	{"Unknown or Generic Error",				-1},	// Generic Error must always be the first in the table.
	{"Takes an image of the mounted volume.",	FFT_COMMAND_DESCRIPTION},
	{ NULL }
};


int mkfile_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	
	FF_FILE *f;
	FF_T_UINT32	Bytes;
	FF_T_UINT32	BytesWritten = 0;
	FF_T_UINT32 ElementSize = 0, Elements = 0, Multiplier = 0;
	FF_T_UINT32 IntBuffer[4096];	// 16Kb of Integers!
	FF_T_UINT32	i = 0, x;
	FF_T_INT8	path[FF_MAX_PATH];
	FF_ERROR	Error;

	LARGE_INTEGER ticksPerSecond;
	LARGE_INTEGER start_ticks, end_ticks, cputime;
	float time, transferRate = 0.0;

	cputime.QuadPart = 0;

	QueryPerformanceFrequency(&ticksPerSecond); 

	if(argc == 5) {
		sscanf(argv[1], "%lu", &ElementSize);
		if(!ElementSize) {
			printf("Invalid Element Size!\n");
			return 0;
		}

		sscanf(argv[2], "%lu", &Elements);
		if(!Elements) {
			printf("Invalid Number of Elements\n");
			return 0;
		}

		sscanf(argv[3], "%lu", &Multiplier);
		if(!Multiplier) {
			printf("Invalid Multiplier\n");
			return 0;
		}

		Bytes = ElementSize * Elements * Multiplier;

		printf("Creating file of size %lu Bytes (%0.2f MB) (%0.3f GB)\n", Bytes, (float)((float)Bytes / 1048576.0), (float)(((float)Bytes / 1048576.0)/1024.0));
		
		ProcessPath(path, argv[4], pEnv);

		f = FF_Open(pEnv->pIoman, path, FF_GetModeBits("wb"), &Error);

		if(f) {
			while(Bytes) {
				
				for(x = 0; x < 4096; x++) {
					IntBuffer[x] = i++;
				}
				
				QueryPerformanceCounter(&start_ticks); 
				if(Bytes >= (4096 * 4)) {
					BytesWritten += 4096 * 4;
					Bytes -= FF_Write(f, 1, 4096 * 4, (FF_T_UINT8 *) IntBuffer);
				} else {
					BytesWritten += Bytes;
					Bytes -= FF_Write(f, 1, Bytes, (FF_T_UINT8 *) IntBuffer);
				}
				QueryPerformanceCounter(&end_ticks);
				
				cputime.QuadPart += (end_ticks.QuadPart - start_ticks.QuadPart);
				time = ((float)cputime.QuadPart/(float)ticksPerSecond.QuadPart);
				transferRate = (BytesWritten / time) / 1024;

				printf("Written %0.2f MB (%7.2f KB/s)\r", (float) ((float)BytesWritten / 1048576.0), transferRate);
			}

			printf("Written %0.2f MB (%7.2f KB/s)\n", (float) ((float)BytesWritten / 1048576.0), transferRate);

			FF_Close(f);
		} else {
			printf("Error opening file: %s\n", FF_GetErrMessage(Error));
		}
		
	} else {
		printf("Generates a File filled with 32-bit integers.\n\n");
		printf("Usage: %s [Element Size] [Elements] [Multiplier] [filename]\n\n", argv[0]);
		printf("E.g. a 1Mb File, \tFullFAT\\>%s 1024\t 1024\t 1\t 1m.dat\n", argv[0]);
		printf("E.g. a 2Mb File, \tFullFAT\\>%s 1024\t 1024\t 2\t 2m.dat\n", argv[0]);
		printf("E.g. a 2Mb File, \tFullFAT\\>%s 1024\t 1024\t 10\t 10m.dat\n", argv[0]);
		printf("E.g. a 2Mb File, \tFullFAT\\>%s 1024\t 1024\t 100\t 100m.dat\n\n", argv[0]);
	}

	return 0;
}
const FFT_ERR_TABLE mkfileInfo[] =
{
	{"Unknown or Generic Error",				-1},	// Generic Error must always be the first in the table.
	{"Generates a File of the specified size.",	FFT_COMMAND_DESCRIPTION},
	{ NULL }
};


int mkwinfile_cmd(int argc, char **argv) {
	
	FILE *f;
	FF_T_UINT32	Bytes;
	FF_T_UINT32 BytesWritten = 0;
	FF_T_UINT32 ElementSize = 0, Elements = 0, Multiplier = 0;
	FF_T_UINT32 IntBuffer[4096];	// 16Kb of Integers!
	FF_T_UINT32	i = 0, x;

	LARGE_INTEGER ticksPerSecond;
	LARGE_INTEGER start_ticks, end_ticks, cputime;
	float time, transferRate = 0;

	cputime.QuadPart = 0;

	QueryPerformanceFrequency(&ticksPerSecond); 

	if(argc == 5) {
		sscanf(argv[1], "%lu", &ElementSize);
		if(!ElementSize) {
			printf("Invalid Element Size!\n");
			return 0;
		}

		sscanf(argv[2], "%lu", &Elements);
		if(!Elements) {
			printf("Invalid Number of Elements\n");
			return 0;
		}

		sscanf(argv[3], "%lu", &Multiplier);
		if(!Multiplier) {
			printf("Invalid Multiplier\n");
			return 0;
		}

		Bytes = ElementSize * Elements * Multiplier;

		printf("Creating file of size %lu Bytes (%0.2f MB) (%0.3f GB)\n", Bytes, (float)((float)Bytes / 1048576.0), (float)(((float)Bytes / 1048576.0)/1024.0));
		
		f = fopen(argv[4], "wb");

		if(f) {
			while(Bytes) {
				for(x = 0; x < 4096; x++) {
					IntBuffer[x] = i++;
				}

				QueryPerformanceCounter(&start_ticks); 
				if(Bytes >= (4096 * 4)) {
					Bytes -= fwrite(IntBuffer, 1, 4096 * 4, f);
					BytesWritten += 4096 * 4;
				} else {
					BytesWritten += Bytes;
					Bytes -= fwrite(IntBuffer, 1, Bytes, f);
				}
				QueryPerformanceCounter(&end_ticks);

				cputime.QuadPart += (end_ticks.QuadPart - start_ticks.QuadPart);
				time = ((float)cputime.QuadPart/(float)ticksPerSecond.QuadPart);
				transferRate = (BytesWritten / time) / 1024;

				printf("Written %0.2f MB (%7.2f KB/s)\r", (float) ((float)BytesWritten / 1048576.0), transferRate);
			}

			printf("Written %0.2f MB (%7.2f KB/s)\n", (float) ((float)BytesWritten / 1048576.0), transferRate);

			fclose(f);
		} else {
			printf("Error opening file\n");
		}
		
	} else {
		printf("Generates a Windows File filled with 32-bit integers.\n\n");
		printf("Usage: %s [Element Size] [Elements] [Multiplier] [filename]\n\n", argv[0]);
		printf("E.g. a 1Mb File, \tFullFAT\\>%s 1024\t 1024\t 1\t 1m.dat\n", argv[0]);
		printf("E.g. a 2Mb File, \tFullFAT\\>%s 1024\t 1024\t 2\t 2m.dat\n", argv[0]);
		printf("E.g. a 2Mb File, \tFullFAT\\>%s 1024\t 1024\t 10\t 10m.dat\n", argv[0]);
		printf("E.g. a 2Mb File, \tFullFAT\\>%s 1024\t 1024\t 100\t 100m.dat\n\n", argv[0]);
	}

	return 0;
}
const FFT_ERR_TABLE mkwinfileInfo[] =
{
	{"Unknown or Generic Error",								-1},	// Generic Error must always be the first in the table.
	{"Generates a File of the specified size. (On Windows).",	FFT_COMMAND_DESCRIPTION},
	{ NULL }
};

/**
 *	@brief	A simple command for breaking out of the FFTerm Environment.
 *			Allows commands to be executed on the underlying operating system.
 **/
int run_cmd(int argc, char **argv) {
	char cmd[2600] = "";
	int i;
	if(argc > 1) {
		for (i = 1; i < argc; i++) {
			if(strstr(argv[i], " ")) {
				strcat(cmd, "\"");
			}
			strcat(cmd, argv[i]);
			
			if(strstr(argv[i], " ")) {
				strcat(cmd, "\"");
			}
			strcat(cmd, " ");			// Ensure a space between each command line argument.
		}
		i = system(cmd);
		printf("\n");

		return i;
	} else {
		printf("Usage: %s [command line]\n", argv[0]);
	}
	return 0;
}
const FFT_ERR_TABLE runInfo[] =
{
	{"Unknown or Generic Error",					-1},	// Generic Error must always be the first in the table.
	{"Breaks out of the FFTerm environment and executes a command.",	FFT_COMMAND_DESCRIPTION},
	{ NULL }
};

/**
 *	@brief	A simple command for displaying the date.
 *			This command tests FullFAT time support is working.
 **/
int date_cmd(int argc, char **argv) {
#ifdef FF_TIME_SUPPORT
	FF_SYSTEMTIME Time;
#endif

	if(argc == 1) {
#ifdef FF_TIME_SUPPORT
		FF_GetSystemTime(&Time);
		printf("The current date is: %02d.%02d.%02d\n\n", Time.Day, Time.Month, Time.Year);
#else
		printf("Date Support not built. Rebuild FullFAT with FF_TIME_SUPPORT enabled.\n");
#endif

	} else {
		printf("Usage: %s\n", argv[0]);
	}
	return 0;
}
const FFT_ERR_TABLE dateInfo[] =
{
	{"Unknown or Generic Error",					-1},	// Generic Error must always be the first in the table.
	{"Displays the current date.",	FFT_COMMAND_DESCRIPTION},
	{ NULL }
};


/**
 *	@brief	A simple command for displaying the time.
 *			This command tests FullFAT time support is working.
 **/
int time_cmd(int argc, char **argv) {
#ifdef FF_TIME_SUPPORT
	FF_SYSTEMTIME Time;
#endif

	if(argc == 1) {
#ifdef FF_TIME_SUPPORT
		FF_GetSystemTime(&Time);
		printf("The current time is: %02d:%02d:%02d\n\n", Time.Hour, Time.Minute, Time.Second);
#else
		printf("Time Support not built. Rebuild FullFAT with FF_TIME_SUPPORT enabled.\n");
#endif

	} else {
		printf("Usage: %s\n", argv[0]);
	}
	return 0;
}
const FFT_ERR_TABLE timeInfo[] =
{
	{"Unknown or Generic Error",					-1},	// Generic Error must always be the first in the table.
	{"Displays the current time.",	FFT_COMMAND_DESCRIPTION},
	{ NULL }
};


/**
 *	@brief	A simple command for displaying all the possible physical drives that FullFAT could use.
 *			This command is unimplemented for 1.0.0.
 **/
int drivelist_cmd(int argc, char **argv) {
	TCHAR	Volume[MAX_PATH];
	HANDLE	hSearch = FindFirstVolume(Volume, MAX_PATH);
	HANDLE	hVolume;
	DWORD Error;

	argc;
	argv;

	if(hSearch) {
		hVolume = CreateFile(&Volume[0], 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING , 0, NULL);
		
		Error = GetLastError();

		CloseHandle(hVolume);
	}

	CloseHandle(hSearch);

	return 0;

}
const FFT_ERR_TABLE drivelistInfo[] =
{
	{"Unknown or Generic Error",					-1},	// Generic Error must always be the first in the table.
	{"Lists all available physical drives (Windows).",	FFT_COMMAND_DESCRIPTION},
	{ NULL }
};

#define HEX_BUF	16


/**
 *	@brief	A basic Hexview application.
 *			Allows file's to be viewed as Hex.
 **/
int hexview_cmd(int argc, char **argv) {
	
	FILE *f;
	char buffer[HEX_BUF];
//	char lines[25][81]; // Screen Buffer
	int read;
	int i;

	printf(" HexView 0.2 for FFTerm and FullFAT\n");
	printf(" By James Walmsley\n");
	printf("--------------------------------------------------------\n");
	printf("                         LOADING                        \n");
	Sleep(1000);

	if(argc == 2) {
		f = fopen(argv[1], "rb");
		if(f) {
			do{
				read = fread(buffer, 1, HEX_BUF, f);
				for(i = 0; i < read; i++) {
					printf("%02X", buffer[i]);
				}
				if(read < HEX_BUF) {
					for(i = 0; i < (HEX_BUF - read); i++) {
						printf("  ");
					}
				}
				printf("\t");
				for(i = 0; i < read; i++) {
					if((buffer[i] >= 32 && buffer[i] <= 255)) {
						printf("%c", buffer[i]);
					} else {
						printf(".");
					}
				}
				printf("\n");

			}while(read);
			fclose(f);
		} else {
			printf("Couldn't open file!\n");
		}
	} else {
		printf("No file specified!\n");
	}

	return 0;
}
const FFT_ERR_TABLE hexviewInfo[] =
{
	{"Unknown or Generic Error",					-1},	// Generic Error must always be the first in the table.
	{"Simple Hexview application.",					FFT_COMMAND_DESCRIPTION},
	{ NULL }
};


/**
 *	@brief	A simple command for Killing all test threads and exiting the console.
 *			This command sends the kill signal to FFTerm.
 **/
int exit_cmd(int argc, char **argv) {
	if(argc) {
		//KillAllThreads();
		return FFT_KILL_CONSOLE;
	}
	printf("Error in the %s command\n", argv[0]);
	return -1;
}
const FFT_ERR_TABLE exitInfo[] =
{
	{"Unknown or Generic Error",					-1},	// Generic Error must always be the first in the table.
	{"Terminates the FullFAT console and demo.",	FFT_COMMAND_DESCRIPTION},
	{ NULL }
};


//---------- THE FOLLOWING FUNCTIONS FORM A SMALL LIBRARY TO HELP THE COMMANDS FOUND ABOVE



/*
	This routine removes all relative ..\ from a path.
	It's probably not the best, but it works. 

	Its based on some old code I wrote a long time ago.
*/
static void ExpandPath(char *acPath) {

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
 *	@private
 **/
static FF_T_BOOL wildCompare(const char * pszWildCard, const char * pszString) {
    /* Check to see if the string contains the wild card */
    if (!memchr(pszWildCard, '*', strlen(pszWildCard)))
    {
        /* if it does not then do a straight string compare */
        if (strcmp(pszWildCard, pszString))
        {
            return FF_FALSE;
        }
    }
    else
    {
        while ((*pszWildCard)
        &&     (*pszString))
        {
            /* Test for the wild card */
            if (*pszWildCard == '*')
            {
                /* Eat more than one */
                while (*pszWildCard == '*')
                {
                    pszWildCard++;
                }
                /* If there are more chars in the string */
                if (*pszWildCard)
                {
                    /* Search for the next char */
                    pszString = memchr(pszString, (int)*pszWildCard,  strlen(pszString));
                    /* if it does not exist then the strings don't match */
                    if (!pszString)
                    {
                        return FF_FALSE;
                    }
                    
                }
                else
                {
                    if (*pszWildCard)
                    {
                        /* continue */
                        break;      
                    }
                    else
                    {
                        return FF_TRUE;
                    }
                }
            }
            else 
            {
                /* Fail if they don't match */
                if (*pszWildCard != *pszString)
                {
                    return FF_FALSE;
                }
            }
            /* Bump both pointers */
            pszWildCard++;
            pszString++;
        }
        /* fail if different lengths */
        if (*pszWildCard != *pszString)
        {
            return FF_FALSE;
        }
    }

    return FF_TRUE;
}
