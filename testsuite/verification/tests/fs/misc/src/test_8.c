#include <verification.h>


int test_8(FF_IOMAN *pIoman, TEST_PARAMS *pParams) {
	FF_FILE *pFile;
	FF_ERROR Error;
	int i;

	char *testString = "This is a test string 012345678\n";

	pFile = FF_Open(pIoman, "\\test.8.dat", FF_GetModeBits("wb"), &Error);
	if(!pFile) { CHECK_ERR(Error) }

	/*p = malloc(1024*1024*1024);
	if(p) {
		free(p);
	}*/

	for(i = 0; i < 16*5; i++) {
		Error = FF_Write(pFile, 1, strlen(testString), (unsigned char *)testString);
		CHECK_ERR(Error);
	}

	Error = FF_Close(pFile);
	CHECK_ERR(Error);

	// Now attempt to open in r+ mode.

	pFile = FF_Open(pIoman, "\\test.8.dat", FF_GetModeBits("r+"), &Error);
	if(!pFile) { CHECK_ERR(Error) }

	for(i = 0; i < 512; i++) {
		FF_GetC(pFile);
	}

	Error = FF_PutC(pFile, '*');

	FF_Close(pFile);

	return PASS;
}
