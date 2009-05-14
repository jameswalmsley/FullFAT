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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "FFTerm.h"

/**
 *	@file		FFTerm.c
 *	@author		James Walmsley
 *	@ingroup	FFTERM
 *
 *	@defgroup	FFTERM	FFTerm
 *	@brief		A platform independent console/terminal program.
 *
 *	Provides a simple and extendable terminals running over multiple, thread-safe
 *	independent consoles.
 *
 **/

/**
 *	@public
 *	@brief	Creates a console for the FFTerm Terminal.
 *
 *	@param	pa_strCmdPrompt		A string containing the command-prompt.
 *	@param	pa_pStdIn			FILE stream pointer for the INPUT for this console.
 *	@param	pa_pStdOut			FILE stream pointer for the OUTPUT for this console.
 *	@param	pError				Pointer to an Error code integer. Can be NULL when not checking errors.
 *	@param	pError				A string describing the Error code can be got by calling FFTerm_GetErrMessage()
 *
 *	@return	A valid console pointer on success, or NULL on any error. The value of pError can be checked for more
 *	@return	information about the specified error.
 *
 *	@see	FFTerm_GetErrMessage()
 *
 **/
FFT_CONSOLE *FFTerm_CreateConsole(FF_T_INT8 *pa_strCmdPrompt, FILE *pa_pStdIn, FILE * pa_pStdOut, FF_T_SINT32 *pError) {
	FFT_CONSOLE *pConsole = (FFT_CONSOLE *) malloc(sizeof(FFT_CONSOLE));
	
	if(pError) {
		*pError = FFT_ERR_NONE;	// Initialise the Error to no error.
	}

	if(strlen(pa_strCmdPrompt) > FFT_MAX_CMD_PROMPT) {
		if(pError) {
			*pError = FFT_ERR_CMD_PROMPT_TOO_LONG;
		}

		return (FFT_CONSOLE *) NULL;
	}

	if(pConsole) {
		pConsole->pCommands		= (FFT_COMMAND *) NULL;
		pConsole->bKill			= FF_FALSE;
		pConsole->pStdIn		= pa_pStdIn;
		pConsole->pStdOut		= pa_pStdOut;
		pConsole->Mode			= FFT_MODE_DEFAULT;
		strcpy(pConsole->strCmdPrompt, pa_strCmdPrompt);
		return pConsole;
	}

	if(pError) {
		*pError = FFT_ERR_NOT_ENOUGH_MEMORY;
	}

	return (FFT_CONSOLE *) NULL;
}


/**
 *	@public
 *	@brief	Adds a simple command to the provided FFTerm console.
 *
 *	Add's commands with functions of the form:
 *	int cmd_function(int argc, char **argv) {
 *		// Command code goes here.
 *		// Just like the main() function of a standard C program.
 *	}	
 *
 *	@param	pConsole		The FFT_CONSOLE object pointer.
 *	@param	pa_cmdName		A string of the command name.
 *	@param	pa_fnCmd		Function pointer to a command handler. (The name of the function for this command).
 *	@param	pa_ErrTable		A pointer to an Error Code Table. This can be NULL.
 *	@param	pa_ErrTable		See the documentation regarding FFT_ERR_TABLE definitions.
 *
 *	@return	FFT_ERR_NONE on Success, a negative error code on failure.
 *	@return	A string describing the Error code can be got by calling FFTerm_GetErrMessage().
 *
 **/
FF_T_SINT32 FFTerm_AddCmd(FFT_CONSOLE *pConsole, const FF_T_INT8 *pa_cmdName, FFT_FN_COMMAND pa_fnCmd, const FFT_ERR_TABLE *pa_ErrTable) {
	
	FFT_COMMAND *pCommand;

	if(!pConsole) {
		return FFT_ERR_NULL_POINTER;
	}

	if(strlen(pa_cmdName) > FFT_MAX_CMD_NAME) {
		return FFT_ERR_CMD_NAME_TOO_LONG;
	}

	if(!FFTerm_GetCmd(pConsole, pa_cmdName, NULL)) {
		
		if(pConsole->pCommands == NULL) {
			pConsole->pCommands = (FFT_COMMAND *) malloc(sizeof(FFT_COMMAND));
			pCommand = pConsole->pCommands;
		} else {
			pCommand = pConsole->pCommands;
			while(pCommand->pNextCmd != NULL) {	// Traverse to the end of the commands list.
				pCommand = pCommand->pNextCmd;
			}
			pCommand->pNextCmd = (FFT_COMMAND *) malloc(sizeof(FFT_COMMAND));
			pCommand = pCommand->pNextCmd;
		}
		
		if(pCommand) {
			pCommand->pNextCmd	= (FFT_COMMAND *) NULL;
			pCommand->fnCmd		= pa_fnCmd;
			pCommand->fnCmdEx	= NULL;
			pCommand->ErrTable  = pa_ErrTable;
			strcpy(pCommand->cmdName, pa_cmdName);

			FFTerm_SortCommands(pConsole);	// Keep commands sorted in Alphanumeric order.
		}

		return FFT_ERR_NONE;
	}

	return FFT_ERR_CMD_ALREADY_EXISTS;
}

