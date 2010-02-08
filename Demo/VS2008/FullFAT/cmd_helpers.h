/*
	Provides a library to help command lines!
*/

typedef struct {
	int optind;
	int nextchar;
	int optopt;
	const char *optarg;
	//bool bDone;
} FF_GETOPT_CONTEXT;

int FF_getopt(int argc, const char **argv, const char *optstring, FF_GETOPT_CONTEXT *ctx);
const char *FF_getarg(int argc, const char **argv, int idx);