#include "geterror_cmd.h"

int geterror_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	const char			*szpSource;// *szpDestination, *szpWildCard;
	//char				szsrcPath[FF_MAX_PATH], szdestPath[FF_MAX_PATH];
	//FF_DIRENT			findData;
	FF_ERROR			Error = FF_ERR_NONE;
	
	FFT_GETOPT_CONTEXT	optionContext;	// CommandLine processing
	//FF_T_BOOL			bRecursive = FF_FALSE, bVerbose = FF_FALSE;	// Option Flags.
	//int 				option;
	pEnv = NULL;

	memset(&optionContext, 0, sizeof(FFT_GETOPT_CONTEXT));			// Initialise the option context to zero.

	//option = FFTerm_getopt(argc, argv, "", &optionContext);			// Get the command line option charachters.

	szpSource 		= FFTerm_getarg(argc, (const char **) argv, 0, &optionContext);	// The remaining options or non optional arguments.
	if(!szpSource) {
		printf("Please provide an error code like: 0x81010002\n");
		return 0;
	}

	if(szpSource[0] == '0' && (szpSource[1] == 'x' || szpSource[1] == 'X')) {
		szpSource += 2;
	}

	sscanf(szpSource, "%08X", (unsigned int *) &Error);

	if(!Error) {
		printf("Please provide an error code like: 0x81010002\n");
		return 0;
	}

	PrintError(Error);

	return 0;
}

const FFT_ERR_TABLE geterrorInfo[] =
{
	{"Generic or Unknown Error",	 -1},
	{"Lookup a FullFAT error code.", FFT_COMMAND_DESCRIPTION},
	{ NULL }
};
