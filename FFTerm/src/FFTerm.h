#ifndef _FFTERM_H_
#define _FFTERM_H_

#include "FFTerm-Types.h"
#include "FFTerm-Error.h"

#define FFT_MAX_CMD_NAME		10

typedef int (*FFT_FN_COMMAND) (int argc, char **argv);

typedef struct _FFT_COMMAND {
	FF_T_INT8		cmdName[FFT_MAX_CMD_NAME];
	FFT_FN_COMMAND	fnCmd;
	struct _FFT_COMMAND	*pNextCmd;		///< Pointer to the next command in the linked list.
} FFT_COMMAND;


typedef struct _FF_CONSOLE {
	FFT_COMMAND	*pCommands;
	
} FFT_CONSOLE;


#endif
