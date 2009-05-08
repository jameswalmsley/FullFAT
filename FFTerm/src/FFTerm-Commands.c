#include "FFTerm-Commands.h"
#include "FFTerm.h"

const FFT_ERR_TABLE CmdExitErrorTable[] =
{
	"Causes the active console to terminate.",			FFT_COMMAND_DESCRIPTION,
	NULL
};

FF_T_SINT32 FFTerm_CmdExit(FF_T_SINT32 argc, FF_T_INT8 **argv) {
	if(argc) {
		return FFT_KILL_CONSOLE;
	}
	return 0;
}

FF_T_SINT32 FFTerm_CmdHelp(FF_T_SINT32 argc, FF_T_INT8 **argv, FFT_CONSOLE *pConsole) {
	FFT_COMMAND *pCommand = pConsole->pCommands;
	printf("\nAvailable Commands:\n\n");
	
	while(pCommand) {
		printf("%s", pCommand->cmdName);
		if(pCommand->ErrTable) {
			printf("\t - %s", FFTerm_LookupErrMessage(pCommand->ErrTable, FFT_COMMAND_DESCRIPTION));
		}
		printf("\n");
		pCommand = pCommand->pNextCmd;
	}

	printf("\nFor more information about each command, type [command name] help\n\n");
	
	return FFT_ERR_NONE;
}

FF_T_SINT32 FFTerm_HookDefaultCommands(FFT_CONSOLE *pConsole) {
	FFTerm_AddCmd(pConsole, "exit", (FFT_FN_COMMAND)FFTerm_CmdExit, CmdExitErrorTable);
	FFTerm_AddExCmd(pConsole, "help", (FFT_FN_COMMAND_EX)FFTerm_CmdHelp, NULL, pConsole);
	return FFT_ERR_NONE;
}

