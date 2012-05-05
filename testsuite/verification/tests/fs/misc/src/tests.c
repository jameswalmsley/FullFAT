#include <verification.h>
#include <fullfat.h>

static int test_1(FF_IOMAN *pIoman, const char **pszpMessage) {
	*pszpMessage = FF_GetErrMessage(0);

	return FAIL;
}

static const VERIFICATION_TEST tests[] = {
	{
		"Unaligned Access",
		"Small repeated unaligned byte write access.",
		"James Walmsley <james@fullfat-fs.co.uk>",
		test_1,
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
