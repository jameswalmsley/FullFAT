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
//#include "cmd.h"	// The Demo's Header File
#include "../../src/fullfat.h"
#include "../../../FFTerm/src/FFTerm.h"
//#include "testdriver_win32.h"

#define PARTITION_NUMBER	0

typedef struct {	// Pass an environment for the FullFAT commands.
	FF_IOMAN	*pIoman;
	FF_T_INT8	WorkingDir[2600];
} FF_ENVIRONMENT;

int main(void) {

	FFT_CONSOLE *pConsole;				// FFTerm Console Pointer.
	FILE *f;							// FILE Stream pointer for Windows FullFAT driver.
	FF_T_SINT32	Error = FF_ERR_NONE;
	FF_IOMAN *pIoman;
	FF_ENVIRONMENT Env;

	//----------- Initialise the environment
	Env.pIoman = NULL;
	strcpy(Env.WorkingDir, "\\");

	// Open a File Stream for FullFAT's I/O driver to work on.
	f = fopen("\\\\.\\PHYSICALDRIVE1", "rb+");

	//f = fopen("c:\\Write.img", "rb+");

	if(!f) {
		//---------- Create FullFAT IO Manager
		pIoman = FF_CreateIOMAN(NULL, 8192, 512, (FF_T_SINT8 *)&Error);

		if(pIoman) {
			//---------- Register a Block Device with FullFAT
			//FF_RegisterBlkDevice(pIoman, 512, (FF_WRITE_BLOCKS) fnWrite_512, (FF_READ_BLOCKS) fnRead_512, f);

			//---------- Try to Mount the Partition with FullFAT
			/*if(FF_MountPartition(pIoman, PARTITION_NUMBER)) {
				if(f) {
					fclose(f);
				}
				FF_DestroyIOMAN(pIoman);
				printf("FullFAT Couldn't mount the specified parition!\n");
				getchar();
				return -1;
			}*/

			Env.pIoman = pIoman;

			//---------- Create the Console
			pConsole = FFTerm_CreateConsole("FullFAT>", stdin, stdout, &Error);

			if(pConsole) {
				FFTerm_SetConsoleMode(pConsole, 0);
				//---------- Add Commands to the console.
/*				FFTerm_AddExCmd(pConsole, "prompt", cmd_prompt, promptInfo, &Env);	// special command named prompt used as a command prompt if hooked.
				FFTerm_AddExCmd(pConsole, "pwd",	pwd_cmd,	pwdInfo,	&Env);	// See cmd.c for their implementations.
				FFTerm_AddExCmd(pConsole, "ls",		ls_cmd,		lsInfo,		&Env);
				FFTerm_AddExCmd(pConsole, "cd",		cd_cmd,		cdInfo,		&Env);
				FFTerm_AddExCmd(pConsole, "cp",		cp_cmd,		cpInfo,		&Env);
				FFTerm_AddExCmd(pConsole, "icp",	icp_cmd,	icpInfo,	&Env);
				FFTerm_AddExCmd(pConsole, "xcp",	xcp_cmd,	xcpInfo,	&Env);
				FFTerm_AddExCmd(pConsole, "md5",	md5_cmd,	md5Info,	&Env);
				FFTerm_AddExCmd(pConsole, "mkdir",	mkdir_cmd,	mkdirInfo,	&Env);
				FFTerm_AddExCmd(pConsole, "info",	info_cmd,	infoInfo,	&Env);
*/
				//---------- Start the console.
				FFTerm_StartConsole(pConsole);
				FF_DestroyIOMAN(pIoman);
				printf("\n\nConsole Was Terminated, END OF Demonstration!, Press ENTER to exit!\n");
				getchar();
				return 0;
			}

			printf("Could not start the console: %s\n", FFTerm_GetErrMessage(Error));
			getchar();
			return -1;
		}
	}

	printf("Could not open the I/O Block device\n");
	getchar();
	return -1;

}
