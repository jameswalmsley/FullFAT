#include <stdlib.h>
#include <string.h>
#include "FFTerm.h"


FFT_CONSOLE *FFTerm_CreateConsole(FF_T_SINT8 *pError) {
	FFT_CONSOLE *pConsole = (FFT_CONSOLE *) malloc(sizeof(FFT_CONSOLE));
	
	if(pError) {
		*pError = FFT_ERR_NONE;
	}

	if(pConsole) {
		pConsole->pCommands = (FFT_COMMAND *) NULL;
	}

	if(pError) {
		*pError = FFT_ERR_NOT_ENOUGH_MEMORY;
	}

	return (FFT_CONSOLE *) NULL;
}

FF_T_SINT8 FFTerm_AddCmd(FFT_CONSOLE *pConsole, FF_T_INT8 *pa_cmdName, FFT_FN_COMMAND pa_fnCmd) {
	
	FFT_COMMAND *pCommand;

	if(!pConsole) {
		return FFT_ERR_NULL_POINTER;
	}
	
	pCommand = pConsole->pCommands;

	if(strlen(pa_cmdName) > FFT_MAX_CMD_NAME) {
		return FFT_ERR_CMD_NAME_TOO_LONG;
	}

	while(pCommand->pNextCmd != NULL) {	// Traverse to the end of the commands list.
		pCommand = pCommand->pNextCmd;
	}

	pCommand->pNextCmd = (FFT_COMMAND *) malloc(sizeof(FFT_COMMAND));
	
	if(pCommand->pNextCmd) {
		pCommand->pNextCmd->pNextCmd	= (FFT_COMMAND *) NULL;
		pCommand->pNextCmd->fnCmd		= pa_fnCmd;
		strcpy(pCommand->pNextCmd->cmdName, pa_cmdName);
	}

	return FFT_ERR_NONE;
}

FFT_COMMAND *FFTerm_GetCmd(FFT_CONSOLE *pConsole, FF_T_INT8 *pa_cmdName, FF_T_SINT8 *pError) {
	
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
		while(pCommand->pNextCmd != NULL) {
			if(strcmp(pCommand->cmdName, pa_cmdName) == 0) {
				return pCommand;
			}
		}
	}

	if(pError) {
		*pError = FFT_ERR_CMD_NOT_FOUND;
	}

	return (FFT_COMMAND *) NULL;
}

FF_T_SINT8 FFTerm_RemoveCmd(FFT_CONSOLE *pConsole, FF_T_INT8 *pa_cmdName) {

	FFT_COMMAND *pCommand;
	FFT_COMMAND *pRmCmd;
	FF_T_SINT8	Error = FFT_ERR_NONE;
	
	if(!pConsole) {
		return FFT_ERR_NULL_POINTER;
	}

	pCommand = pConsole->pCommands;
	pRmCmd = FFTerm_GetCmd(pConsole, pa_cmdName, &Error);

	if(Error) {
		return Error;
	}

	if(pRmCmd) {
		while(pCommand != NULL) {
			if(pCommand == pRmCmd) {
				pCommand = pRmCmd->pNextCmd;
				return FFT_ERR_NONE;
			}
			pCommand = pCommand->pNextCmd;
		}
	}

	return FFT_ERR_CMD_NOT_FOUND;
}