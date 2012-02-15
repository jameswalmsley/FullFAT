#ifndef _CMD_PROMPT_H_
#define _CMD_PROMPT_H_

#include "cmd_helpers.h"
#include "../../src/fullfat.h"
#include "../../../ffterm/src/ffterm.h"

int cmd_prompt(int argc, char **argv, FF_ENVIRONMENT *pEnv);
extern const FFT_ERR_TABLE cmdpromptInfo[];

#endif
