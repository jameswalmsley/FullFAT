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


/*

	This is the main DLL wrapper source file.

*/
#include <fullfat.h>
#include <ffterm.h>
#include "../../../../Drivers/Windows/blkdev_win32.h"		// Prototypes for our Windows 32-bit driver.
#include "FFDLL_Error.h"									// Error Codes from the DLL.
#include "cmd.h"

#define DLL_EXPORT _declspec(dllexport)

FFT_CONSOLE *g_pConsole = NULL;								// Storing the objects globally.
FF_IOMAN	*g_pIoman = NULL;								// Allowing only a single instance per process.
HANDLE		 g_hDisk = NULL;

FF_ENVIRONMENT	g_Env;


/**
 * @public	Executes a command through the FFTerm command line interpreter.
 *
 * @param	commandLind		String of the command line.
 *
 * @return	The error code returned by the command.
 *
 **/
DLL_EXPORT int FFC_Exec(char *commandLine) {

	if(!g_pIoman) {
		return -1;		// Only possible to execute commands if an image is mounted!
	}
	
	if(g_pConsole) {
		printf("DEBUG cmdLine: %s\n", commandLine);
		return FFTerm_ExecCommand(g_pConsole, commandLine);
	}

	return -1;
}


/**
 * @public
 * @brief	Initialises the FFTerm Console object, and hooks all commands.
 *
 **/
DLL_EXPORT int FFC_InitTerminal() {
	FF_T_SINT32 slError = FFT_ERR_NONE;


	if(!g_pConsole) {

		g_Env.pIoman = NULL;

		g_pConsole = FFTerm_CreateConsole("FullFAT>", stdin, stdout, &slError);
		
		// Add in the commands!
		//---------- Add Commands to the console.
		FFTerm_AddExCmd	(g_pConsole, "prompt",	(FFT_FN_COMMAND_EX) cmd_prompt,		promptInfo,		&g_Env);	// Dynamic command prompt (prompt is a reserved command name).
		FFTerm_AddExCmd	(g_pConsole, "pwd",		(FFT_FN_COMMAND_EX) pwd_cmd,		pwdInfo,		&g_Env);	// See cmd.c for their implementations.
		FFTerm_AddExCmd	(g_pConsole, "ls",		(FFT_FN_COMMAND_EX) ls_cmd,			lsInfo,			&g_Env);	// Directory Listing Command.
		FFTerm_AddExCmd	(g_pConsole, "dir",		(FFT_FN_COMMAND_EX) ls_cmd,			lsInfo,			&g_Env);	// Directory Listing Command.
		//FFTerm_AddExCmd	(g_pConsole, "cd",		(FFT_FN_COMMAND_EX) cd_cmd,			cdInfo,			&g_Env);	// Change Directory Command.
		FFTerm_AddExCmd	(g_pConsole, "cp",		(FFT_FN_COMMAND_EX) cp_cmd,			cpInfo,			&g_Env);	// Copy command (FullFAT file to FullFAT file).
		FFTerm_AddExCmd	(g_pConsole, "copy",	(FFT_FN_COMMAND_EX) cp_cmd,			cpInfo,			&g_Env);	// Copy command (FullFAT file to FullFAT file).
		FFTerm_AddExCmd	(g_pConsole, "icp",		(FFT_FN_COMMAND_EX) icp_cmd,		icpInfo,		&g_Env);	// Copy command (Windows file to FullFAT file).
		FFTerm_AddExCmd	(g_pConsole, "xcp",		(FFT_FN_COMMAND_EX) xcp_cmd,		xcpInfo,		&g_Env);	// Copy command (FullFAT file to Windows file).
//		FFTerm_AddExCmd	(g_pConsole, "md5",		(FFT_FN_COMMAND_EX) md5_cmd,		md5Info,		&g_Env);	// MD5 Data Hashing command.
		FFTerm_AddExCmd	(g_pConsole, "mkdir",	(FFT_FN_COMMAND_EX) mkdir_cmd,		mkdirInfo,		&g_Env);	// Make directory command.
		FFTerm_AddExCmd	(g_pConsole, "info",	(FFT_FN_COMMAND_EX) info_cmd,		infoInfo,		&g_Env);	// Information command.
		FFTerm_AddExCmd	(g_pConsole, "view",	(FFT_FN_COMMAND_EX) view_cmd,		viewInfo,		&g_Env);	// View command, (types a file).
		FFTerm_AddExCmd	(g_pConsole, "type",	(FFT_FN_COMMAND_EX) view_cmd,		viewInfo,		&g_Env);	// View command, (types a file).
		FFTerm_AddExCmd	(g_pConsole, "rm",		(FFT_FN_COMMAND_EX) rm_cmd,			rmInfo,			&g_Env);	// Remove file or dir command.
		FFTerm_AddExCmd	(g_pConsole, "del",		(FFT_FN_COMMAND_EX) rm_cmd,			rmInfo,			&g_Env);	// Remove file or dir command.
		FFTerm_AddExCmd	(g_pConsole, "move",	(FFT_FN_COMMAND_EX) move_cmd,		moveInfo,		&g_Env);	// Move or rename a file or dir.
		FFTerm_AddExCmd	(g_pConsole, "rename",	(FFT_FN_COMMAND_EX) move_cmd,		moveInfo,		&g_Env);	// Move or rename a file or dir.
		FFTerm_AddExCmd	(g_pConsole, "mkimg",	(FFT_FN_COMMAND_EX) mkimg_cmd,		mkimgInfo,		&g_Env);	// Make image command, (makes a windows file image of the media).
		FFTerm_AddExCmd	(g_pConsole, "mkfile",	(FFT_FN_COMMAND_EX) mkfile_cmd,		mkfileInfo,		&g_Env);	// File generator command.
		FFTerm_AddCmd	(g_pConsole, "mkwinfile",(FFT_FN_COMMAND)	mkwinfile_cmd,	mkwinfileInfo);			// File generator command (windows version).
//		FFTerm_AddCmd	(g_pConsole, "md5win",	(FFT_FN_COMMAND)	md5win_cmd,		md5winInfo);			// Windows MD5 Command.
		//FFTerm_AddCmd	(g_pConsole, "run",		(FFT_FN_COMMAND)	run_cmd,		runInfo);				// Special Run Command.
		FFTerm_AddCmd	(g_pConsole, "time",	(FFT_FN_COMMAND)	time_cmd,		timeInfo);				// Time Command.
		FFTerm_AddCmd	(g_pConsole, "date",	(FFT_FN_COMMAND)	date_cmd,		dateInfo);				// Date Command.
		FFTerm_AddCmd	(g_pConsole, "exit",	(FFT_FN_COMMAND)	exit_cmd,		exitInfo);				// Special Exit Command.
		//FFTerm_AddCmd	(g_pConsole, "hexview",	(FFT_FN_COMMAND)	hexview_cmd,	hexviewInfo);			// File Hexviewer.
		//FFTerm_AddCmd	(g_pConsole, "drivelist",(FFT_FN_COMMAND)	drivelist_cmd,	drivelistInfo);			// List of available drives.

	}

	//FFTerm_StartConsole(g_pConsole);

	if(g_pConsole) {
		return -1;
	}

	return 0;	
}


