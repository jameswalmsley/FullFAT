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


//#include "cmd.h"								// The Demo's Header File for shell commands.
//#include "test_threads.h"
#include "../cmd/commands.h"					// Include the commands.
#include "../cmd/cmd_helpers.h"
#include "../../src/fullfat.h"					// Include everything required for FullFAT.
#include "../../../ffterm/src/ffterm.h"			// Include the FFTerm project header.
#include "../../Drivers/Linux/blkdev_linux.h"	// Prototypes for our Windows 32-bit driver.
#include "../../../ffterm/Platforms/linux/FFTerm-Platform-linux.h"

#include <locale.h>

#define PARTITION_NUMBER	0					// FullFAT can mount primary partitions only. Specified at Runtime.

int lin_ls(int argc, char **argv) {
	DIR				*pDir;
	DIRENT			mydir;
	int				ret;

	if(argc != 2) {
		return 0;
	}

	pDir = FindFirstFile(argv[1], &mydir);
	
	if(pDir) {
		do {
			printf("%s", mydir.dir.d_name);
			if(S_ISDIR(mydir.itemInfo.st_mode)) {
				printf("\t <DIR>");
			}
			printf("\n");
			ret = FindNextFile(pDir, &mydir);
		} while(ret);
		FindClose(pDir);
		return 0;
	}

	return 0;
}

int totalReads	= 0;
int totalWrites = 0;

void read_inc(void) {
	totalReads += 1;
}

void write_inc(void) {
	totalWrites += 1;
}

int test(int argc, char **argv, FF_ENVIRONMENT *pEnv) {

	FF_FILE* ftestHandle;
	FF_ERROR fError;
	long bw = 0;
	int i, di;
	int secNumber = 0;
	md5_state_t 		state;
	md5_byte_t			digest[16];
	char szpHash[255];
	char				temp[3];

	FF_T_UINT8 SdCardBuf[1000];
	FF_T_UINT8	buf2[1000];

	for(i = 0; i < 1000; i++) {
		SdCardBuf[i] = (unsigned char) i;
	}

	md5_init(&state);
	
	ftestHandle = FF_Open(pEnv->pIoman,"\\test.dat", FF_GetModeBits("a+"), &fError);
	totalReads = 0;
	totalWrites = 0;
	for (secNumber = 0; secNumber < 1048; secNumber++)
	{
		fError = FF_Write(ftestHandle, 1, 1000, (FF_T_UINT8*) SdCardBuf);
		if(FF_isERR(fError)) {
			printf("Error: %s\n", FF_GetErrMessage(fError));
			break;
		}
		bw += fError;

		md5_append(&state, (const md5_byte_t *)SdCardBuf, 1000);
	}

	printf("reads: %d, writes %d\n", totalReads, totalWrites);

	FF_Close(ftestHandle);

	md5_finish(&state, digest);

	strcpy(szpHash, "");

	for (di = 0; di < 16; ++di) {
		sprintf(temp, "%02x", digest[di]);
		strcat(szpHash, temp);
	}

	printf("HASH: %s\n", szpHash);

	

	return 0;
}


