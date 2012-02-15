/*
	Provides a library to help command lines!
*/

#ifndef _CMD_HELPERS_
#define _CMD_HELPERS_

#define no_argument			0
#define required_argument 	1
#define optional_argument	2

struct option {
	const char *name;
	int has_arg;
	int *flag;
	int val;
};

typedef struct {
	int optind;
	int nextchar;
	int optopt;
	const char *optarg;
} FF_GETOPT_CONTEXT;

typedef struct {
	int 			optind;
	int 			nextchar;
	int 			optopt;
	const char 		*optarg;
	struct option 	*longopts;		///< Pointer to an options structure.
	int				*longindex;		///< Stores the index value through a pointer (as per GNU spec).
	int				longindex_val;	///< Stores the index value here directly, instead.
} FF_GETOPTLONG_CONTEXT;

int FF_getopt			(int argc, const char **argv, const char *optstring, FF_GETOPT_CONTEXT *ctx);
int FF_getopt_long		(int argc, const char **argv, const char *optstring, FF_GETOPTLONG_CONTEXT *ctx);
int FF_getopt_long_only	(int argc, const char **argv, const char *optstring, FF_GETOPTLONG_CONTEXT *ctx);
const char *FF_getarg	(int argc, const char **argv, int argindex);
int	append_filename(char *path, char *filename);

#endif
