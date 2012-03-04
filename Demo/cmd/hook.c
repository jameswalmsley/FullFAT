/**
 *	This file simply hooks all the commands into the FFTerminal.
 **/

#include "commands.h"

int hook_commands(FF_ENVIRONMENT *pEnv) {
	FFT_CONSOLE *pConsole = pEnv->pConsole;

	FFTerm_AddExCmd(pConsole, "cd",			(FFT_FN_COMMAND_EX) cd_cmd, 		cdInfo,			pEnv);
	FFTerm_AddExCmd(pConsole, "cp",			(FFT_FN_COMMAND_EX) cp_cmd, 		cpInfo,			pEnv);
	FFTerm_AddExCmd(pConsole, "ls",			(FFT_FN_COMMAND_EX) ls_cmd,			lsInfo,			pEnv);
	FFTerm_AddExCmd(pConsole, "md5sum",		(FFT_FN_COMMAND_EX) md5sum_cmd,		md5sumInfo,		pEnv);
	FFTerm_AddExCmd(pConsole, "mkdir",		(FFT_FN_COMMAND_EX) mkdir_cmd,		mkdirInfo,		pEnv);
	FFTerm_AddExCmd(pConsole, "prompt",		(FFT_FN_COMMAND_EX) cmd_prompt,		cmdpromptInfo,	pEnv);
	FFTerm_AddExCmd(pConsole, "pwd",		(FFT_FN_COMMAND_EX)	pwd_cmd,		pwdInfo,		pEnv);
	FFTerm_AddExCmd(pConsole, "fsinfo",		(FFT_FN_COMMAND_EX) fsinfo_cmd,		fsinfoInfo,		pEnv);
	FFTerm_AddExCmd(pConsole, "testsuite",	(FFT_FN_COMMAND_EX) cmd_testsuite,	NULL,			pEnv);
	FFTerm_AddExCmd(pConsole, "more",		(FFT_FN_COMMAND_EX) more_cmd,		moreInfo,		pEnv);
	FFTerm_AddExCmd(pConsole, "hex",		(FFT_FN_COMMAND_EX) hexview_cmd,	hexviewInfo,	pEnv);
	FFTerm_AddExCmd(pConsole, "mkfile",		(FFT_FN_COMMAND_EX) mkfile_cmd,		mkfileInfo,		pEnv);

	return 0;
}
