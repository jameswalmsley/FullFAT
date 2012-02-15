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
#include "mkdir_cmd.h"

/**
 *	@brief	A simple command for making dirs.
 **/
int mkdir_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	
	FF_ERROR	Error;

#ifdef FF_UNICODE_SUPPORT
	FF_T_WCHAR	path[FF_MAX_PATH];
	FF_T_WCHAR	wcargv[FF_MAX_PATH];
#else
	FF_T_INT8	path[FF_MAX_PATH];
#endif

	if(argc == 2) {
#ifdef FF_UNICODE_SUPPORT
		FF_cstrtowcs(wcargv, argv[1]);
		ProcessPath(path, wcargv, pEnv);
#else
		ProcessPath(path, argv[1], pEnv);
#endif
		Error = FF_MkDir(pEnv->pIoman, path);
		if(FF_isERR(Error)) {
			printf("Could not mkdir - %s\n", FF_GetErrMessage(Error));
		}
	} else {
		printf("Usage: %s [path]\n", argv[0]);
	}
	return 0;
}
const FFT_ERR_TABLE mkdirInfo[] =
{
	{"Generic or Unknown Error",							-1},
	{"Creates directories.",	FFT_COMMAND_DESCRIPTION},
	{ NULL }
};
