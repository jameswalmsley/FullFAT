#ifndef _MKFILE_CMD_
#define _MKFILE_CMD_

#include "cmd_helpers.h"
#include "../../src/fullfat.h"
#include "../../../ffterm/src/ffterm.h"

int mkfile_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv);
extern const FFT_ERR_TABLE mkfileInfo[];

#endif
