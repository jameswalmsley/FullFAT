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

#include "FFTerm-Commands.h"
#include "FFTerm.h"

const FFT_ERR_TABLE CmdExitErrorTable[] =
{
	"Causes the active console to terminate.",			FFT_COMMAND_DESCRIPTION,
	NULL
};

FF_T_SINT32 FFTerm_CmdExit(FF_T_SINT32 argc, FF_T_INT8 **argv) {
	if(argc) {
		return FFT_KILL_CONSOLE;
	}
	return 0;
}

FF_T_SINT32 FFTerm_CmdHelp(FF_T_SINT32 argc, FF_T_INT8 **argv, FFT_CONSOLE *pConsole) {
	FFT_COMMAND *pCommand = pConsole->pCommands;
	printf("\nAvailable Commands:\n\n");
	
	while(pCommand) {
		printf("%s", pCommand->cmdName);
		if(pCommand->ErrTable) {
			printf("\t - %s", FFTerm_LookupErrMessage(pCommand->ErrTable, FFT_COMMAND_DESCRIPTION));
		}
		printf("\n");
		pCommand = pCommand->pNextCmd;
	}

	printf("\nFor more information about each command, type [command name] help\n\n");
	
	return FFT_ERR_NONE;
}

FF_T_SINT32 FFTerm_HookDefaultCommands(FFT_CONSOLE *pConsole) {
	FFTerm_AddCmd(pConsole, "exit", (FFT_FN_COMMAND)FFTerm_CmdExit, CmdExitErrorTable);
	FFTerm_AddExCmd(pConsole, "help", (FFT_FN_COMMAND_EX)FFTerm_CmdHelp, NULL, pConsole);
	return FFT_ERR_NONE;
}

