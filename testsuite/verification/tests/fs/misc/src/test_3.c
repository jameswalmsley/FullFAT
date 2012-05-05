#include <verification.h>
#include <fullfat.h>

int test_3(FF_IOMAN *pIoman, TEST_PARAMS *pParams) {
	int i;
	char namebuffer[128];
	FF_FILE *pFile;
	FF_ERROR Error;

	if(pIoman->pPartition->Type == FF_T_FAT32) {
		*pParams->pszpMessage = "Test not applicable for FAT32 formatted media!";
		return PASS;
	}

	for(i = 0; i < 0xFFFF; i++) {
		sprintf(namebuffer, "tst%d.tst", i);
		pFile = FF_Open(pIoman, namebuffer, FF_MODE_WRITE|FF_MODE_CREATE, &Error);
		if(!pFile) {
			CHECK_ERR(Error);
		}
		FF_Close(pFile);
	}

	return PASS;
}
