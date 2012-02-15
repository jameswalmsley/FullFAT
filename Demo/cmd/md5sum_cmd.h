#ifndef _MD5SUM_CMD_H_
#define _MD5SUM_CMD_H_

#include "cmd_helpers.h"
#include "../../src/fullfat.h"
#include "../../../ffterm/src/ffterm.h"
#include "md5.h"

int md5sum_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv);
extern const FFT_ERR_TABLE md5sumInfo[];

#endif
