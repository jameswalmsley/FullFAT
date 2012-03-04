#ifndef _FSINFO_CMD_
#define _FSINFO_CMD_

#include "cmd_helpers.h"
#include "../../src/fullfat.h"
#include "../../../ffterm/src/ffterm.h"

int fsinfo_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv);
extern const FFT_ERR_TABLE fsinfoInfo[];

#endif
