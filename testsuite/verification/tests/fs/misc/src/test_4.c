#include <verification.h>
#include <fullfat.h>

int test_4(FF_IOMAN *pIoman, TEST_PARAMS *pParams) {
	FF_FILE *pFile;
	FF_ERROR Error;

	Error = FF_MkDir(pIoman, "/wildcard");
	CHECK_ERR(Error);

	pFile = FF_Open(pIoman, "/wildcard/test1.dat", FF_MODE_CREATE, &Error);
	if(!pFile) {
		CHECK_ERR(Error);
	}

	Error = FF_Close(pFile);
	CHECK_ERR(Error);

	pFile = FF_Open(pIoman, "/wildcard/test2.dat", FF_MODE_CREATE, &Error);
	if(!pFile) {
		CHECK_ERR(Error);
	}

	Error = FF_Close(pFile);
	CHECK_ERR(Error);

	pFile = FF_Open(pIoman, "/wildcard/test1.bat", FF_MODE_CREATE, &Error);
	if(!pFile) {
		CHECK_ERR(Error);
	}

	Error = FF_Close(pFile);
	CHECK_ERR(Error);

	pFile = FF_Open(pIoman, "/wildcard/test2.bat", FF_MODE_CREATE, &Error);
	if(!pFile) {
		CHECK_ERR(Error);
	}

	Error = FF_Close(pFile);
	CHECK_ERR(Error);

	pFile = FF_Open(pIoman, "/wildcard/test1", FF_MODE_CREATE, &Error);
	if(!pFile) {
		CHECK_ERR(Error);
	}

	Error = FF_Close(pFile);
	CHECK_ERR(Error);

	pFile = FF_Open(pIoman, "/wildcard/test2", FF_MODE_CREATE, &Error);
	if(!pFile) {
		CHECK_ERR(Error);
	}

	Error = FF_Close(pFile);
	CHECK_ERR(Error);

	Error = FF_RmFile(pIoman, "/wildcard/test1.dat"); 	CHECK_ERR(Error);
	Error = FF_RmFile(pIoman, "/wildcard/test2.dat"); 	CHECK_ERR(Error);
	Error = FF_RmFile(pIoman, "/wildcard/test1.bat"); 	CHECK_ERR(Error);
	Error = FF_RmFile(pIoman, "/wildcard/test2.bat"); 	CHECK_ERR(Error);
	Error = FF_RmFile(pIoman, "/wildcard/test1"); 		CHECK_ERR(Error);
	Error = FF_RmFile(pIoman, "/wildcard/test2"); 		CHECK_ERR(Error);
	Error = FF_RmDir(pIoman, "/wildcard");				CHECK_ERR(Error);

	return PASS;
}
