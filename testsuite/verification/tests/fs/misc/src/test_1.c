#include <verification.h>
#include <fullfat.h>
#include <md5.h>

int i, di;
md5_state_t 		state;
md5_byte_t			digest[16];
md5_byte_t			digest2[16];
char				szpHash[255];
char				temp[3];


int test_1(FF_IOMAN *pIoman, TEST_PARAMS *pParams) {
	*pParams->pszpMessage = "No Error";
	FF_FILE *pFile;
	FF_ERROR Error;
	unsigned char buffer[8192];
	int i,y;

	for(y = 0; y < 2; y++) {
		Error = FF_RmFile(pIoman, "\\test.dat");
		if(FF_GETERROR(Error) != FF_ERR_FILE_NOT_FOUND) {
			CHECK_ERR(Error);
		}

		pFile = FF_Open(pIoman, "\\test.dat", /*FF_GetModeBits("a+")*/ FF_MODE_CREATE|FF_MODE_READ|FF_MODE_WRITE, &Error);
		CHECK_ERR(Error);

		md5_init(&state);
		for(i = 0; i < 8192; i++) {
			buffer[i] = (char) i;
			md5_append(&state, (const md5_byte_t *) buffer + i, 1);
		}

		md5_finish(&state, digest);

		for(i = 0; i < 8192; i++) {
			if(y == 0) {
				Error = FF_PutC(pFile, buffer[i]);
			} else {
				Error = FF_Write(pFile, 1, 1, buffer+i);
			}

			CHECK_ERR(Error);
		}

		FF_Close(pFile);

		// Is on disk stuff same?
		pFile = FF_Open(pIoman, "\\test.dat", FF_MODE_READ, &Error);
		CHECK_ERR(Error);

		Error = FF_Read(pFile, 1, 8192, buffer);
		CHECK_ERR(Error);

		FF_Close(pFile);

		md5_init(&state);
		for(i = 0; i < 8192; i++) {
			buffer[i] = (char) i;
			md5_append(&state, (const md5_byte_t *) buffer + i, 1);
		}
		md5_finish(&state, digest2);

		if(!memcmp(digest, digest2, 16)) {
			return PASS;
		}
	}

	DO_FAIL;
}
