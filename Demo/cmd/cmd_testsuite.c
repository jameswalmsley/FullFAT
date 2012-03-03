/*
	The test-suite runs a series of operationg on the current partition.
	If does a whole manor of file/IO operations and checks the on-disk affected files
	and comapres MD5 sums.

	This proves that no data corruption has occurred because of various combinations of the 
	FullFAT api.

	
*/

#include "cmd_testsuite.h"
#include "md5.h"

typedef int (TEST) (FF_IOMAN *pIoman);

typedef struct {
	TEST		*pfnTest;
	const char *description;
} TEST_ITEM;


#define FAIL 0
#define PASS 1

int i, di;
md5_state_t 		state;
md5_byte_t			digest[16];
md5_byte_t			digest2[16];
char				szpHash[255];
char				temp[3];

char errBuf[1024];

static char *message = NULL;

#define DO_FF_FAIL(x)	printf("FAILED Line:%d : %s\n", __LINE__, __FILE__);  FF_GetErrDescription(x, errBuf, 1024); \
						printf("%s\n", errBuf); return FAIL

#define DO_FAIL			printf("FAILED Line:%d : %s\n", __LINE__, __FILE__);  return FAIL

#define CHECK_ERR(x)	{if(FF_isERR(x)) {DO_FF_FAIL(x);}}

int test_1(FF_IOMAN *pIoman) {
	FF_FILE *pFile;
	FF_ERROR Error;
	char buffer[8192];
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

int test_2(FF_IOMAN *pIoman) {
	FF_FILE *pFile;
	FF_ERROR Error;
	char buffer[8192];
	char buffer2[8192];

	memset(buffer, 0, 8192);
	memset(buffer2, 0, 8192);

	sprintf(buffer, "Hello World\n");

	Error = FF_RmFile(pIoman, "\\test.dat");
	CHECK_ERR(Error);

	pFile = FF_Open(pIoman, "\\test.dat", FF_MODE_READ|FF_MODE_WRITE|FF_MODE_CREATE, &Error);
	CHECK_ERR(Error);

	Error = FF_Write(pFile, 1, strlen(buffer), buffer);
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

int test_3(FF_IOMAN *pIoman) {
	int i;
	char namebuffer[128];
	FF_FILE *pFile;
	FF_ERROR Error;

	if(pIoman->pPartition->Type == FF_T_FAT32) {
		message = "Test not applicable for FAT32 formatted media!";
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


int test_4(FF_IOMAN *pIoman) {
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
	

	return PASS;
}

typedef struct {
	const char *string;
	const char *wc;
	int	expectMatch;
} WC_TABLE;

int test_5(FF_IOMAN *pIoman) {
	WC_TABLE tests[] = {
		{"boot.txt",	"*",		1},
		{"bootdir",		"*",		1},
		{"myfiles.lst",	"*",		1},
		
		{"boot.txt",	"*.*",		1},
		{"bootdir",		"*.*",		0},
		{"myfiles.lst",	"*.*",		1},

		{"boot.txt",	"*.???",	1},
		{"bootdir",		"*.???",	0},
		{"myfiles.lst",	"*.???",	1},

		{"boot.txt",	"*?",		1},
		{"bootdir",		"*?",		1},
		{"myfiles.lst",	"*?",		1},

		{"boot.txt",	"*oot*",	1},
		{"bootdir",		"*oot*",	1},
		{"myfiles.lst",	"*oot*",	0},

		{"boot.txt",	"boot*",	1},
		{"bootdir",		"boot*",	1},
		{"myfiles.lst",	"boot*",	0},

		{"boot.txt",	"boot*.*",	1},
		{"bootdir",		"boot*.*",	0},
		{"myfiles.lst",	"boot*.*",	0},

		{"boot.txt",	"b*.*",		1},
		{"bootdir",		"b*.*",		0},
		{"myfiles.lst",	"b*.*",		0},

		{"boot.txt",	"*txt",		1},
		{"bootdir",		"*txt",		0},
		{"myfiles.lst",	"*txt",		0},

		{"boot.txt",	"*.lst",	0},
		{"bootdir",		"*.lst",	0},
		{"myfiles.lst",	"*.lst",	1},

		{"DSC0051.jpg",		"DSC005?.jp*",	1},
		{"DSC0052.jpg",		"DSC005?.jp*",	1},
		{"DSC0062.jpg",		"DSC005?.jp*",	0},
		{"DSC0051.jpeg",	"DSC005?.jp*",	1},
		{"DSC0052.jpeg",	"DSC005?.jp*",	1},
	};

	int bFail = 0;
	int i;

	for(i = 0; i < sizeof(tests)/sizeof(WC_TABLE); i++) {
		printf("Matching %-16s with %-16s:  ", tests[i].string, tests[i].wc);
		if(FF_wildcompare(tests[i].wc, tests[i].string) == tests[i].expectMatch) {
			printf("PASS (Match=%d)\n", tests[i].expectMatch);
		} else {
			printf("FAIL (Match=%d)\n", tests[i].expectMatch);
			bFail = 1;
		}
	}

	if(bFail) {
		DO_FAIL;
	}

	return PASS;
}

const TEST_ITEM tests[] = {
	{test_5,		"Testing wildcard algorithm."},
	{test_1,		"Small repeated unaligned byte write access. (FF_PutC()/FF_Write())."},
	{test_2,		"Re-arrange Text file."},
	{test_3,		"Fill-up root dir!. (Test FAT16)."},
	{test_4, 		"Testing wildcard searching."},
};

int cmd_testsuite(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	
	int i;

	printf("Thankyou for helping to verify FullFAT!\n\nStarting tests:\n\n");

	if(argc == 1) {

		for(i = 0; i < sizeof(tests)/sizeof(TEST_ITEM); i++) {
			message = NULL;
			if(tests[i].pfnTest(pEnv->pIoman)) {
				printf("PASS : %s : %s\n", tests[i].description, message);
			} else {
				printf("FAIL : %s : %s\n", tests[i].description, message);
				printf("\nABORTING TESTS - Please contact james@fullfat-fs.co.uk -- THIS MUST BE FIXED!\n");
				return -1;
			}
		}

	}

	return 0;
}






																										