/**
 * @public
 * @brief	Frees the memory used by the Terminal
 *
 *			"This is TODO! FFTerm needs a cleanup function adding to its API."
 *			"Therefore there is a memory leak!, Although its no problem because the memory will be freed
 *			when the calling process exits."
 **/
DLL_EXPORT int FFC_CleanupTerminal() {
	
	// FFTerm doesn't have a cleanup function! :( Will add later.
	
	g_pConsole = NULL;
	return 0;
}

void printError(FF_ERROR Error) {
	// Optionally print meaningful errors to the console (or your fstream buffer)
	// So that you can get more information about what went wrong in the DLL.
	printf("FullFAT Error: %s\n", FF_GetErrMessage(Error));
}


/**
 * @public
 * @brief	Mounts and Image with FullFAT.
 *
 *
 **/
DLL_EXPORT int FFC_MountImage(char *szFilename, unsigned long ulBlockSize, unsigned long ulPartitionNumber) {

	FF_ERROR Error = FF_ERR_NONE;

	if(g_pIoman || g_hDisk) {
		return FFD_ERR_IMAGE_ALREADY_MOUNTED;
	}

	g_hDisk = fnOpen(szFilename, ulBlockSize);

	if(!g_hDisk) {
		return FFD_ERR_COULD_NOT_OPEN_IMAGE;
	}
	
	g_pIoman = FF_CreateIOMAN(NULL, 8192, GetBlockSize(g_hDisk), &Error);	// Using the BlockSize from the Device Driver (see blkdev_win32.c)

	if(!g_pIoman) {
		return FFD_ERR_COULD_NOT_INITIALISE_FULLFAT;
	} else {
		Error = FF_RegisterBlkDevice(g_pIoman, GetBlockSize(g_hDisk), (FF_WRITE_BLOCKS) fnWrite, (FF_READ_BLOCKS) fnRead, g_hDisk);

		if(Error) {
			printError(Error);

			FF_DestroyIOMAN(g_pIoman);
			fnClose(g_hDisk);

			return -1;
		}

		// Attempt to mount!

		Error = FF_MountPartition(g_pIoman, (FF_T_UINT8) ulPartitionNumber);
		if(Error) {
			if(g_hDisk) {
				fnClose(g_hDisk);
			}
			FF_DestroyIOMAN(g_pIoman);
			g_pIoman = NULL;
			return -1;
		}

		g_Env.pIoman = g_pIoman;
		strcpy(g_Env.WorkingDir, "\\");	// Reset working dir!
	}

	return FFD_ERR_NONE;
}

/**
 * @public
 * @brief	Unmounts an Image from FullFAT, flushing changes to disk, and closing the imagefile.
 *
 **/
DLL_EXPORT int FFC_UnmountImage() {
	
	FF_ERROR Error;

	if(g_pIoman) {
		Error =  FF_UnmountPartition(g_pIoman);

		if(Error) {
			printError(Error);
			return -1;
		}

		g_pConsole = NULL;

		fnClose(g_hDisk);

		g_hDisk = NULL;

		return 0;
	}

	return -1;
}


