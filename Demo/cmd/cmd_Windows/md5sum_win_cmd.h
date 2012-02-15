#ifndef _MD5SUM_WIN_CMD_H_
#define _MD5SUM_WIN_CMD_H_

#include "../cmd_helpers.h"
#include "../../../src/fullfat.h"
#include "../../../../ffterm/src/ffterm.h"
#include "../md5.h"

int md5sum_win_cmd(int argc, char **argv);
extern const FFT_ERR_TABLE md5sum_win_Info[];

#endif