/**
 *	@public
 *	@brief	Adds an extended command to the provided FFTerm console.
 *
 *	Add's commands with functions of the form:
 *	int cmd_function(int argc, char **argv, void *pParam) {
 *		// Command code goes here.
 *		// pParam is passed as an argument.
 *	}
 *
 *	@param	pConsole		The FFT_CONSOLE object pointer.
 *	@param	pa_cmdName		A string of the command name.
 *	@param	pa_fnCmd		Function pointer to a command handler. (The name of the function for this command).
 *	@param	pa_ErrTable		A pointer to an Error Code Table. This can be NULL.
 *	@param	pa_ErrTable		See the documentation regarding FFT_ERR_TABLE definitions.
 *	@param	pParam			A pointer to anything that the command might need.
 *
 *	@return	FFT_ERR_NONE on Success, a negative error code on failure.
 *	@return	A string describing the Error code can be got by calling FFTerm_GetErrMessage().
 *
 **/
FF_T_SINT32 FFTerm_AddExCmd(FFT_CONSOLE *pConsole, const FF_T_INT8 *pa_cmdName, FFT_FN_COMMAND_EX pa_fnCmd, const FFT_ERR_TABLE *pa_ErrTable, void *pParam) {
	
	FFT_COMMAND *pCommand;

	if(!pConsole) {
		return FFT_ERR_NULL_POINTER;
	}

	if(strlen(pa_cmdName) > FFT_MAX_CMD_NAME) {
		return FFT_ERR_CMD_NAME_TOO_LONG;
	}

	if(!FFTerm_GetCmd(pConsole, pa_cmdName, NULL)) {
		
		if(pConsole->pCommands == NULL) {
			pConsole->pCommands = (FFT_COMMAND *) malloc(sizeof(FFT_COMMAND));
			pCommand = pConsole->pCommands;
		} else {
			pCommand = pConsole->pCommands;
			while(pCommand->pNextCmd != NULL) {	// Traverse to the end of the commands list.
				pCommand = pCommand->pNextCmd;
			}
			pCommand->pNextCmd = (FFT_COMMAND *) malloc(sizeof(FFT_COMMAND));
			pCommand = pCommand->pNextCmd;
		}
		
		if(pCommand) {
			pCommand->pNextCmd	= (FFT_COMMAND *) NULL;
			pCommand->fnCmdEx	= pa_fnCmd;
			pCommand->fnCmd		= NULL;
			pCommand->ErrTable  = pa_ErrTable;
			pCommand->CmdExParam = pParam;
			strcpy(pCommand->cmdName, pa_cmdName);
			FFTerm_SortCommands(pConsole);	// Keep commands sorted in Alphanumeric order.
		}

		return FFT_ERR_NONE;
	}

	return FFT_ERR_CMD_ALREADY_EXISTS;
}

/**
 *	@public
 *	@brief	Removes a command from the specified FFTerm console.
 *
 *	@param	pConsole		The FFT_CONSOLE object pointer.
 *	@param	pa_cmdName		String of the command to remove.
 *
 **/
