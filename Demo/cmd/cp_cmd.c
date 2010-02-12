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
#include "cp_cmd.h"

/*
	This is a new implementation of a file copying command for FullFAT.
	
	It behaves similar to the GNU cp command.
*/

int cp_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	FF_DIRENT 			findData;
	FF_GETOPT_CONTEXT	optionContext;
	FF_T_BOOL			bRecursive = FF_FALSE, bVerbose = FF_FALSE;

	const char				*szpSource, *szpDestination;

	int option;

	memset(&optionContext, 0, sizeof(FF_GETOPT_CONTEXT));
	option = FF_getopt(argc, argv, "rRv", &optionContext);

	while(option != EOF) {						// Process Commandline options
		switch(option) {
			case 'r':
			case 'R':
				bRecursive = FF_TRUE;
				break;

			case 'v':
				bVerbose = FF_TRUE;
				break;

			default:
				break;
		}

		option = FF_getopt(argc, argv, "rRv", &optionContext);
	}

	if(optionContext.optind + 0 < argc) {
		szpSource = argv[optionContext.optind + 0];
	}

	if(optionContext.optind + 1 < argc) {
		szpDestination = argv[optionContext.optind + 1];
	}
	
	if(!szpSource) {
		printf("%s: No source file argument\n", argv[0]);
		return 0;
	}

	if(!szpDestination) {
		printf("%s: No destination file argument\n", argv[0]);
		return 0;
	}
	
	return 0;
}
const FFT_ERR_TABLE cpInfo[] =
{
	{"Generic or Unknown Error",										-1},
	{"Copies the specified file to the specified location.",			FFT_COMMAND_DESCRIPTION},
	{ NULL }
};
