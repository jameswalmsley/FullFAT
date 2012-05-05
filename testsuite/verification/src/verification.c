/**
 *	FullFAT Verification Executioner.
 **/

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <fullfat.h>
#include "verification.h"
#include <Drivers/Linux/blkdev_linux.h>	// Prototypes for our Linux 32-bit Address driver.

typedef struct {
	void *lib_handle;
	const VERIFICATION_INTERFACE *pIf;
} TEST_MODULE;

static TEST_MODULE *pModules = NULL;
static int nModules = 0;


void load_module(const char *path) {
	void *lib_handle;
	FN_GET_VERIFICATION_IF pfn = NULL;
	const VERIFICATION_INTERFACE *pIf;
	lib_handle = dlopen(path, RTLD_LAZY);
	if(!lib_handle) {
		fprintf(stderr, "%s\n", dlerror());
		return;
	}

	pfn = dlsym(lib_handle, "getVerificationIF");
	if(!pfn) {
		fprintf(stderr, "Get interface symbol not found\n");
		return;
	}
	
	pIf = pfn();
	if(!pIf) {
		fprintf(stderr, "Invalid interface\n");
		return;
	}

	nModules += 1;
	pModules = realloc(pModules, sizeof(TEST_MODULE)*nModules);
	pModules[nModules-1].lib_handle = lib_handle;
	pModules[nModules-1].pIf = pIf;

	printf("Loaded module: %s\n", pIf->szpModuleName);
}

static int exec_test(FF_IOMAN *pIoman, const VERIFICATION_TEST *pTest) {
	const char *pMessage = "";
	char *pf = "FAIL";
	int bFail = 1;
	if(pTest->pfnTest(pIoman, &pMessage)) {
		pf = "PASS";
		bFail = 0;
	}

	printf("%s : %-50s : %s\n", pf, pTest->szpTestDescription, pMessage);
	return bFail;
}

int main(int argc, char **argv) {

	FF_IOMAN *pIoman = NULL;
	FF_ERROR Error;

	BLK_DEV_LINUX hDisk;

	int i;
	int y;
	const VERIFICATION_INTERFACE *pIf;

	printf("\n");
	printf("Thanks for helping to verify FullFAT\n");
	
	load_module("tests/fs/misc/verify.fs.misc.so");
	printf("\n\n");

	hDisk = fnOpen("ffimage.img", 512);
	if(!hDisk) {
		fprintf(stderr, "Could not open block device driver.\n");
		return -1;
	}

	pIoman = FF_CreateIOMAN(NULL, 8192, 512, &Error);
	if(!pIoman) {
		fprintf(stderr, "FF Error: %s\n", FF_GetErrMessage(Error));
		return 0;
	}

	Error = FF_RegisterBlkDevice(pIoman, GetBlockSize(hDisk), (FF_WRITE_BLOCKS) fnWrite, (FF_READ_BLOCKS) fnRead, hDisk);
	if(FF_isERR(Error)) {
		printf("Error Registering Device\nFF_RegisterBlkDevice() function returned with Error %ld.\nFullFAT says: %s\n", Error, FF_GetErrMessage(Error));
	}

	// Mount!
	Error = FF_MountPartition(pIoman, 0);
	if(FF_isERR(Error)) {
		fprintf(stderr, "Could not mount partition:\n%s\n", FF_GetErrMessage(Error));
		fnClose(hDisk);
		FF_DestroyIOMAN(pIoman);
		return -1;
	}
			

	for(i = 0; i < nModules; i++) {
		pIf = pModules[i].pIf;
		printf("Verifying : %s\n\n", pIf->szpModuleName);
		for(y = 0; y < pIf->nTests; y++) {
			exec_test(pIoman, &pIf->pTests[y]);
		}
	}

	return 0;
}
