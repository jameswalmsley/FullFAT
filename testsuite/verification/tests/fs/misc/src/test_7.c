#include <verification.h>

int test_7(FF_IOMAN *pIoman, TEST_PARAMS *pParams) {
	FF_FILE *pFile;
	FF_ERROR Error;
	int i;
	int c;
	unsigned long size = pIoman->pPartition->SectorsPerCluster * pIoman->pPartition->BlkSize;
	unsigned long startSize;
	char test[] = "Another simple sentence.";
	

	pFile = FF_Open(pIoman, "\\test.7.dat", FF_GetModeBits("a+"), &Error);
	if(!pFile) { CHECK_ERR(Error) }

	startSize = pFile->Filesize;

	for(i = startSize; i < (size * 3)+startSize; i++) {
		Error = FF_PutC(pFile, test[i % (sizeof(test)-1)]);
	}

	Error = FF_Close(pFile);
	CHECK_ERR(Error);

	pFile = FF_Open(pIoman, "\\test.7.dat", FF_GetModeBits("rb"), &Error);
	if(!pFile) { CHECK_ERR(Error) }

	i = 0;

	do {
		c = FF_GetC(pFile);
		if(c >= 0) {
			if(c != test[i++ % (sizeof(test)-1)]) {
				sprintf(pParams->errBuf, "Char at offset: %d does not match input.\n", i);
				*pParams->pszpMessage = pParams->errBuf;
				FF_Close(pFile);
				DO_FAIL;
			}
		}
	} while(c >= 0);

	FF_Close(pFile);

	return PASS;
}
