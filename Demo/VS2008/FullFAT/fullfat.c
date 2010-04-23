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


#include "../../cmd/commands.h"							// The Demo's Header File for shell commands.

#include "../../../src/fullfat.h"						// Include everything required for FullFAT.
#include "../../../../FFTerm/src/FFTerm.h"				// Include the FFTerm project header.
#include "../../../Drivers/Windows/blkdev_win32.h"		// Prototypes for our Windows 32-bit driver.

#include <locale.h>
#include <wchar.h>

#define PARTITION_NUMBER	0

int fds[3] = { 0, 1, 2 };

int *getfds() {
	return fds;
}


int main(void) {
	
	FFT_CONSOLE		*pConsole;							// FFTerm Console Pointer.										
	FF_ERROR		Error = FF_ERR_NONE;				// ERROR code value.
	FF_IOMAN		*pIoman;							// FullFAT I/O Manager Pointer, to be created.
	FF_ENVIRONMENT	Env;								// Special Micro-Environment for the Demo (working Directory etc). See cmd.h.
	HANDLE			hDisk;								// FILE Stream pointer for Windows FullFAT driver. (Device HANDLE).

	char			utf8string[30];

	wchar_t c[] = L"\\Grüßen_aus_Österreich";
	int i,y;

	
	//----------- Initialise the environment
	Env.pIoman = NULL;									// Initialise the FullFAT I/O Manager to NULL.
#ifdef FF_UNICODE_SUPPORT
	wcscpy(Env.WorkingDir, L"\\");
#else
	strcpy(Env.WorkingDir, "\\");						// Reset the Working Directory to the root folder.
#endif

	setlocale(LC_ALL, "");
	//wprintf(L"This is a Unicode String! Ich heiße Jämes!\n");

	//FF_wildcompare(L"*s?.c", L"test.c");

	//FF_Utf16ctoUtf8c(utf8string, L"𐀀", 10);
	
	//c = getwc(stdin);

	// Opens a HANDLE to a Windows Disk, or Drive Image, the second parameter is the blocksize,
	// and is only used in conjunction with DriveImage files.
	hDisk = fnOpen("c:\\new1gb.img.bak", 512);
	
	//hDisk = fnOpen("\\\\.\\E:", 0);	// Driver now expects a Volume, to allow Vista and Seven write access.

	// When opening a physical drive handle, the blocksize is ignored, and detected automatically.
	//hDisk = fnOpen("\\\\.\\PHYSICALDRIVE2", 0);

	if(hDisk) {
		//---------- Create FullFAT IO Manager
		pIoman = FF_CreateIOMAN(NULL, 8192, GetBlockSize(hDisk), &Error);	// Using the BlockSize from the Device Driver (see blkdev_win32.c)

		if(pIoman) {
			//---------- Register a Block Device with FullFAT.
			Error = FF_RegisterBlkDevice(pIoman, GetBlockSize(hDisk), (FF_WRITE_BLOCKS) fnWrite, (FF_READ_BLOCKS) fnRead, hDisk);
			if(Error) {
				printf("Error Registering Device\nFF_RegisterBlkDevice() function returned with Error %ld.\nFullFAT says: %s\n", Error, FF_GetErrMessage(Error));
			}

			//---------- Try to Mount the Partition with FullFAT.
			Error = FF_MountPartition(pIoman, PARTITION_NUMBER);
			if(Error) {
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
			
			i = 0;
			y = 0;
			while(c[y]) {
				i += FF_Utf16ctoUtf8c((FF_T_UINT8 *) &utf8string[i], &c[y], 30 - y);
				y += FF_GetUtf16SequenceLen(c[y]);
			}
			utf8string[i] = '\0';
//			FF_MkDir(pIoman, "\\A simple Ascii String");

			//---------- Create the Console. (FFTerm - FullFAT Terminal).
			pConsole = FFTerm_CreateConsole("FullFAT>", stdin, stdout, &Error);					// Create a console with a "FullFAT> prompt.

			if(pConsole) {
				FFTerm_SetConsoleMode(pConsole, 0);
				//---------- Add Commands to the console.

				FFTerm_AddExCmd(pConsole, "cd",		(FFT_FN_COMMAND_EX) cd_cmd, 	cdInfo,			&Env);
				FFTerm_AddExCmd(pConsole, "cp",		(FFT_FN_COMMAND_EX) cp_cmd, 	cpInfo,			&Env);
				FFTerm_AddExCmd(pConsole, "ls", 	(FFT_FN_COMMAND_EX) ls_cmd, 	lsInfo, 		&Env);
				FFTerm_AddExCmd(pConsole, "md5sum", (FFT_FN_COMMAND_EX) md5sum_cmd, md5sumInfo, 	&Env);
				FFTerm_AddExCmd(pConsole, "mkdir", 	(FFT_FN_COMMAND_EX) mkdir_cmd, 	mkdirInfo,		&Env);
				//FFTerm_AddExCmd(pConsole, "more", 	(FFT_FN_COMMAND_EX) more_cmd,	moreInfo, 		&Env);
				FFTerm_AddExCmd(pConsole, "prompt", (FFT_FN_COMMAND_EX) cmd_prompt, cmdpromptInfo, 	&Env);
				FFTerm_AddExCmd(pConsole, "pwd", 	(FFT_FN_COMMAND_EX)	pwd_cmd,	pwdInfo,		&Env);

				//---------- Start the console.
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

	getchar();
	return -1;
}
