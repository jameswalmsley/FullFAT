#include "../../../src/fullfat.h"
#include "../../../FFTerm/src/FFTerm.h"

typedef struct {	// Pass an environment for the FullFAT commands.
	FF_IOMAN	*pIoman;
	FF_T_INT8	WorkingDir[2600];
} FF_ENVIRONMENT;

extern const FFT_ERR_TABLE pwdInfo[];
extern const FFT_ERR_TABLE lsInfo[];
extern const FFT_ERR_TABLE cdInfo[];
extern const FFT_ERR_TABLE md5Info[];

int pwd_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv);
int ls_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv);
int cd_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv);
int md5_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv);

