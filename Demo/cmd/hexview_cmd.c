/**
 *	@brief	A basic Hexview application.
 *			Allows file's to be viewed as Hex.
 **/

#include "hexview_cmd.h"

#define HEX_BUF 16

int hexview_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	
	FF_FILE *f;
	FF_ERROR Error;
	char buffer[HEX_BUF];
//	char lines[25][81]; // Screen Buffer
	FF_T_INT8	path[FF_MAX_PATH];
	int read;
	int i;

	printf(" HexView 0.2 for FFTerm and FullFAT\n");
	printf(" By James Walmsley\n");
	printf("--------------------------------------------------------\n");

	if(argc == 2) {
		ProcessPath(path, argv[1], pEnv);
		f = FF_Open(pEnv->pIoman, path, FF_GetModeBits("rb"), &Error);
		if(!f) {
			printf("Error: %s\n", FF_GetErrMessage(Error));
			return -1;
		}
		do{
			read = FF_Read(f, 1, HEX_BUF, (unsigned char *)buffer);
			for(i = 0; i < read; i++) {
				printf("%02X ", buffer[i]);
			}
			if(read < HEX_BUF) {
				for(i = 0; i < (HEX_BUF - read); i++) {
					printf(".");
				}
			}
			printf("\t");
			for(i = 0; i < read; i++) {
				if((buffer[i] >= 32 && buffer[i] <= 128)) {
					printf("%c", buffer[i]);
				} else {
					printf(".");
				}
			}
			printf("\n");
			
		}while(read);

		Error = FF_Close(f);
		
	} else {	
		printf("No file specified!\n");
	}

	return 0;
}
const FFT_ERR_TABLE hexviewInfo[] =
{
	{"Unknown or Generic Error",					-1},	// Generic Error must always be the first in the table.
	{"Simple Hexview application.",					FFT_COMMAND_DESCRIPTION},
	{ NULL }
};
