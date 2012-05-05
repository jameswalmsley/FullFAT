#ifndef _VERIFICATION_H_
#define _VERIFICATION_H_

#include <fullfat.h>

#define FAIL 0
#define PASS 1

typedef int (*TEST_FUNCTION) (FF_IOMAN *pIoman, const char **pszpMessage);

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
