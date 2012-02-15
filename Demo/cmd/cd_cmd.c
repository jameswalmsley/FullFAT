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
#include "cd_cmd.h"

#ifdef FF_UNICODE_SUPPORT
#include <wchar.h>
#endif

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
	int			i;
#ifdef FF_UNICODE_SUPPORT
	FF_T_WCHAR	path[FF_MAX_PATH];
	FF_T_WCHAR	wcargv[FF_MAX_PATH];
#else
	FF_T_INT8	path[FF_MAX_PATH];
#endif

	if(argc == 2) {
#ifdef FF_UNICODE_SUPPORT
		FF_cstrtowcs(wcargv, argv[1]);		// Convert argv[1] into a UTF-16 string.
		ProcessPath(path, wcargv, pEnv);	// Make path absolute if relative.
		wcsExpandPath(path);	// Remove any relativity from the path (../ or ..\).
#else
		ProcessPath(path, argv[1], pEnv);	// Make path absolute if relative.
		ExpandPath(path);	// Remove any relativity from the path (../ or ..\).
#endif

#ifdef FF_UNICODE_SUPPORT
		i = wcslen(path);
#else
		i = strlen(path);
#endif

		if(i > 1) {
			if(path[i - 1] == '\\' || path[i - 1] == '/') {	// Get rid of the trailing slash, or FindFirst() will open that dir.
				path[i - 1] = '\0';
			}
		} else {
			if(path[0] == '\\' || path[0] == '/') {	// Root Dir!
#ifdef FF_UNICODE_SUPPORT
				wcscpy(pEnv->WorkingDir, path);
#else
				strcpy(pEnv->WorkingDir, path);
#endif
				return 0;
			}
		}
		
		if(!FF_FindFirst(pEnv->pIoman, &findData, path)) {
			if(findData.Attrib & FF_FAT_ATTR_DIR) {
				// Found a directory, change working dir!
#ifdef FF_UNICODE_SUPPORT
				wcscpy(pEnv->WorkingDir, path);
#else
				strcpy(pEnv->WorkingDir, path);
#endif
			} else {
#ifdef FF_UNICODE_SUPPORT
				wprintf(L"%s: %ls: Not a directory.\n", argv[0], path);
#else
				printf("%s: %s: Not a directory.\n", argv[0], path);
#endif
			}
		} else {
#ifdef FF_UNICODE_SUPPORT
			wprintf(L"%s: %ls: No such file or directory.\n", argv[0], path);
#else
			printf("%s: %s: No such file or directory.\n", argv[0], path);
#endif

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
