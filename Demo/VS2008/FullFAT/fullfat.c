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




#include "cmd.h"										// The Demo's Header File for shell commands.
#include "test_threads.h"
#include "../../../src/fullfat.h"						// Include everything required for FullFAT.
#include "../../../../FFTerm/src/FFTerm.h"				// Include the FFTerm project header.
#include "../../../Drivers/Windows/blkdev_win32.h"		// Prototypes for our Windows 32-bit driver.

#define PARTITION_NUMBER	0							// FullFAT can mount primary partitions only.

int main(void) {
	
	FFT_CONSOLE		*pConsole;							// FFTerm Console Pointer.										
	FF_ERROR		Error = FF_ERR_NONE;				// ERROR code value.
	FF_IOMAN		*pIoman;
	FF_ENVIRONMENT	Env;								// Special Micro-Environment for the Demo (working Directory etc).
	HANDLE			hDisk;								// FILE Stream pointer for Windows FullFAT driver. (Device HANDLE).

	//----------- Initialise the environment
	Env.pIoman = NULL;
	strcpy(Env.WorkingDir, "\\");

	// Open a File Stream for FullFAT's I/O driver to work on.

	hDisk = fnOpen("s:\\ramdisk.bin");
	//hDisk = fnOpen("\\\\.\\PHYSICALDRIVE3");

	if(hDisk) {
		//---------- Create FullFAT IO Manager
		pIoman = FF_CreateIOMAN(NULL, 4096, GetBlockSize(hDisk), &Error);	// Using the BlockSize from the Device Driver (see blkdev_win32.c)

		if(pIoman) {
			//---------- Register a Block Device with FullFAT.
			Error = FF_RegisterBlkDevice(pIoman, GetBlockSize(hDisk), (FF_WRITE_BLOCKS) fnWrite, (FF_READ_BLOCKS) fnRead, hDisk);
			if(Error) {
				printf("Error Registering Device\nFF_RegisterBlkDevice() function returned with Error %d.\nFullFAT says: %s\n", Error, FF_GetErrMessage(Error));				
			}

			//---------- Try to Mount the Partition with FullFAT.
			Error = FF_MountPartition(pIoman, PARTITION_NUMBER);
			if(Error) {
				if(hDisk) {
					fnClose(hDisk);
				}
				FF_DestroyIOMAN(pIoman);
				printf("FullFAT Couldn't mount the specified parition!\n");

				printf("FF_MountPartition() function returned with Error %d\nFullFAT says: %s\n", Error, FF_GetErrMessage(Error));
				getchar();
				return -1;
			}

			Env.pIoman = pIoman;

			//---------- Create the Console. (FFTerm - FullFAT Terminal).
			pConsole = FFTerm_CreateConsole("FullFAT>", stdin, stdout, &Error);					// Create a console with a "FullFAT> prompt.

			if(pConsole) {
				FFTerm_SetConsoleMode(pConsole, 0);
				//---------- Add Commands to the console.
				FFTerm_AddExCmd	(pConsole, "prompt",	cmd_prompt,		promptInfo,		&Env);	// Dynamic command prompt (prompt is a reserved command name).
				FFTerm_AddExCmd	(pConsole, "pwd",		pwd_cmd,		pwdInfo,		&Env);	// See cmd.c for their implementations.
				FFTerm_AddExCmd	(pConsole, "ls",		ls_cmd,			lsInfo,			&Env);	// Directory Listing Command
				FFTerm_AddExCmd	(pConsole, "cd",		cd_cmd,			cdInfo,			&Env);	// Change Directory Command
				FFTerm_AddExCmd	(pConsole, "cp",		cp_cmd,			cpInfo,			&Env);	// Copy command (FullFAT file to FullFAT file)
				FFTerm_AddExCmd	(pConsole, "icp",		icp_cmd,		icpInfo,		&Env);	// Copy command (Windows file to FullFAT file)
				FFTerm_AddExCmd	(pConsole, "xcp",		xcp_cmd,		xcpInfo,		&Env);	// Copy command (FullFAT file to Windows file)
				FFTerm_AddExCmd	(pConsole, "md5",		md5_cmd,		md5Info,		&Env);	// MD5 Data Hashing command.
				FFTerm_AddExCmd	(pConsole, "mkdir",		mkdir_cmd,		mkdirInfo,		&Env);	// Make directory command.
				FFTerm_AddExCmd	(pConsole, "info",		info_cmd,		infoInfo,		&Env);	// Information command.
				FFTerm_AddExCmd	(pConsole, "view",		view_cmd,		viewInfo,		&Env);	// View command, (types a file).
				FFTerm_AddExCmd	(pConsole, "rm",		rm_cmd,			rmInfo,			&Env);	// Remove file or dir command.
				FFTerm_AddExCmd	(pConsole, "mkimg",		mkimg_cmd,		mkimgInfo,		&Env);	// Make image command, (makes a windows file image of the media).
				FFTerm_AddExCmd	(pConsole, "mkfile",	mkfile_cmd,		mkfileInfo,		&Env);	// File generator command.
				FFTerm_AddCmd	(pConsole, "mkwinfile",	mkwinfile_cmd,	mkwinfileInfo);			// File generator command (windows version).
				FFTerm_AddCmd	(pConsole, "md5win",	md5win_cmd,		md5winInfo);			// Windows MD5 Command.
				FFTerm_AddCmd	(pConsole, "exit",		exit_cmd,		exitInfo);				// Special Exit Command.
				
				// Special Thread IO commands
				FFTerm_AddExCmd(pConsole, "mkthread",	createthread_cmd,	mkthreadInfo,	&Env);
				FFTerm_AddExCmd(pConsole, "tlist",		listthreads_cmd,	listthreadsInfo,&Env);
				FFTerm_AddExCmd(pConsole, "tkill",		killthread_cmd,		killthreadInfo,	&Env);
				
				//---------- Some test code used to test the FILE I/O Api.
				
				/*pF = FF_Open(pIoman, "\\test2.txt", FF_GetModeBits("a+"), &Error);
				for(i = 0; i < 1024; i++) {
					FF_PutC(pF, 'J');
				}
				FF_Close(pF);*/

				
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
		printf("Could not initialise FullFAT I/O Manager.\nError calling FF_CreateIOMAN() function.\nError Code %d\nFullFAT says: %s\n", Error, FF_GetErrMessage(Error));
	} else {
		printf("Could not open the I/O Block device\nError calling fopen() function. (Device (file) not found?)\n");
	}

	getchar();
	return -1;
}
