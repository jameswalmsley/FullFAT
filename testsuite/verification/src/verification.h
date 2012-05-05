#ifndef _VERIFICATION_H_
#define _VERIFICATION_H_

#include <fullfat.h>
#include <ffterm.h>
#include <stdio.h>

#define FAIL 0
#define PASS 1

#define DO_FF_FAIL(x)	printf("FAILED Line:%d : %s\n", __LINE__, __FILE__);  FF_GetErrDescription(x, pParams->errBuf, 1024); \
						printf("%s\n", pParams->errBuf); return FAIL

#define DO_FAIL			printf("FAILED Line:%d : %s\n", __LINE__, __FILE__);  return FAIL

#define CHECK_ERR(x)	{if(FF_isERR(x)) {DO_FF_FAIL(x);}}


typedef struct {
	const char **pszpMessage;
	char *errBuf;
	int bPrintDebug;
	FFT_CONSOLE *pConsole;
} TEST_PARAMS;

typedef int (*TEST_FUNCTION) (FF_IOMAN *pIoman, TEST_PARAMS *pParams);

typedef struct {
	const char 		*szpTestName;			///< Name of the test.
	const char 		*szpTestDescription;	///< Description of the test.
	const char 		*szpTestAuthor;			///< Who wrote the test.
	TEST_FUNCTION 	 pfnTest;				///< Test entry point.
} VERIFICATION_TEST;


typedef struct {
	const char 		*szpModuleName;
	const char 		*szpModuleDescription;
	const char 		*szpModuleAuthor;
	int nTests; 				///< Number of tests in this module.
	const VERIFICATION_TEST *pTests;	///< Pointer to an array of tests.
} VERIFICATION_INTERFACE;

typedef const VERIFICATION_INTERFACE *(*FN_GET_VERIFICATION_IF)();

#endif
