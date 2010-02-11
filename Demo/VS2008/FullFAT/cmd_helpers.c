/*
	Provides a library to help command lines!
*/

#include <stdio.h>
#include <string.h>
#include "cmd_helpers.h"

const char *FF_getarg(int argc, const char **argv, int idx) {
	int i = 0;
	int argCount = 0;

	for(i = 0; i < argc; i++) {
		if(argv[i][0] != '-') {
			if(argCount == idx) {
				return argv[i];
			}
			argCount++;
		}
	}

	return NULL;
}

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

