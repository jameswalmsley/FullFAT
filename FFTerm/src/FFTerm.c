#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "FFTerm.h"


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


FF_T_SINT32 FFTerm_AddCmd(FFT_CONSOLE *pConsole, const FF_T_INT8 *pa_cmdName, FFT_FN_COMMAND pa_fnCmd) {
	
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
			strcpy(pCommand->cmdName, pa_cmdName);
		}

		return FFT_ERR_NONE;
	}

	return FFT_ERR_CMD_ALREADY_EXISTS;
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

	if(!pConsole) {
		return FFT_ERR_NULL_POINTER;
	}

	while(pConsole->bKill != FF_TRUE) {
		
		FFTerm_PrintPrompt(pConsole);

		// Get Command Line
		FFTerm_GetCommandLine(pConsole);

		// Process Command Line into Arguments
		printf("\n CMDLINE: %s\n", pConsole->strCmdLine);

		// Execute Relevent Commands
	}
	
	return FFT_ERR_NONE;
}


// Allows another thread to kill the chosen console.
FF_T_SINT32 FFTerm_KillConsole(FFT_CONSOLE *pConsole) {

	if(!pConsole) {
		return FFT_ERR_NULL_POINTER;
	}

	return FFT_ERR_NONE;
}

