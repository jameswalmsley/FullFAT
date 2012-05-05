#include <verification.h>
#include <md5.h>

int i, di;
md5_state_t 		state;
md5_byte_t			digest[16];
md5_byte_t			digest2[16];
char				szpHash[255];
char				temp[3];

static char *get_md5(unsigned char *data, size_t len, char *hashbuf, size_t hashlen) {
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

int test_6(FF_IOMAN *pIoman, TEST_PARAMS *pParams) {
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
	if(FFTerm_Exec(pParams->pConsole, (char *)buffer)) {
		DO_FAIL;
	}

	sprintf((char *)buffer, "md5sum -v %s %s", md5[1], "test2.txt");
	if(FFTerm_Exec(pParams->pConsole, (char *)buffer)) {
		DO_FAIL;
	}

	return PASS;
}
