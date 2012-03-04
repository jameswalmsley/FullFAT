/**
 *	More/View pretty printer for FullFAT.
 **/

#include "more_cmd.h"

int more_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	FF_FILE		*f;
	FF_ERROR	Error;
	FF_T_SINT32	c;
	FF_T_INT8	path[FF_MAX_PATH];

	if(argc == 2) {

		ProcessPath(path, argv[1], pEnv);

		f = FF_Open(pEnv->pIoman, path, FF_MODE_READ, &Error);
		if(f) {
			printf("//---------- START OF FILE\n");
			while(!FF_isEOF(f)) {
				c = FF_GetC(f);
				if(c >= 0) {
					printf("%c", (FF_T_INT8) c);
				} else {
					printf("Error while reading file: %s\n", FF_GetErrMessage(c));
					FF_Close(f);
					return -3;
				}
			}
			printf("\n//---------- END OF FILE\n");

			FF_Close(f);
		} else {
			printf("Could not open file: %s\n", FF_GetErrMessage(Error));
			return -2;
		}
	} else {
		printf("Usage: %s [filename]\n", argv[0]);
	}
	return 0;
}
const FFT_ERR_TABLE moreInfo[] =
{											
	{"Unknown or Generic Error",				-1},	// Generic Error must always be the first in the table.
	{"Types out the specified file.",			FFT_COMMAND_DESCRIPTION},
	{"File open failed. (File not found?)",		-2},
	{"Error while reading from device!",		-3},
	{ NULL }
};

