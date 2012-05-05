#include <verification.h>
#include <fullfat.h>
#include <stdio.h>
#include <md5.h>
#include "test_1.h"


int test_2(FF_IOMAN *pIoman, TEST_PARAMS *pParams);
int test_3(FF_IOMAN *pIoman, TEST_PARAMS *pParams);
int test_4(FF_IOMAN *pIoman, TEST_PARAMS *pParams);
int test_5(FF_IOMAN *pIoman, TEST_PARAMS *pParams);
int test_6(FF_IOMAN *pIoman, TEST_PARAMS *pParams);
int test_7(FF_IOMAN *pIoman, TEST_PARAMS *pParams);
int test_8(FF_IOMAN *pIoman, TEST_PARAMS *pParams);
int test_9(FF_IOMAN *pIoman, TEST_PARAMS *pParams);

static const VERIFICATION_TEST tests[] = {
	{
		"Unaligned Access",
		"Small repeated unaligned byte write access.",
		"James Walmsley <james@fullfat-fs.co.uk>",
		test_1,
	},
	{
		"Re-arrange Text file",
		"Stresses FullFAT with text file re-arrangement",
		"James Walmsley <james@fullfat-fs.co.uk>",
		test_2,
	},
	{
		"Fill Root Dir (FAT16)",
		"Attempts to cause a root dir overflow",
		"James Walmsley <james@fullfat-fs.co.uk>",
		test_3,
	},
	{
		"Wildcard Search",
		"Attempts to find files matching wildcards",
		"James Walmsley <james@fullfat-fs.co.uk>",
		test_4,
	},
	{
		"Wildcard Algorithm",
		"Tests the wildcard algorithm thoroughly",
		"James Walmsley <james@fullfat-fs.co.uk>",
		test_5,
	},
	{
		"Sequential Append/Seek",
		"Appends and seeks within multiple files.",
		"James Walmsley <james@fullfat-fs.co.uk>",
		test_6,
	},
	{
		"FF_PutC() Across Sector/Clusters",
		"Verifies the FF_PutC() functions across boundaries",
		"James Walmsley <james@fullfat-fs.co.uk>",
		test_7,
	},
	{
		"Write to File and re-open",
		"Validating pBuf problem",
		"James Walmsley <james@fullfat-fs.co.uk>",
		test_8,
	},
	{
		"Directory Filler",
		"Verify non-root directory fill",
		"James Walmsley <james@fullfat-fs.co.uk>",
		test_9,
	},
};

static const VERIFICATION_INTERFACE verify = {
	"Miscellaneous FS Verification",
	"Test cases to reproduce bugs reported by 3rd parties not easily categorised",
	"James Walmsley <james@fullfat-fs.co.uk>",
	sizeof(tests)/sizeof(VERIFICATION_TEST),
	tests,
};


const VERIFICATION_INTERFACE getVerificationIF() {
	return verify;
}
