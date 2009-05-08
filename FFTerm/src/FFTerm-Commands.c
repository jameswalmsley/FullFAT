#include "FFTerm-Commands.h"

FF_T_SINT32 FFTerm_CmdExit(FF_T_SINT32 argc, FF_T_INT8 **argv) {
	return FFT_KILL_CONSOLE;
}

FF_T_SINT32 FFTerm_CmdHelp(FF_T_SINT32 argc, FF_T_INT8 **argv, FFT_CONSOLE *pConsole) {
	FFT_COMMAND *pCommand = pConsole->pCommands;
	printf("Available Commands:\n");
	
	while(pCommand) {
		printf("%s\n", pCommand->cmdName);
		pCommand = pCommand->pNextCmd;
	}

	printf("For more information about each command, type [command name] help\n");
	
	return FFT_ERR_NONE;
}

FF_T_SINT32 FFTerm_HookDefaultCommands(FFT_CONSOLE *pConsole) {
	FFTerm_AddCmd(pConsole, "exit", (FFT_FN_COMMAND)FFTerm_CmdExit, NULL);
	FFTerm_AddExCmd(pConsole, "help", FFTerm_CmdHelp, NULL, pConsole);
	return FFT_ERR_NONE;
}

