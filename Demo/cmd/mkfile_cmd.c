
#include "mkfile_cmd.h"

int mkfile_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	
	FF_FILE *f;
	FF_T_UINT32	Bytes;
	FF_T_UINT32	BytesWritten = 0;
	FF_T_UINT32 ElementSize = 0, Elements = 0, Multiplier = 0;
	FF_T_UINT8 IntBuffer[4096*4];	// 16Kb of Integers!
	FF_T_UINT32	i = 0, x;
	FF_T_INT8	path[FF_MAX_PATH];
	FF_ERROR	Error;

	FF_T_UINT64 TicksPerSecond, TicksStart, TicksEnd, Ticks;

	float transferRate = 0.0, time;

	TicksPerSecond = FFTerm_GetTickRate(pEnv->pConsole);

	if(argc == 5) {
		sscanf(argv[1], "%lu", &ElementSize);
		/*if(!ElementSize) {
			printf("Invalid Element Size!\n");
			return 0;
			}*/

		sscanf(argv[2], "%lu", &Elements);
		/*if(!Elements) {
			printf("Invalid Number of Elements\n");
			return 0;
			}*/

		sscanf(argv[3], "%lu", &Multiplier);
		/*if(!Multiplier) {
			printf("Invalid Multiplier\n");
			return 0;
			}*/

		Bytes = ElementSize * Elements * Multiplier;

		printf("Creating file of size %lu Bytes (%0.2f MB) (%0.3f GB)\n", Bytes, (float)((float)Bytes / 1048576.0), (float)(((float)Bytes / 1048576.0)/1024.0));
		
		ProcessPath(path, argv[4], pEnv);

		f = FF_Open(pEnv->pIoman, path, FF_GetModeBits("wb"), &Error);

		if(f) {
			while(Bytes) {
				
				for(x = 0; x < 4096*4; x++) {
					IntBuffer[x] = (FF_T_UINT8)i++;
				}
				
				TicksStart = FFTerm_GetTicks(pEnv->pConsole);
				if(Bytes >= (4096 * 4)) {
					BytesWritten += 4096 * 4;
					Bytes -= FF_Write(f, 1, 4096 * 4, (FF_T_UINT8 *) IntBuffer);
				} else {
					BytesWritten += Bytes;
					Bytes -= FF_Write(f, 1, Bytes, (FF_T_UINT8 *) IntBuffer);
				}
				TicksEnd = FFTerm_GetTicks(pEnv->pConsole);
				
				Ticks += (TicksEnd - TicksStart);
				time = ((float)Ticks/(float)TicksPerSecond);
				transferRate = (BytesWritten / time) / 1024;

				printf("Written %0.2f MB (%7.2f KB/s)\r", (float) ((float)BytesWritten / 1048576.0), transferRate);
			}

			printf("Written %0.2f MB (%7.2f KB/s)\n", (float) ((float)BytesWritten / 1048576.0), transferRate);

			FF_Close(f);
		} else {
			printf("Error opening file: %s\n", FF_GetErrMessage(Error));
		}
		
	} else {
		printf("Generates a File filled with 32-bit integers.\n\n");
		printf("Usage: %s [Element Size] [Elements] [Multiplier] [filename]\n\n", argv[0]);
		printf("E.g. a 1Mb File, \tFullFAT\\>%s 1024\t 1024\t 1\t 1m.dat\n", argv[0]);
		printf("E.g. a 2Mb File, \tFullFAT\\>%s 1024\t 1024\t 2\t 2m.dat\n", argv[0]);
		printf("E.g. a 10Mb File, \tFullFAT\\>%s 1024\t 1024\t 10\t 10m.dat\n", argv[0]);
		printf("E.g. a 100Mb File, \tFullFAT\\>%s 1024\t 1024\t 100\t 100m.dat\n\n", argv[0]);
	}

	return 0;
}
const FFT_ERR_TABLE mkfileInfo[] =
{
	{"Unknown or Generic Error",				-1},	// Generic Error must always be the first in the table.
	{"Generates a File of the specified size.",	FFT_COMMAND_DESCRIPTION},
	{ NULL }
};
