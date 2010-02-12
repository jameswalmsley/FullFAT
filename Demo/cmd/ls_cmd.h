#ifndef _LS_CMD_H_
#define _LS_CMD_H_

#include "cmd_helpers.h"
#include "../../src/FullFAT.h"
#include "../../../ffterm/src/ffterm.h"

int ls_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv);
extern const FFT_ERR_TABLE lsInfo[];

#endif
