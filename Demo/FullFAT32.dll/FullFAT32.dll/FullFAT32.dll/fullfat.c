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
#include "../../../../src/fullfat.h"									// Include everything required for FullFAT.
#include <ffterm.h>										// Include the FFTerm project header.
#include "../../../../Drivers/Windows/blkdev_win32.h"		// Prototypes for our Windows 32-bit driver.

/*
#define PARTITION_NUMBER	0							// FullFAT can mount primary partitions only. Specified at Runtime.

int main(void) {
	
	FFT_CONSOLE		*pConsole;							// FFTerm Console Pointer.										
	FF_ERROR		Error = FF_ERR_NONE;				// ERROR code value.
	FF_IOMAN		*pIoman;							// FullFAT I/O Manager Pointer, to be created.
	FF_ENVIRONMENT	Env;								// Special Micro-Environment for the Demo (working Directory etc). See cmd.h.
	HANDLE			hDisk;								// FILE Stream pointer for Windows FullFAT driver. (Device HANDLE).

	//----------- Initialise the environment
	Env.pIoman = NULL;									// Initialise the FullFAT I/O Manager to NULL.
	strcpy(Env.WorkingDir, "\\");						// Reset the Working Directory to the root folder.

	// Opens a HANDLE to a Windows Disk, or Drive Image, the second parameter is the blocksize,
	// and is only used in conjunction with DriveImage files.
	hDisk = fnOpen("c:\\FullFAT.img", 512);
	
	// When opening a physical drive handle, the blocksize is ignored, and detected automatically.
	//hDisk = fnOpen("\\\\.\\PHYSICALDRIVE4", 0);

	if(hDisk) {
		//---------- Create FullFAT IO Manager
		pIoman = FF_CreateIOMAN(NULL, 4096, GetBlockSize(hDisk), &Error);	// Using the BlockSize from the Device Driver (see blkdev_win32.c)

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

			//---------- Create the Console. (FFTerm - FullFAT Terminal).
			pConsole = FFTerm_CreateConsole("FullFAT>", stdin, stdout, &Error);					// Create a console with a "FullFAT> prompt.

			if(pConsole) {
				FFTerm_SetConsoleMode(pConsole, 0);
				//---------- Add Commands to the console.
				FFTerm_AddExCmd	(pConsole, "prompt",	(FFT_FN_COMMAND_EX) cmd_prompt,		promptInfo,		&Env);	// Dynamic command prompt (prompt is a reserved command name).
				FFTerm_AddExCmd	(pConsole, "pwd",		(FFT_FN_COMMAND_EX) pwd_cmd,		pwdInfo,		&Env);	// See cmd.c for their implementations.
				FFTerm_AddExCmd	(pConsole, "ls",		(FFT_FN_COMMAND_EX) ls_cmd,			lsInfo,			&Env);	// Directory Listing Command.
				FFTerm_AddExCmd	(pConsole, "dir",		(FFT_FN_COMMAND_EX) ls_cmd,			lsInfo,			&Env);	// Directory Listing Command.
				FFTerm_AddExCmd	(pConsole, "cd",		(FFT_FN_COMMAND_EX) cd_cmd,			cdInfo,			&Env);	// Change Directory Command.
				FFTerm_AddExCmd	(pConsole, "cp",		(FFT_FN_COMMAND_EX) cp_cmd,			cpInfo,			&Env);	// Copy command (FullFAT file to FullFAT file).
				FFTerm_AddExCmd	(pConsole, "copy",		(FFT_FN_COMMAND_EX) cp_cmd,			cpInfo,			&Env);	// Copy command (FullFAT file to FullFAT file).
				FFTerm_AddExCmd	(pConsole, "icp",		(FFT_FN_COMMAND_EX) icp_cmd,		icpInfo,		&Env);	// Copy command (Windows file to FullFAT file).
				FFTerm_AddExCmd	(pConsole, "xcp",		(FFT_FN_COMMAND_EX) xcp_cmd,		xcpInfo,		&Env);	// Copy command (FullFAT file to Windows file).
				FFTerm_AddExCmd	(pConsole, "md5",		(FFT_FN_COMMAND_EX) md5_cmd,		md5Info,		&Env);	// MD5 Data Hashing command.
				FFTerm_AddExCmd	(pConsole, "mkdir",		(FFT_FN_COMMAND_EX) mkdir_cmd,		mkdirInfo,		&Env);	// Make directory command.
				FFTerm_AddExCmd	(pConsole, "info",		(FFT_FN_COMMAND_EX) info_cmd,		infoInfo,		&Env);	// Information command.
				FFTerm_AddExCmd	(pConsole, "view",		(FFT_FN_COMMAND_EX) view_cmd,		viewInfo,		&Env);	// View command, (types a file).
				FFTerm_AddExCmd	(pConsole, "type",		(FFT_FN_COMMAND_EX) view_cmd,		viewInfo,		&Env);	// View command, (types a file).
				FFTerm_AddExCmd	(pConsole, "rm",		(FFT_FN_COMMAND_EX) rm_cmd,			rmInfo,			&Env);	// Remove file or dir command.
				FFTerm_AddExCmd	(pConsole, "del",		(FFT_FN_COMMAND_EX) rm_cmd,			rmInfo,			&Env);	// Remove file or dir command.
				FFTerm_AddExCmd	(pConsole, "move",		(FFT_FN_COMMAND_EX) move_cmd,		moveInfo,		&Env);	// Move or rename a file or dir.
				FFTerm_AddExCmd	(pConsole, "rename",	(FFT_FN_COMMAND_EX) move_cmd,		moveInfo,		&Env);	// Move or rename a file or dir.
				FFTerm_AddExCmd	(pConsole, "mkimg",		(FFT_FN_COMMAND_EX) mkimg_cmd,		mkimgInfo,		&Env);	// Make image command, (makes a windows file image of the media).
				FFTerm_AddExCmd	(pConsole, "mkfile",	(FFT_FN_COMMAND_EX) mkfile_cmd,		mkfileInfo,		&Env);	// File generator command.
				FFTerm_AddCmd	(pConsole, "mkwinfile",	(FFT_FN_COMMAND)	mkwinfile_cmd,	mkwinfileInfo);			// File generator command (windows version).
				FFTerm_AddCmd	(pConsole, "md5win",	(FFT_FN_COMMAND)	md5win_cmd,		md5winInfo);			// Windows MD5 Command.
				FFTerm_AddCmd	(pConsole, "run",		(FFT_FN_COMMAND)	run_cmd,		runInfo);				// Special Run Command.
				FFTerm_AddCmd	(pConsole, "time",		(FFT_FN_COMMAND)	time_cmd,		timeInfo);				// Time Command.
				FFTerm_AddCmd	(pConsole, "date",		(FFT_FN_COMMAND)	date_cmd,		dateInfo);				// Date Command.
				FFTerm_AddCmd	(pConsole, "exit",		(FFT_FN_COMMAND)	exit_cmd,		exitInfo);				// Special Exit Command.
				FFTerm_AddCmd	(pConsole, "hexview",	(FFT_FN_COMMAND)	hexview_cmd,	hexviewInfo);			// File Hexviewer.
				FFTerm_AddCmd	(pConsole, "drivelist",	(FFT_FN_COMMAND)	drivelist_cmd,	drivelistInfo);			// List of available drives.
				
				// Special Thread IO commands
				FFTerm_AddExCmd(pConsole, "mkthread",	(FFT_FN_COMMAND_EX) createthread_cmd,	mkthreadInfo,	&Env);
				FFTerm_AddExCmd(pConsole, "tlist",		(FFT_FN_COMMAND_EX) listthreads_cmd,	listthreadsInfo,&Env);
				FFTerm_AddExCmd(pConsole, "tkill",		(FFT_FN_COMMAND_EX) killthread_cmd,		killthreadInfo,	&Env);
				
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
		printf("Could not open the I/O Block device\nError calling fopen() function. (Device (file) not found?)\n");
	}

	getchar();
	return -1;
}
*/