FF_T_SINT32 FFTerm_RemoveCmd(FFT_CONSOLE *pConsole, const FF_T_INT8 *pa_cmdName) {

	FFT_COMMAND *pCommand;
	FFT_COMMAND *pRmCmd;
	FF_T_SINT32	Error = FFT_ERR_NONE;
	
	if(!pConsole) {
		return FFT_ERR_NULL_POINTER;
	}

	pCommand = pConsole->pCommands;
	pRmCmd = FFTerm_GetCmd(pConsole, pa_cmdName, &Error);

	if(Error) {
		return Error;
	}

	if(pRmCmd) {

		if(pCommand == pRmCmd) {
			pConsole->pCommands = pRmCmd->pNextCmd;
			free(pRmCmd);
			return FFT_ERR_NONE;
		}

		while(pCommand != NULL) {
			if(pCommand->pNextCmd == pRmCmd) {
				pCommand->pNextCmd = pRmCmd->pNextCmd;
				free(pRmCmd);
				return FFT_ERR_NONE;
			}
			pCommand = pCommand->pNextCmd;
		}
	}

	return FFT_ERR_CMD_NOT_FOUND;
}

/**
 *	@public
 *	@brief	Sets the Mode of the specified console.
 *
 **/
FF_T_SINT32 FFTerm_SetConsoleMode(FFT_CONSOLE *pConsole, FF_T_UINT32 Mode) {
	if(!pConsole) {
		return FFT_ERR_NULL_POINTER;
	}

	pConsole->Mode = Mode;

	return FFT_ERR_NONE;
}

/**
 *	@public
 *	@brief	Gets the Mode of the specified console.
 *
 **/
FF_T_SINT32 FFTerm_GetConsoleMode(FFT_CONSOLE *pConsole, FF_T_UINT32 *Mode) {
	if(!pConsole) {
		return FFT_ERR_NULL_POINTER;
	}

	if(Mode) {
		*Mode = pConsole->Mode;
		return FFT_ERR_NONE;
	}

	return FFT_ERR_NULL_POINTER;
}


/**
 *	@public
 *	@brief	Starts the specified Console.
 *
 *	@param	pConsole		The FFT_CONSOLE object pointer.
 *
 *	@return	FFT_ERR_NONE on Success.
 **/
FF_T_SINT32 FFTerm_StartConsole(FFT_CONSOLE *pConsole) {

	FFT_COMMAND *pCommand;
	FF_T_SINT32	Error = FFT_ERR_NONE;
	FF_T_SINT32 Result = FFT_ERR_NONE;
	FF_T_SINT32	Argc;

	if(!pConsole) {
		return FFT_ERR_NULL_POINTER;
	}

	FFTerm_HookDefaultCommands(pConsole);	// Hook the default commands.

	while(pConsole->bKill != FF_TRUE) {
		
		Error = FFT_ERR_NONE;
		
		FFTerm_PrintPrompt(pConsole);
		
		// Get Command Line
		FFTerm_GetCommandLine(pConsole);

		// Process Command Line into Arguments
		Argc = FFTerm_GenerateArguments(pConsole->strCmdLine, (FF_T_INT8**)pConsole->pArgs);

		pCommand = FFTerm_GetCmd(pConsole, pConsole->pArgs[0], &Error);

		if(pCommand) {
			if(pCommand->fnCmd || pCommand->fnCmdEx) {
				if(pCommand->fnCmd) {
					Result = (FF_T_SINT32) pCommand->fnCmd((int)Argc, pConsole->pArgs);
					if(Result == FFT_KILL_CONSOLE) {
						pConsole->bKill = FF_TRUE;
						break;
					}
				} else {
					Result = (FF_T_SINT32) pCommand->fnCmdEx((int)Argc, pConsole->pArgs, pCommand->CmdExParam);
					if(Result == FFT_KILL_CONSOLE) {
						pConsole->bKill = FF_TRUE;
						break;
					}
				}

				if(Result) {
					if(pCommand->ErrTable) {
						fprintf(pConsole->pStdOut, "Command returned with message: \"%s\"\n", FFTerm_LookupErrMessage(pCommand->ErrTable, Result));
					} else {
						fprintf(pConsole->pStdOut, "Command returned with Code (%d)\n", Result);
					}
				}
			}
		} else {
			if(strlen(pConsole->strCmdLine) > 0) {
				fprintf(pConsole->pStdOut, "%s\n", FFTerm_GetErrMessage(Error));
			}
		}
		// Execute Relevent Commands
	}

	fprintf(pConsole->pStdOut, "Kill Signal Received - Closing Console\n");
	
	return FFT_ERR_NONE;
}

