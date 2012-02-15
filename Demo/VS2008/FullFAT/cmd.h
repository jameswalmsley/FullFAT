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

#ifndef _CMD_H_
#define _CMD_H_

#include "../../../src/fullfat.h"
#include "../../../../FFTerm/src/FFTerm.h"

typedef struct {							// Provides an environment for the FullFAT commands.
	FF_IOMAN	*pIoman;
	FF_T_INT8	WorkingDir[FF_MAX_PATH];	// A working directory Environment variable.
} FF_ENVIRONMENT;

void ProcessPath(char *dest, const char *src, FF_ENVIRONMENT *pEnv);

extern const FFT_ERR_TABLE promptInfo[];
extern const FFT_ERR_TABLE pwdInfo[];
extern const FFT_ERR_TABLE lsInfo[];
extern const FFT_ERR_TABLE cdInfo[];
extern const FFT_ERR_TABLE md5Info[];
extern const FFT_ERR_TABLE cpInfo[];
extern const FFT_ERR_TABLE icpInfo[];
extern const FFT_ERR_TABLE xcpInfo[];
extern const FFT_ERR_TABLE mkdirInfo[];
extern const FFT_ERR_TABLE infoInfo[];
extern const FFT_ERR_TABLE mountInfo[];
extern const FFT_ERR_TABLE viewInfo[];
extern const FFT_ERR_TABLE rmInfo[];
extern const FFT_ERR_TABLE mkimgInfo[];
extern const FFT_ERR_TABLE mkfileInfo[];
extern const FFT_ERR_TABLE mkwinfileInfo[];
extern const FFT_ERR_TABLE exitInfo[];
extern const FFT_ERR_TABLE md5winInfo[];
extern const FFT_ERR_TABLE hexviewInfo[];
extern const FFT_ERR_TABLE runInfo[];
extern const FFT_ERR_TABLE timeInfo[];
extern const FFT_ERR_TABLE dateInfo[];
extern const FFT_ERR_TABLE moveInfo[];
extern const FFT_ERR_TABLE drivelistInfo[];


int cmd_prompt		(int argc, char **argv, FF_ENVIRONMENT *pEnv);
int pwd_cmd			(int argc, char **argv, FF_ENVIRONMENT *pEnv);
int ls_cmd			(int argc, char **argv, FF_ENVIRONMENT *pEnv);
int cd_cmd			(int argc, char **argv, FF_ENVIRONMENT *pEnv);
int md5_cmd			(int argc, char **argv, FF_ENVIRONMENT *pEnv);
int cp_cmd			(int argc, char **argv, FF_ENVIRONMENT *pEnv);
int icp_cmd			(int argc, char **argv, FF_ENVIRONMENT *pEnv);
int xcp_cmd			(int argc, char **argv, FF_ENVIRONMENT *pEnv);
int mkdir_cmd		(int argc, char **argv, FF_ENVIRONMENT *pEnv);
int info_cmd		(int argc, char **argv, FF_ENVIRONMENT *pEnv);
int mount_cmd		(int argc, char **argv, FF_ENVIRONMENT *pEnv);
int view_cmd		(int argc, char **argv, FF_ENVIRONMENT *pEnv);
int rm_cmd			(int argc, char **argv, FF_ENVIRONMENT *pEnv);
int move_cmd		(int argc, char **argv, FF_ENVIRONMENT *pEnv);
int mkimg_cmd		(int argc, char **argv, FF_ENVIRONMENT *pEnv);
int mkfile_cmd		(int argc, char **argv, FF_ENVIRONMENT *pEnv);
int mkwinfile_cmd	(int argc, char **argv);
int exit_cmd		(int argc, char **argv);
int run_cmd			(int argc, char **argv);
int time_cmd		(int argc, char **argv);
int date_cmd		(int argc, char **argv);
int md5win_cmd		(int argc, char **argv);
int hexview_cmd		(int argc, char **argv);
int drivelist_cmd	(int argc, char **argv);

int icpdir_copy(const char *szSource, const char *szDestination, FF_T_BOOL bRecursive, FF_ENVIRONMENT *pEnv);
int icp_copy(char *szSource, char *szDestination, FF_ENVIRONMENT *pEnv);

// Ordered Dir Listings
int ls_dir(const char *szPath, FF_T_BOOL bList, FF_T_BOOL bRecursive, FF_T_BOOL bShowHidden, FF_ENVIRONMENT *pEnv);

extern void KillAllThreads(void);

#endif
