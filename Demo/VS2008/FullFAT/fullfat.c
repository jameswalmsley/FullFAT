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


#include "../../cmd/commands.h"							// The Demo's Header File for shell commands.

#include "../../../src/fullfat.h"						// Include everything required for FullFAT.
#include "../../../../FFTerm/src/FFTerm.h"				// Include the FFTerm project header.
#include "../../../../FFTerm/Platforms/Win32/FFTerm-Platform-win32.h"
#include "../../../Drivers/Windows/blkdev_win32.h"		// Prototypes for our Windows 32-bit driver.

#include <locale.h>
#include <wchar.h>
#include <Windows.h>

#define PARTITION_NUMBER	0

int version(int argc, char **argv);
extern const FFT_ERR_TABLE versionInfo[];

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
		bw += FF_Write(ftestHandle, 1, 1000, (FF_T_UINT8*) SdCardBuf);

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
	
	FFT_CONSOLE		*pConsole;							// FFTerm Console Pointer.

	FF_ERROR		Error = FF_ERR_NONE;				// ERROR code value.
	FF_IOMAN		*pIoman;							// FullFAT I/O Manager Pointer, to be created.

	FF_ENVIRONMENT	Env;								// Special Micro-Environment for the Demo (working Directory etc). See cmd.h.

	HANDLE			hDisk;								// FILE Stream pointer for Windows FullFAT driver. (Device HANDLE).
	char buf[2600];
	
	//----------- Initialise the environment
	Env.pIoman = NULL;									// Initialise the FullFAT I/O Manager to NULL.
#ifdef FF_UNICODE_SUPPORT
	wcscpy(Env.WorkingDir, L"\\");
#else
	strcpy(Env.WorkingDir, "\\");						// Reset the Working Directory to the root folder.
#endif

	GetCurrentDirectoryA(2600, buf);
	printf("%s\n", buf);

	// Opens a HANDLE to a Windows Disk, or Drive Image, the second parameter is the blocksize,
	// and is only used in conjunction with DriveImage files.
	hDisk = fnOpen("ffimage.img", 512);

	//hDisk = fnOpen("\\\\.\\E:", 0);	// Driver now expects a Volume, to allow Vista and Seven write access.

	// When opening a physical drive handle, the blocksize is ignored, and detected automatically.
	//hDisk = fnOpen("\\\\.\\PHYSICALDRIVE2", 0);

	if(hDisk) {
		//---------- Create FullFAT IO Manager
		pIoman = FF_CreateIOMAN(NULL, 8192, GetBlockSize(hDisk), &Error);	// Using the BlockSize from the Device Driver (see blkdev_win32.c)

		if(pIoman) {
			//---------- Register a Block Device with FullFAT.
			Error = FF_RegisterBlkDevice(pIoman, GetBlockSize(hDisk), (FF_WRITE_BLOCKS) fnWrite, (FF_READ_BLOCKS) fnRead, hDisk);
			if(FF_isERR(Error)) {
				printf("Error Registering Device\nFF_RegisterBlkDevice() function returned with Error %ld.\nFullFAT says: %s\n", Error, FF_GetErrMessage(Error));
			}

			//FF_FormatPartition(pIoman, 0, 1024*64);

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
				FFTerm_RegisterPlatformSpecification(pConsole, w32_getSpec());

//				FFTerm_SetConsoleMode(pConsole, 0);
				//---------- Add Commands to the console.

				hook_commands(&Env);

				FFTerm_AddExCmd(pConsole, "test", (FFT_FN_COMMAND_EX) test, NULL, &Env);

//				fseek_test(&pIoman);

				//---------- Start the console.
				printf("Welcome to FullFAT %s. Type 'help' for a list of commands.\n", FF_VERSION);
				FFTerm_StartConsole(pConsole);						// Start the console (looping till exit command).
				FF_UnmountPartition(pIoman);						// Dis-mount the mounted partition from FullFAT.
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

	//getchar();
	return -1;
}

int version(int argc, char **argv) {
	printf("FullFAT version: %s\n", FF_VERSION);
	printf("Code base      : %s\n", FF_REVISION);
	argc = 0;		// Prevent some compiler warnings.
	argv = NULL;
	return 0;
}
const FFT_ERR_TABLE versionInfo[] =
{
	{"Generic or Unknown Error",										-1},
	{"Displays version information for this release.",					FFT_COMMAND_DESCRIPTION},
	{ NULL }
};