/**
 *	@public
 *	@brief	Kills the specified Console.
 *
 *	Allows another thread to Kill the specified console.
 *
 *	@param	pConsole		The FFT_CONSOLE object pointer.
 *
 *	@return	FFT_ERR_NONE on Success.
 **/
FF_T_SINT32 FFTerm_KillConsole(FFT_CONSOLE *pConsole) {

	if(!pConsole) {
		return FFT_ERR_NULL_POINTER;
	}

	pConsole->bKill = FF_TRUE;

	return FFT_ERR_NONE;
}

/**
 *	@private
 **/
static FFT_COMMAND *FFTerm_GetCmd(FFT_CONSOLE *pConsole, const FF_T_INT8 *pa_cmdName, FF_T_SINT32 *pError) {
	
	FFT_COMMAND *pCommand;
	FF_T_INT8	pa_cmdLower[FFT_MAX_CMD_NAME];
	FF_T_INT8	cmdLower[FFT_MAX_CMD_NAME];

	if(pError) {
		*pError = FFT_ERR_NONE;
	}
	
	if(!pConsole) {
		if(pError) {
			*pError = FFT_ERR_NULL_POINTER;
		}
		return NULL;
	}

	strcpy(pa_cmdLower, pa_cmdName);
	FFTerm_tolower(pa_cmdLower);		// Remove case from input command name.

	pCommand = pConsole->pCommands;

	if(pCommand) {
		while(pCommand != NULL) {
			strcpy(cmdLower, pCommand->cmdName);
			FFTerm_tolower(cmdLower);
			if(strcmp(cmdLower, pa_cmdLower) == 0) {
				return pCommand;
			}
			pCommand = pCommand->pNextCmd;
		}
	}

	if(pError) {
		*pError = FFT_ERR_CMD_NOT_FOUND;
	}

	return (FFT_COMMAND *) NULL;
}

/**
 *	@private
 **/
static FF_T_BOOL FFTerm_isSpace(FF_T_INT8 c) {
	if(c == ' ' || c == '\t') {
		return FF_TRUE;
	}

	return FF_FALSE;
}

/**
 *	@private
 **/
static FF_T_SINT32 FFTerm_GenerateArguments(FF_T_INT8 *strCmdLine, FF_T_INT8 **ppArgv) {
	FF_T_UINT32	iArgc;		// Argument Counter
	FF_T_UINT32	iCmdLineLen	= strlen(strCmdLine);
	FF_T_INT8	*pCmdLineEnd = (strCmdLine +iCmdLineLen);		// Pointer to the end of the CmdLine
	
	for(iArgc = 0; iArgc < FFT_MAX_CMD_LINE_ARGS;) {
		// Skip past any white space at the beginning.
		while((strCmdLine < pCmdLineEnd) && (FFTerm_isSpace(*strCmdLine))) {
			strCmdLine++;			
		}

		ppArgv[iArgc] = strCmdLine;
		
		// Allow arguments with spaces in them via inverted commas
		if(*strCmdLine == '\"') {
			strCmdLine++;
			ppArgv[iArgc] = strCmdLine;
			while((*strCmdLine) && (*strCmdLine != '\"')) {
				strCmdLine++;
			}
			// Skip past the " character
			*strCmdLine = '\0';
			strCmdLine++;
		}

		while((*strCmdLine) && (!FFTerm_isSpace(*strCmdLine))) {
			strCmdLine++;
		}

		*strCmdLine = '\0';
		strCmdLine++;

		// Check for end of command line.

		if(strCmdLine >= pCmdLineEnd) {
			break;
		} else {
			iArgc++;
		}
	}

	iArgc++;
	if(iArgc > FFT_MAX_CMD_LINE_ARGS) {
		iArgc = FFT_MAX_CMD_LINE_ARGS;
	}

	return iArgc;
}

/**
 *	@private
 **/
