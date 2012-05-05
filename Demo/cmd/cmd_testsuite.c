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

static FF_ENVIRONMENT *g_pEnv = NULL;


#define FAIL 0
#define PASS 1

int i, di;
md5_state_t 		state;
md5_byte_t			digest[16];
md5_byte_t			digest2[16];
char				szpHash[255];
char				temp[3];

char errBuf[1024];

static int bPrintDebug = 0;

static char *message = NULL;

#define DO_FF_FAIL(x)	printf("FAILED Line:%d : %s\n", __LINE__, __FILE__);  FF_GetErrDescription(x, errBuf, 1024); \
						printf("%s\n", errBuf); return FAIL

#define DO_FAIL			printf("FAILED Line:%d : %s\n", __LINE__, __FILE__);  return FAIL

#define CHECK_ERR(x)	{if(FF_isERR(x)) {DO_FF_FAIL(x);}}

char *get_md5(unsigned char *data, size_t len, char *hashbuf, size_t hashlen) {
	int i;
	char temp[3];
	
	md5_init(&state);
	for(i = 0; i < len; i++) {
		md5_append(&state, (const md5_byte_t *) data +i, 1);
	}

	md5_finish(&state, digest);

	strcpy(hashbuf,"");

	for(i = 0; i < 16; i++) {
		sprintf(temp, "%02x", digest[i]);
		strcat(hashbuf, temp);
	}
	
	return hashbuf;
}

int test_1(FF_IOMAN *pIoman) {
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

int test_2(FF_IOMAN *pIoman) {
	FF_FILE *pFile;
	FF_ERROR Error;
	unsigned char buffer[8192];
	unsigned char buffer2[8192];

	memset(buffer, 0, 8192);
	memset(buffer2, 0, 8192);

	sprintf((char *)buffer, "Hello World\n");

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

	Error = FF_RmFile(pIoman, "/wildcard/test1.dat"); 	CHECK_ERR(Error);
	Error = FF_RmFile(pIoman, "/wildcard/test2.dat"); 	CHECK_ERR(Error);
	Error = FF_RmFile(pIoman, "/wildcard/test1.bat"); 	CHECK_ERR(Error);
	Error = FF_RmFile(pIoman, "/wildcard/test2.bat"); 	CHECK_ERR(Error);
	Error = FF_RmFile(pIoman, "/wildcard/test1"); 		CHECK_ERR(Error);
	Error = FF_RmFile(pIoman, "/wildcard/test2"); 		CHECK_ERR(Error);
	Error = FF_RmDir(pIoman, "/wildcard");				CHECK_ERR(Error);

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
		if(bPrintDebug) {
			printf("Matching %-16s with %-16s:  ", tests[i].string, tests[i].wc);
		}
		if(FF_wildcompare(tests[i].wc, tests[i].string) == tests[i].expectMatch) {
			if(bPrintDebug) {
				printf("PASS (Match=%d)\n", tests[i].expectMatch);
			}
		} else {
			if(bPrintDebug) {
				printf("FAIL (Match=%d)\n", tests[i].expectMatch);
			}
			bFail = 1;
		}
	}

	if(bFail) {
		DO_FAIL;
	}

	return PASS;
}

