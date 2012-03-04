#ifndef _HEXVIEW_CMD_H_
#define _HEXVIEW_CMD_H_

#include "cmd_helpers.h"
#include "../../src/fullfat.h"
#include "../../../ffterm/src/ffterm.h"

int hexview_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv);
extern const FFT_ERR_TABLE hexviewInfo[];

#endif
