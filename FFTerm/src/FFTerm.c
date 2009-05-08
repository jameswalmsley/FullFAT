#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "FFTerm.h"

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
		}

		return FFT_ERR_NONE;
	}

	return FFT_ERR_CMD_ALREADY_EXISTS;
}


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
		}

		return FFT_ERR_NONE;
	}

	return FFT_ERR_CMD_ALREADY_EXISTS;
}


FFT_CONSOLE *FFTerm_CreateConsole(FF_T_INT8 *pa_strCmdPrompt, FILE *pa_pStdIn, FILE * pa_pStdOut, FF_T_SINT32 *pError) {
	FFT_CONSOLE *pConsole = (FFT_CONSOLE *) malloc(sizeof(FFT_CONSOLE));
	
	if(pError) {
		*pError = FFT_ERR_NONE;
	}

	if(strlen(pa_strCmdPrompt) > FFT_MAX_CMD_PROMPT) {
		if(pError) {
			*pError = FFT_ERR_CMD_PROMPT_TOO_LONG;
		}

		return (FFT_CONSOLE *) NULL;
	}

	if(pConsole) {
		pConsole->pCommands = (FFT_COMMAND *) NULL;
		pConsole->bKill = FF_FALSE;
		pConsole->pStdIn = pa_pStdIn;
		pConsole->pStdOut = pa_pStdOut;
		pConsole->Mode = FFT_MODE_DEFAULT;
		strcpy(pConsole->strCmdPrompt, pa_strCmdPrompt);
		return pConsole;
	}

	if(pError) {
		*pError = FFT_ERR_NOT_ENOUGH_MEMORY;
	}

	return (FFT_CONSOLE *) NULL;
}


FFT_COMMAND *FFTerm_GetCmd(FFT_CONSOLE *pConsole, const FF_T_INT8 *pa_cmdName, FF_T_SINT32 *pError) {
	
	FFT_COMMAND *pCommand;

	if(pError) {
		*pError = FFT_ERR_NONE;
	}
	
	if(!pConsole) {
		if(pError) {
			*pError = FFT_ERR_NULL_POINTER;
		}
		return NULL;
	}

	pCommand = pConsole->pCommands;

	if(pCommand) {
		while(pCommand != NULL) {
			if(strcmp(pCommand->cmdName, pa_cmdName) == 0) {
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

FF_T_BOOL FFTerm_isSpace(FF_T_INT8 c) {
	if(c == ' ') {
		return FF_TRUE;
	}

	return FF_FALSE;
}

FF_T_SINT32	FFTerm_GenerateArguments(FF_T_INT8 *strCmdLine, FF_T_INT8 **ppArgv) {
	FF_T_UINT32	iArgc;		// Argument Counter
	FF_T_UINT32	iArgLen;	// Argument Length Counter
	FF_T_UINT32	iCmdLineLen	= strlen(strCmdLine);
	FF_T_INT8	*pCmdLineEnd = (strCmdLine +iCmdLineLen);		// Pointer to the end of the CmdLine

	
	for(iArgc = 0; iArgc < FFT_MAX_CMD_LINE_ARGS;) {
		// pArg = ppArgv[iArgc];
		// Skip past any white space at the beginning.
		while((strCmdLine < pCmdLineEnd) && (FFTerm_isSpace(*strCmdLine))) {
			strCmdLine++;			
		}

		// Initialise the length of the argument.
		iArgLen = 0;
		ppArgv[iArgc] = strCmdLine;
		
		// Allow arguments with spaces in them via inverted commas
		if(*strCmdLine == '\"') {
			strCmdLine++;
			ppArgv[iArgc] = strCmdLine;
			while((*strCmdLine) && (*strCmdLine != '\"')) {
				iArgLen++;
				if(iArgLen >= FFT_MAX_CMD_ARG_LENGTH) {
				//	*pArg = '\0';
					break;
				}
				//*pArg++ = *strCmdLine++;
				strCmdLine++;
			}
			// Skip past the " character
			*strCmdLine = '\0';
			strCmdLine++;
		}

		while((*strCmdLine) && (!FFTerm_isSpace(*strCmdLine))) {
			iArgLen++;
			if(iArgLen >= FFT_MAX_CMD_ARG_LENGTH) {
			//	*pArg = '\0';
				break;
			}
			strCmdLine++;
			//*pArg++ = *strCmdLine++;
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

FF_T_SINT32 FFTerm_GetCommandLine(FFT_CONSOLE *pConsole) {
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

FF_T_SINT32 FFTerm_SetConsoleMode(FFT_CONSOLE *pConsole, FF_T_UINT32 Mode) {
	if(!pConsole) {
		return FFT_ERR_NULL_POINTER;
	}

	pConsole->Mode = Mode;

	return FFT_ERR_NONE;
}

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

FF_T_SINT32 FFTerm_PrintPrompt(FFT_CONSOLE *pConsole) {
	fprintf(pConsole->pStdOut, "%s", pConsole->strCmdPrompt);
	return FFT_ERR_NONE;
}

// Starts the console.
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
					Result = pCommand->fnCmd(Argc, pConsole->pArgs);
					if(Result == FFT_KILL_CONSOLE) {
						pConsole->bKill = FF_TRUE;
						break;
					}
				} else {
					Result = pCommand->fnCmdEx(Argc, pConsole->pArgs, pCommand->CmdExParam);
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

// Allows another thread to kill the chosen console.
FF_T_SINT32 FFTerm_KillConsole(FFT_CONSOLE *pConsole) {

	if(!pConsole) {
		return FFT_ERR_NULL_POINTER;
	}

	return FFT_ERR_NONE;
}

