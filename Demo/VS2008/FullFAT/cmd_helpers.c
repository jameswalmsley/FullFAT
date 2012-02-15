/*
	Provides a library to help command lines!
*/

#include <stdio.h>
#include <string.h>
#include "cmd_helpers.h"

/*
	Gets an argument from the commandline (not an option with -option or --long-option).
	Returns NULL is the idx is out of range, or an argument is not available.
*/
const char *FF_getarg(int argc, const char **argv, int argindex) {
	int i = 0;
	int argCount = 0;

	for(i = 0; i < argc; i++) {
		if(argv[i][0] != '-') {
			if(argCount == argindex) {
				return argv[i];
			}
			argCount++;
		}
	}

	return NULL;
}


/*
	Iterates through commandline Arguments.
	See the documentation for GNU's getopt() function. This has the same behaviour,
	except all information is provided in the ctx pointer, and not globally.
*/
int FF_getopt(int argc, const char **argv, const char *optstring, FF_GETOPT_CONTEXT *ctx) {
	int i;
	const char *pc;

	for(i = ctx->optind; i < argc; i++) {

		if(ctx->nextchar) {
			if(argv[i][ctx->nextchar + 1] == '\0') {
				i++;
				ctx->optind = i;
				ctx->nextchar = 0;
			}
		}

		if(i >= argc) {
			break;
		}

		if(argv[i][0] == '-' && !(argv[i][1] == '-' || argv[i][1] == '\0')) {
			// A single option argument was found, now process!

			pc = strchr(optstring, argv[i][ctx->nextchar + 1]);
			if(pc) {
				if(*(pc + 1) == ':') {
					if(ctx->nextchar) {	// This kind of option must come on its own
						ctx->optopt = argv[i][ctx->nextchar + 1];
						ctx->nextchar++;
						return '?';
					}
					// Option Argument
					if(argv[i][2]) {
						ctx->optarg = &argv[i][2];
						if(ctx->optarg[0] == '-') {
							ctx->optarg = NULL;
							ctx->nextchar = 0;
							ctx->optind = i+1;
							return ':';
						}
					} else {
						ctx->optarg = argv[i + 1];
						if((i+1) >= argc) {
							ctx->optarg = NULL;
							ctx->nextchar = 0;
							ctx->optind = i+1;
							return ':';
						}
					}
					ctx->optind = i+1;
					ctx->nextchar = 0;

					return argv[i][1];

				} else {
					ctx->optarg = NULL;
				}
				//ctx->optind = i+1;
				ctx->nextchar++;
				return argv[i][ctx->nextchar];
			} else {
				//ctx->optind += 1;
				ctx->optopt = argv[i][ctx->nextchar + 1];
				ctx->nextchar++;
				return '?';
			}
		}

		ctx->optind += 1;
	}

	//ctx->bDone 	= 1;
	ctx->optind = 0;

	return EOF;
}


/**
	Only process long options!
**/
int FF_getopt_long_only(int argc, const char **argv, const char *optstring, FF_GETOPTLONG_CONTEXT *ctx) {
	int i;
	struct option *pOptions = ctx->longopts;
	
	for(i = ctx->optind; i < argc; i++) {
		if(argv[i][0] == '-' && argv[i][1] == '-') {	// Identified a long option!
			// Does this long option match one in the options list?
			while(pOptions) {
				if(!strcmp(pOptions->name, &argv[i][2])) {	// Matches this item!
					ctx->longindex_val = (int) ((pOptions - ctx->longopts) / sizeof(struct option));
					if(ctx->longindex) {
						*ctx->longindex = (int) (pOptions - ctx->longopts);
					}

					// Check for parameters
					switch(pOptions->has_arg) {
						case 0:	// No argument
							ctx->optind = i;
							if(!pOptions->flag) {
								return pOptions->val;
							} else {
								*pOptions->flag = pOptions->val;
								return 0;
							}
							break;
						case 1:	// Compulsory argument
							if(argv[i][3] == '=') {
								ctx->optarg = &argv[i][4];							
							} else {
								ctx->optarg = argv[i + 1];
								ctx->optind = i+1;
							}
							break;
						case 2:	// Optional argument
							break;
					}


				} else {
					// Unrecognised Long option!
					ctx->optind = i;
					return '?';
				}

				pOptions++;
			}

			// Option unrecognised!
			return '?';
		}
	}

	return EOF;
}


/**
	As above, except we process long arguments as well!
**/
int FF_getopt_long(int argc, const char **argv, const char *optstring, FF_GETOPTLONG_CONTEXT *ctx) {
	int i;
	const char *pc;

	for(i = ctx->optind; i < argc; i++) {

		if(ctx->nextchar) {
			if(argv[i][ctx->nextchar + 1] == '\0') {
				i++;
				ctx->optind = i;
				ctx->nextchar = 0;
			}
		}

		if(i >= argc) {
			break;
		}

		if(argv[i][0] == '-' && !(argv[i][1] == '-' || argv[i][1] == '\0')) {
			// A single option argument was found, now process!

			pc = strchr(optstring, argv[i][ctx->nextchar + 1]);
			if(pc) {
				if(*(pc + 1) == ':') {
					if(ctx->nextchar) {	// This kind of option must come on its own
						ctx->optopt = argv[i][ctx->nextchar + 1];
						ctx->nextchar++;
						return '?';
					}
					// Option Argument
					if(argv[i][2]) {
						ctx->optarg = &argv[i][2];
						if(ctx->optarg[0] == '-') {
							ctx->optarg = NULL;
							ctx->nextchar = 0;
							ctx->optind = i+1;
							return ':';
						}
					} else {
						ctx->optarg = argv[i + 1];
						if((i+1) >= argc) {
							ctx->optarg = NULL;
							ctx->nextchar = 0;
							ctx->optind = i+1;
							return ':';
						}
					}
					ctx->optind = i+1;
					ctx->nextchar = 0;

					return argv[i][1];

				} else {
					ctx->optarg = NULL;
				}
				//ctx->optind = i+1;
				ctx->nextchar++;
				return argv[i][ctx->nextchar];
			} else {
				//ctx->optind += 1;
				ctx->optopt = argv[i][ctx->nextchar + 1];
				ctx->nextchar++;
				return '?';
			}
		}

		ctx->optind += 1;
	}

	//ctx->bDone 	= 1;
	ctx->optind = 0;

	return EOF;
}

int	append_filename(char *path, char *filename) {
	int i = strlen(path);

	while(path[i] != '\\' && path[i] != '/') {
		i--;
	}

	strcpy(&path[i+1], filename);

	return 0;
}
