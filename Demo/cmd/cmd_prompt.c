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
#include "cmd_prompt.h"

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
#ifdef FF_UNICODE_SUPPORT
		printf("%ls>", pEnv->WorkingDir);
#else
		printf("%s>", pEnv->WorkingDir);
#endif
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
const FFT_ERR_TABLE cmdpromptInfo[] =
{
	{"Unknown or Generic Error",						-1},						// Generic Error (always the first entry).
	{"This message is displayed when -2 is returned",	-2},						// Error Message for a -2 return code.
	{"The command prompt!",								FFT_COMMAND_DESCRIPTION},	// Command Description.
	{ NULL }																		// Always terminated with NULL.
};
