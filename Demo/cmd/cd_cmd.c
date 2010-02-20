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
#include "cd_cmd.h"

/**
 *	@public
 *	@brief	Changes the Current Working Directory
 *
 *	To change the directory, we simply check the provided path, if it exists,
 *	then we change the environment's working directory to that path.
 *
 **/
int cd_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	FF_DIRENT	findData;
	FF_T_INT8	path[FF_MAX_PATH];
	int			i;

	if(argc == 2) {
		ProcessPath(path, argv[1], pEnv);	// Make path absolute if relative.
		ExpandPath(path);	// Remove any relativity from the path (../ or ..\).

		i = strlen(path);

		if(i > 1) {
			if(path[i - 1] == '\\' || path[i - 1] == '/') {	// Get rid of the trailing slash, or FindFirst() will open that dir.
				path[i - 1] = '\0';
			}
		} else {
			if(path[0] == '\\' || path[0] == '/') {	// Root Dir!
				strcpy(pEnv->WorkingDir, path);
				return 0;
			}
		}
		
		if(!FF_FindFirst(pEnv->pIoman, &findData, path)) {
			if(findData.Attrib & FF_FAT_ATTR_DIR) {
				// Found a directory, change working dir!
				strcpy(pEnv->WorkingDir, path);
			} else {
				printf("%s: %s: Not a directory.\n", argv[0], path);
			}
		} else {
			printf("%s: %s: No such file or directory.\n", argv[0], path);
		}
	} else {
		printf("Usage: %s [path]\n", argv[0]);
	}
	return 0;
}
const FFT_ERR_TABLE cdInfo[] =
{
	{"Unknown or Generic Error",		-1},							// Generic Error (always the first entry).
	{"Changes the current working directory to the specified path.",			FFT_COMMAND_DESCRIPTION},
	{ NULL }
};
