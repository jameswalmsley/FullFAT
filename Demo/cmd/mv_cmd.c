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

#include "mv_cmd.h"

int mv_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	const char			*szpSource, *szpDestination;//, *szpWildCard;
	char				szsrcPath[FF_MAX_PATH], szdestPath[FF_MAX_PATH];
	//FF_DIRENT			findData;
	
	FFT_GETOPT_CONTEXT	optionContext;	// CommandLine processing
//	FF_T_BOOL			bRecursive = FF_FALSE, bVerbose = FF_FALSE;	// Option Flags.
	int 				option;

	memset(&optionContext, 0, sizeof(FFT_GETOPT_CONTEXT));			// Initialise the option context to zero.

	option = FFTerm_getopt(argc, (const char **) argv, "rRv", &optionContext);		// Get the command line option charachters.

	while(option != EOF) {											// Process Commandline options
		switch(option) {
			case 'r':
			case 'R':
//				bRecursive = FF_TRUE;								// Set recursive flag if -r or -R appears on the commandline.
				break;

			case 'v':
//				bVerbose = FF_TRUE;									// Set verbose flag if -v appears on the commandline.
				break;

			default:
				break;
		}

		option = FFTerm_getopt(argc, (const char **) argv, "rRv", &optionContext);	// Get the next option.
	}

	szpSource 		= FFTerm_getarg(argc, (const char **) argv, 0, &optionContext);	// The remaining options or non optional arguments.
	szpDestination 	= FFTerm_getarg(argc, (const char **) argv, 1, &optionContext);	// getarg() retrieves them intelligently.
	
	if(!szpSource) {
		printf("%s: No source file argument.\n", argv[0]);			// No source file provided.
		return 0;
	}

	if(!szpDestination) {
		printf("%s: No destination file argument.\n", argv[0]);		// No destination provided.
		return 0;
	}

	ProcessPath(szsrcPath, szpSource, pEnv);						// Process the paths into absolute paths.
	ProcessPath(szdestPath, szpDestination, pEnv);

	//szpWildCard = GetWildcard(szpSource);							// Get the last token of the source path. (This may include a wildCard).

	FF_Move(pEnv->pIoman, szsrcPath, szdestPath);					// Now move the file/directory.
	
	return 0;
}
const FFT_ERR_TABLE mvInfo[] =
{
	{"Generic or Unknown Error",										-1},
	{"Moves the specified file/dir to the specified location.",			FFT_COMMAND_DESCRIPTION},
	{ NULL }
};
