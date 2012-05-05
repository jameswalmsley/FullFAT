/**
 *	FullFAT Verification Executioner.
 **/

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include "verification.h"

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


int main(int argc, char **argv) {
	printf("Verifying FullFAT\n");
	load_module("tests/fs/misc/verify.fs.misc.so");
	return 0;
}
