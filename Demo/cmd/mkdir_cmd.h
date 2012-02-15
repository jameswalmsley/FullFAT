#ifndef _MKDIR_CMD_H_
#define _MKDIR_CMD_H_

#include "cmd_helpers.h"
#include "../../src/fullfat.h"
#include "../../../ffterm/src/ffterm.h"

int mkdir_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv);
extern const FFT_ERR_TABLE mkdirInfo[];

#endif