int test_6(FF_IOMAN *pIoman) {
	int i;
	FF_FILE *pFile;
	FF_ERROR Error;
	char *buf1 = "This is a test sentence for FullFAT.";
	char *buf2 = "Another simple test sentence for FF.";

	char md5[2][64];
	unsigned char buffer[2048];

	Error = FF_RmFile(pIoman, "\\test1.txt");
	//CHECK_ERR(Error);

	Error = FF_RmFile(pIoman, "\\test2.txt");
	//CHECK_ERR(Error);

	for(i = 0; i < 20; i++) {
		pFile = FF_Open(pIoman, "\\test1.txt", FF_GetModeBits("a+"), &Error);
		if(!pFile) { CHECK_ERR(Error); }

		Error = FF_Seek(pFile, 0, FF_SEEK_END); 						CHECK_ERR(Error);		
		Error = FF_Write(pFile, 1, strlen(buf1), (FF_T_UINT8 *) buf1); 	CHECK_ERR(Error);
		Error = FF_Close(pFile); 										CHECK_ERR(Error);

		pFile = FF_Open(pIoman, "\\test2.txt", FF_GetModeBits("a+"), &Error);
		if(!pFile) { CHECK_ERR(Error); }

		Error = FF_Seek(pFile, 0, FF_SEEK_END);							CHECK_ERR(Error);
		Error = FF_Write(pFile, 1, strlen(buf2), (FF_T_UINT8 *) buf2);	CHECK_ERR(Error);
		Error = FF_Close(pFile);										CHECK_ERR(Error);
	}

	// Calculate the MD5 sum of each buffer (what we expect from the file?
	
	memset(buffer, 0, sizeof(buffer));
	for(i = 0; i < 20; i++) {
		sprintf((char *)buffer, "%s%s", buffer, buf1);	// The contents of file 1 should be this.
	}

	get_md5(buffer, 20 * strlen(buf1), md5[0], 33);

	memset(buffer, 0, sizeof(buffer));
	for(i = 0; i < 20; i++) {
		sprintf((char *)buffer, "%s%s", buffer, buf2);
	}

	get_md5(buffer, 20 * strlen(buf2), md5[1], 33);


	// Verify with md5sum commandlet.
	sprintf((char *)buffer, "md5sum -v %s %s", md5[0], "test1.txt");
	if(FFTerm_Exec(g_pEnv->pConsole, (char *)buffer)) {
		DO_FAIL;
	}

	sprintf((char *)buffer, "md5sum -v %s %s", md5[1], "test2.txt");
	if(FFTerm_Exec(g_pEnv->pConsole, (char *)buffer)) {
		DO_FAIL;
	}

	return PASS;
}

int test_7(FF_IOMAN *pIoman) {
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
				sprintf(errBuf, "Char at offset: %d does not match input.\n", i);
				message = errBuf;
				FF_Close(pFile);
				DO_FAIL;
			}
		}
	} while(c >= 0);

	FF_Close(pFile);

	return PASS;
}

int test_8(FF_IOMAN *pIoman) {
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

static const TEST_ITEM tests[] = {
	{test_1,		"Small repeated unaligned byte write access."},
	{test_2,		"Re-arrange Text file."},
	{test_3,		"Fill-up root dir!. (Test FAT16)."},
	{test_4,		"Testing wildcard searching."},
	{test_5,		"Testing wildcard algorithm."},
	{test_6,		"Sequential append and seek of multiple files."},
	{test_7,		"Testing FF_PutC() writing across sectors/clusters."},
	{test_8,		"Write a file, and then re-open validate pbuf."},
};

static int exec_test(FF_IOMAN *pIoman, int testID) {
	int bFail;
	message = " ";
	printf("%4d : ", testID);
	if(tests[testID].pfnTest(pIoman)) {
		printf("PASS : %-50s : %s\n", tests[testID].description, message);
		bFail = 0;
	} else {
		printf("FAIL : %-50s : %s\n", tests[testID].description, message);
		bFail = 1;
	}

	return bFail;
}

int cmd_testsuite(int argc, char **argv, FF_ENVIRONMENT *pEnv) {

	int i;
	int bFail = 0;

	g_pEnv = pEnv;

	printf("Thankyou for helping to verify FullFAT!\n\nStarting tests:\n\n");

	if(argc == 1 || argc == 2) {
		printf("  ID : Description                                     : Execution Message\n");
		printf("--------------------------------------------------------------------------\n");
	}

	if(argc == 1) {	   
		for(i = 0; i < sizeof(tests)/sizeof(TEST_ITEM); i++) {
			if(exec_test(pEnv->pIoman, i)) {
				bFail = 1;
			}
		}
		printf("\n\n");	 		
	}

	if(argc == 2) {
		i = atoi(argv[1]);
		bPrintDebug = 1;
		if(i < 0  || i >= sizeof(tests)/sizeof(TEST_ITEM)) {
			printf("Invalid test ID!\n");
			return -1;
		}

		if(exec_test(pEnv->pIoman, i)) {
			bFail = 1;
		}

		bPrintDebug = 0;

		return 0;
	}

	if(bFail) {
		printf("\nABORTING TESTS - Please contact james@fullfat-fs.co.uk -- THIS MUST BE FIXED!\n");
		printf("\n\n");
		return -1;
	}

	if(argc > 2) {
		printf("Usage:\n");
		printf("%s [test ID]\n", argv[0]);
	}

	return 0;
}
