#include <verification.h>

typedef struct {
	const char *string;
	const char *wc;
	int	expectMatch;
} WC_TABLE;

int test_5(FF_IOMAN *pIoman, TEST_PARAMS *pParams) {
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

		if(FF_wildcompare(tests[i].wc, tests[i].string) == tests[i].expectMatch) {
			if(pParams->bPrintDebug) {
				printf("PASS : WC:(Match=%d)", tests[i].expectMatch);
			}
		} else {
			if(pParams->bPrintDebug) {
				printf("FAIL : WC:(Match=%d)", tests[i].expectMatch);
			}
			bFail = 1;
		}

		if(pParams->bPrintDebug) {
			printf("Comp %-16s with %-12s:\n", tests[i].string, tests[i].wc);
		}
	}

	if(bFail) {
		DO_FAIL;
	}

	*pParams->pszpMessage = "Amazing!";
	return PASS;
}
