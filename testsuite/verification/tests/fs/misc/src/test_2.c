#include <verification.h>
#include <fullfat.h>

int test_2(FF_IOMAN *pIoman, TEST_PARAMS *pParams) {
	FF_FILE *pFile;
	FF_ERROR Error;
	unsigned char buffer[8192];
	unsigned char buffer2[8192];

	memset(buffer, 0, 8192);
	memset(buffer2, 0, 8192);

	sprintf((char *) buffer, "Hello World\n");

	Error = FF_RmFile(pIoman, "\\test.dat");
	CHECK_ERR(Error);

	pFile = FF_Open(pIoman, "\\test.dat", FF_MODE_READ|FF_MODE_WRITE|FF_MODE_CREATE, &Error);
	CHECK_ERR(Error);

	Error = FF_Write(pFile, 1, strlen((char *)buffer), buffer);
	CHECK_ERR(Error);

	Error = FF_Close(pFile);
	CHECK_ERR(Error);

	pFile = FF_Open(pIoman, "\\test.dat", FF_MODE_READ|FF_MODE_WRITE, &Error);
	if(!pFile) { CHECK_ERR(Error);}

	Error = FF_Seek(pFile, 1, FF_SEEK_SET);
	CHECK_ERR(Error);

	buffer[pFile->FilePointer] = 'J';
	Error = FF_PutC(pFile, 'J');
	CHECK_ERR(Error);

	Error = FF_Seek(pFile, 5, FF_SEEK_SET);
	CHECK_ERR(Error);

	Error = FF_Read(pFile, 1, 1, buffer2);
	CHECK_ERR(Error);

	buffer[pFile->FilePointer] = '_';
	Error = FF_PutC(pFile, '_');
	CHECK_ERR(Error);

	Error = FF_Close(pFile);
	CHECK_ERR(Error);

	pFile = FF_Open(pIoman, "\\test.dat", FF_MODE_READ, &Error);
	if(!pFile) { CHECK_ERR(Error);}

	Error = FF_Read(pFile, 1, 8192, buffer2);
	CHECK_ERR(Error);

	Error = FF_Close(pFile);
	CHECK_ERR(Error);

	if(!memcmp(buffer, buffer2, 8192)) {
		return PASS;
	}

	DO_FAIL;
}