static FF_T_SINT32 FFTerm_GetCommandLine(FFT_CONSOLE *pConsole) {
	FF_T_UINT32	i = 0;
	FF_T_INT8	*pBuf = pConsole->strCmdLine;
	FF_T_INT32	c;
	FF_T_BOOL	bBreak = FF_FALSE;

	while(!bBreak) {
		if((c = fgetc(pConsole->pStdIn)) >= 0) {
			
			switch(c) {
				case FFT_RETURN:
				{
					if((pConsole->Mode & (FFT_ENABLE_ECHO_INPUT | FFT_ENABLE_WINDOWS))) {
						fputc('\n', pConsole->pStdOut);
					}

					pBuf[i] = '\0';
					bBreak = FF_TRUE;
					break;
				}

				case FFT_BACKSPACE:
				{
					fprintf(pConsole->pStdOut, "\b \b");
					if(i > 0) {
						i--;
						pBuf--;
						*pBuf = '\0';
					}
					break;
				}

				default:
				{
					if(i < (FFT_MAX_CMD_LINE_INPUT - 1)) {
						*(pBuf++) = (FF_T_INT8) c;
						*pBuf = '\0';
						if((pConsole->Mode & FFT_ENABLE_ECHO_INPUT)) {
							fputc(c, pConsole->pStdOut);
						}

						i++;
					}
					break;
				}
			}
		}
	}
	
	return FFT_ERR_NONE;

}


/**
 *	@private
 **/
static FF_T_SINT32 FFTerm_PrintPrompt(FFT_CONSOLE *pConsole) {
	FFT_COMMAND *pCommand;
	pCommand = FFTerm_GetCmd(pConsole, "prompt", NULL);
	if(pCommand) {
		if(pCommand->fnCmd || pCommand->fnCmdEx) {
			if(pCommand->fnCmd) {
				pCommand->fnCmd(0, NULL);
			}
			if(pCommand->fnCmdEx) {
				pCommand->fnCmdEx(0, NULL, pCommand->CmdExParam);
			}
		}
	} else {
		fprintf(pConsole->pStdOut, "%s", pConsole->strCmdPrompt);
	}
	return FFT_ERR_NONE;
}



/**
 *	@private
 **/
const FF_T_INT8 *FFTerm_LookupErrMessage(const FFT_ERR_TABLE *pa_pErrTable, FF_T_SINT32 iErrorCode) {
	const FFT_ERR_TABLE *pErrTable = pa_pErrTable;
	while (pErrTable->strErrorString){
        if (pErrTable->iErrorCode == iErrorCode) {
            return pErrTable->strErrorString;
        }
		pErrTable++;
    }
	return pa_pErrTable->strErrorString;
}


/**
 *	@private
 **/
FF_T_SINT32	FFTerm_SortCommands(FFT_CONSOLE *pConsole) {
	FFT_COMMAND *pCommand = pConsole->pCommands;
	FFT_COMMAND *pCmd1, *pCmd2;
	FF_T_UINT32	iSwaps;
	FF_T_SINT32	iResult;

	FF_T_INT8	CmdLower[FFT_MAX_CMD_NAME];
	FF_T_INT8	NextCmdLower[FFT_MAX_CMD_NAME];

	do {
		iSwaps = 0;
		pCommand = pConsole->pCommands;
		while(pCommand && pCommand->pNextCmd) {
			strcpy(CmdLower, pCommand->cmdName);
			strcpy(NextCmdLower, pCommand->pNextCmd->cmdName);
			FFTerm_tolower(CmdLower);
			FFTerm_tolower(NextCmdLower);
			iResult = strcmp(CmdLower, NextCmdLower);
			if(iResult == 1) {	// Put pCommand after the next command.
				iSwaps++;
				pCmd1 = pConsole->pCommands;
				if(pCmd1 == pCommand) {
					pCmd2 = pCommand->pNextCmd;
					pConsole->pCommands = pCmd2;
					pCommand->pNextCmd = pCmd2->pNextCmd;
					pCmd2->pNextCmd = pCommand;
				} else {
					while(pCmd1 && pCmd1->pNextCmd) {
						if(pCmd1->pNextCmd == pCommand) {
							pCmd2 = pCommand->pNextCmd;
							pCmd1->pNextCmd = pCmd2;
							pCommand->pNextCmd = pCmd2->pNextCmd;
							pCmd2->pNextCmd = pCommand;
							break;
						}
						pCmd1 = pCmd1->pNextCmd;
					}
				}

			}
			if(iResult == -1) {	// Leave current string where it is.
				

			}
			pCommand = pCommand->pNextCmd;
		}
	} while(iSwaps);
	
	return FFT_ERR_NONE;
}

static void FFTerm_tolower(FF_T_INT8 *string) {
	while(*string) {
		if(*string >= 'A' && *string <= 'Z') {
			*string += 32;
		}
		string++;
	}
}