int main(void) {
	
	FFT_CONSOLE		*pConsole;					// FFTerm Console Pointer.										
	FF_ERROR		Error = FF_ERR_NONE;		// ERROR code value.
	FF_IOMAN		*pIoman;					// FullFAT I/O Manager Pointer, to be created.
	FF_ENVIRONMENT	Env;						// Special Micro-Environment for the Demo (working Directory etc).
	BLK_DEV_LINUX	hDisk;						// FILE Stream pointer for Windows FullFAT driver. (Device HANDLE).

	//----------- Initialise the environment
	Env.pIoman = NULL;							// Initialise the FullFAT I/O Manager to NULL.
#ifdef FF_UNICODE_SUPPORT
	wcscpy(Env.WorkingDir, L"\\");				// Reset the Working Directory to the root folder.
#else
	strcpy(Env.WorkingDir, "\\");				// Reset the Working Directory to the root folder.
#endif

	// Opens a HANDLE to a Windows Disk, or Drive Image, the second parameter is the blocksize,
	// and is only used in conjunction with DriveImage files.
	//hDisk = fnOpen("c:\\FullFAT.img", 512);

	// Test unicode conversion routines!
	setlocale(LC_ALL, NULL);
	
	
	hDisk = fnOpen("ffimage.img", 512);	// Driver now expects a Volume, to allow Vista and Seven write access.

	// When opening a physical drive handle, the blocksize is ignored, and detected automatically.
	//hDisk = fnOpen("/dev/sdc", 512);

	if(hDisk) {
		//---------- Create FullFAT IO Manager
		pIoman = FF_CreateIOMAN(NULL, 4096, GetBlockSize(hDisk), &Error);	// Using the BlockSize from the Device Driver (see blkdev_win32.c)

		if(pIoman) {
			//---------- Register a Block Device with FullFAT.
			Error = FF_RegisterBlkDevice(pIoman, GetBlockSize(hDisk), (FF_WRITE_BLOCKS) fnWrite, (FF_READ_BLOCKS) fnRead, hDisk);
			if(FF_isERR(Error)) {
				printf("Error Registering Device\nFF_RegisterBlkDevice() function returned with Error %ld.\nFullFAT says: %s\n", Error, FF_GetErrMessage(Error));
			}

			printf("Attempt to format the first partition!\n");

			//FF_FormatPartition(pIoman, 0, 512);

			//---------- Try to Mount the Partition with FullFAT.
			Error = FF_MountPartition(pIoman, PARTITION_NUMBER);
			if(FF_isERR(Error)) {
				if(hDisk) {
					fnClose(hDisk);
				}
				FF_DestroyIOMAN(pIoman);
				printf("FullFAT Couldn't mount the specified parition!\n");

				printf("FF_MountPartition() function returned with Error %ld\nFullFAT says: %s\n", Error, FF_GetErrMessage(Error));
				getchar();
				return -1;
			}

			Env.pIoman = pIoman;

			//---------- Create the Console. (FFTerm - FullFAT Terminal).
			pConsole = FFTerm_CreateConsole("FullFAT>", stdin, stdout, &Error);					// Create a console with a "FullFAT> prompt.
			Env.pConsole = pConsole;

		
			if(pConsole) {

				FFTerm_RegisterPlatformSpecification(pConsole, linux_getSpec());

				//---------- Add Commands to the console.
				hook_commands(&Env);		// See Demo/cmd/hook.c
				
				//---------- Add Platform Specific commands.
				FFTerm_AddCmd(pConsole, "lslin", 	(FFT_FN_COMMAND) lin_ls, NULL);
				FFTerm_AddCmd(pConsole, "md5lin",	(FFT_FN_COMMAND)	md5sum_lin_cmd, md5sum_lin_Info);

				FFTerm_AddExCmd(pConsole, "test", (FFT_FN_COMMAND_EX) test, NULL, &Env);

				//---------- Start the console.
				FFTerm_StartConsole(pConsole);						// Start the console (looping till exit command).
				FF_UnmountPartition(pIoman);						// Unmount the mounted partition from FullFAT.
				FF_DestroyIOMAN(pIoman);							// Clean-up the FF_IOMAN Object.

				//---------- Final User Interaction
				printf("\n\nConsole Was Terminated, END OF Demonstration!, Press ENTER to exit!\n");
				getchar();

				fnClose(hDisk);

				return 0;
			}
			
			printf("Could not start the console: %s\n", FFTerm_GetErrMessage(Error));			// Problem starting the FFTerm console.
			getchar();
			return -1;
		}

		// FullFAT failed to initialise. Print some meaningful information from FullFAT itself, using the FF_GetErrMessage() function.
		printf("Could not initialise FullFAT I/O Manager.\nError calling FF_CreateIOMAN() function.\nError Code %ld\nFullFAT says: %s\n", Error, FF_GetErrMessage(Error));
	} else {
		printf("Could not open the I/O Block device\nError calling blockdeviceopen() function. (Device (file) not found?)\n");
	}

	getchar();
	return -1;
}
