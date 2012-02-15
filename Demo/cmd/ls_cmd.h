#ifndef _LS_CMD_H_
#define _LS_CMD_H_

#include "cmd_helpers.h"
#include "../../src/fullfat.h"
#include "../../../ffterm/src/ffterm.h"

int ls_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv);
extern const FFT_ERR_TABLE lsInfo[];

/*#ifdef WIN32	// Windows has none compliant name for strcasecmp
#define stricmp _stricmp
#define wcsicmp _wcsicmp
#else
#define stricmp strcasecmp
#define wcsicmp	wcscasecmp
#endif*/
#endif
