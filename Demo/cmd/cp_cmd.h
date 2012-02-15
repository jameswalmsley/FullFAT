#ifndef _CP_CMD_H_
#define _CP_CMD_H_

#include "cmd_helpers.h"
#include "../../src/fullfat.h"
#include "../../../ffterm/src/ffterm.h"

int cp_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv);
extern const FFT_ERR_TABLE cpInfo[];

#define CP_BUFFER_SIZE	FF_CMD_BUFFER_SIZE		// This setting is important to test FullFAT's true speed potential.
												// Increasing this value will reduce the operating system overhead.

#endif
