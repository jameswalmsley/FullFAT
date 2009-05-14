/*****************************************************************************
 *  FFTerm - Simple & Platform independent, Thread-Safe Terminal/Console     *
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
 *  See RESTRICTIONS.TXT for extra restrictions on the use of FFTerm.        *
 *                                                                           *
 *  Removing the above notice is illegal and will invalidate this license.   *
 *****************************************************************************
 *  See http://worm.me.uk/ffterm for more information.                       *
 *  Or  http://ffterm.googlecode.com/ for latest releases and the wiki.      *
 *****************************************************************************/

#include "FFTerm-Commands.h"
#include "FFTerm.h"

static int FFTerm_CmdExit(int argc, char **argv) {
	if(argc) {
		return FFT_KILL_CONSOLE;
	}
	return 0;
}

const FFT_ERR_TABLE FFTerm_CmdExitInfo[] =
{
	"Causes the active console to terminate.",			FFT_COMMAND_DESCRIPTION,
	NULL
};

static int FFTerm_CmdHelp(int argc, char **argv, FFT_CONSOLE *pConsole) {
	FFT_COMMAND *pCommand = pConsole->pCommands;
	/**
	 *	Removing the following notice without a commercial license is illegal.
	 **/
	printf("\nFFTerm v%s by James Walmsley (c)2009\n", FFT_VERSION_NUMBER);
	printf("For more information about FFTerm, see http://worm.me.uk/ffterm/\n");
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
const FFT_ERR_TABLE FFTerm_CmdHelpInfo[] =
{
	"This help information screen.",			FFT_COMMAND_DESCRIPTION,
	NULL
};


FF_T_SINT32 FFTerm_HookDefaultCommands(FFT_CONSOLE *pConsole) {
	FFTerm_AddCmd(pConsole, "exit", FFTerm_CmdExit, FFTerm_CmdExitInfo);
	FFTerm_AddExCmd(pConsole, "help", FFTerm_CmdHelp, FFTerm_CmdHelpInfo, pConsole);
	return FFT_ERR_NONE;
}

