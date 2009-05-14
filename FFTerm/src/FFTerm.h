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

#ifndef _FFTERM_H_
#define _FFTERM_H_

#include "FFTerm-Types.h"
#include "FFTerm-Error.h"

#include <stdio.h>

#define FFT_VERSION_NUMBER		"0.5.0"

#define	FFT_ENABLE_ECHO_INPUT	0x0004
#define FFT_ENABLE_LINE_INPUT	0x0002
#define FFT_ENABLE_WINDOWS		0x8000

#define FFT_MODE_DEFAULT		0		//FFT_ENABLE_ECHO_INPUT
#define WINDOWS

#define FFT_MAX_CMD_NAME		10
#define FFT_MAX_CMD_PROMPT		20

#define FFT_MAX_CMD_LINE_INPUT	2048	// 2Kb Per Command Line
//#define FFT_MAX_CMD_ARG_LENGTH	128
#define FFT_MAX_CMD_LINE_ARGS	256		// 1Kb of Argument pointers

#define FFT_RETURN				0x0A
#define FFT_BACKSPACE			0x08
#define FFT_CRLF				"\n"

#define FFT_KILL_CONSOLE		-666	///< Special return value from any Command to kill the console.
#define FFT_COMMAND_DESCRIPTION	-1001	///< Special Error Code for getting a Command Description.

typedef int (*FFT_FN_COMMAND)	 (int argc, char **argv);				// Deliberately left as platform types.
typedef int (*FFT_FN_COMMAND_EX) (int argc, char **argv, void *pParam);	// Extended Command function.

typedef struct {
	const FF_T_INT8 * const strErrorString;
    const FF_T_SINT32 iErrorCode;
} FFT_ERR_TABLE;

typedef struct _FFT_COMMAND {
	FF_T_INT8			cmdName[FFT_MAX_CMD_NAME];
	FFT_FN_COMMAND		fnCmd;
	FFT_FN_COMMAND_EX	fnCmdEx;
	void				*CmdExParam;
	const FFT_ERR_TABLE	*ErrTable;
	struct _FFT_COMMAND	*pNextCmd;		///< Pointer to the next command in the linked list.
} FFT_COMMAND;

typedef struct {
				FF_T_INT8	strCmdPrompt[FFT_MAX_CMD_PROMPT];
				FF_T_INT8	strCmdLine[FFT_MAX_CMD_LINE_INPUT];
				FF_T_INT8	*pArgs[FFT_MAX_CMD_LINE_ARGS];
				FF_T_UINT32	Mode;	
	volatile	FF_T_BOOL	bKill;		///< Volatile because it can be set in a multi-threaded environment. (Or the exit command!);
				FFT_COMMAND	*pCommands;
				FILE		*pStdIn;
				FILE		*pStdOut;
} FFT_CONSOLE;


extern FF_T_SINT32	 FFTerm_HookDefaultCommands	(FFT_CONSOLE *pConsole);
FFT_CONSOLE			*FFTerm_CreateConsole		(FF_T_INT8 *pa_strCmdPrompt, FILE *pa_pStdIn, FILE * pa_pStdOut, FF_T_SINT32 *pError);
FF_T_SINT32			 FFTerm_StartConsole		(FFT_CONSOLE *pConsole);
FFT_COMMAND			*FFTerm_GetCmd				(FFT_CONSOLE *pConsole, const FF_T_INT8 *pa_cmdName, FF_T_SINT32 *pError);
FF_T_SINT32			 FFTerm_AddCmd				(FFT_CONSOLE *pConsole, const FF_T_INT8 *pa_cmdName, FFT_FN_COMMAND pa_fnCmd, const FFT_ERR_TABLE *pa_ErrTable);
FF_T_SINT32			 FFTerm_AddExCmd			(FFT_CONSOLE *pConsole, const FF_T_INT8 *pa_cmdName, FFT_FN_COMMAND_EX pa_fnCmd, const FFT_ERR_TABLE *pa_ErrTable, void *pParam);
FF_T_SINT32			 FFTerm_RemoveCmd			(FFT_CONSOLE *pConsole, const FF_T_INT8 *pa_cmdName);
FF_T_SINT32			 FFTerm_SetConsoleMode		(FFT_CONSOLE *pConsole, FF_T_UINT32 Mode);
FF_T_SINT32			 FFTerm_GetConsoleMode		(FFT_CONSOLE *pConsole, FF_T_UINT32 *Mode);
static FF_T_SINT32	 FFTerm_PrintPrompt			(FFT_CONSOLE *pConsole);
const FF_T_INT8		*FFTerm_LookupErrMessage	(const FFT_ERR_TABLE *pa_pErrTable, FF_T_SINT32 iErrorCode);
static FF_T_SINT32	 FFTerm_SortCommands		(FFT_CONSOLE *pConsole);
static FF_T_SINT32	 FFTerm_GetCommandLine(FFT_CONSOLE *pConsole);
static FF_T_SINT32	 FFTerm_GenerateArguments	(FF_T_INT8 *strCmdLine, FF_T_INT8 **ppArgv);
static void			 FFTerm_tolower				(FF_T_INT8 *string);

#endif
