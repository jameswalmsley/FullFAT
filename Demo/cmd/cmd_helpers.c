/*
	Provides a library to help command lines!
*/

#include <stdio.h>
#include <string.h>
#include "cmd_helpers.h"
#include "../../../ffterm/src/ffterm.h"

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
	int i, swaps;
	const char *pc;
	const char *argvSwap;

	// Sort through the commandline arguments, and re-arrange them. So all options come first.
	do {
		swaps = 0;
		for(i = 1; i < argc; i++) {
			// Ensure we skip past option arguments!
			if(!swaps && argv[i][0] == '-') {
				pc = strchr(optstring, argv[i][1]);	// argv[i] is now the option string, lets see if it had a compulsory argument?
				if(pc) {
					if(*(pc + 1) == ':') {
						if(!argv[i][2] && (i + 1) < argc && argv[i + 1][0] != '-') { // If argv[i][3] is no \0 then do nothing! The param is connected.
							// Swap the next param's
							i++;	// Ensure we don't swap an -option back over this argument!
						}
					}
				}
			} else {
			
				if(argv[i][0] != '-' && ((i + 1) < argc)) {
					if(argv[i + 1][0] == '-') {
						// Swap Option!
						argvSwap = argv[i];
						argv[i] = argv[i + 1];
						argv[i + 1] = argvSwap;

						// Detect if option has an argument, this must be swapped too!
						pc = strchr(optstring, argv[i][1]);	// argv[i] is now the option string, lets see if it had a compulsory argument?
						if(pc) {
							if(*(pc + 1) == ':') {
								if(!argv[i][2] && (i + 2) < argc && argv[i + 2][0] != '-') { // If argv[i][3] is no \0 then do nothing! The param is connected.
									// Swap the next param's
									argvSwap = argv[i + 1];
									argv[i + 1] = argv[i + 2];
									argv[i + 2] = argvSwap;
									i++;	// Ensure we don't swap an -option back over this argument!
								}
							}
						}
						swaps++;

					}
				}
			}
		}
	} while(swaps);

	if(ctx->optind == 0) {
		ctx->optind = 1;	// argv[0] is of no interest!
	}
	
	for(i = ctx->optind; i < argc; i++) {

		if(ctx->nextchar) {
			if(argv[i][ctx->nextchar+1] == '\0') {
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
						ctx->optind = i+2; // Skip over this argument next time.
						ctx->nextchar = 0;
						return argv[i][1];
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
		} else if(argv[i][0] != '-') {
			// End of optional arguments, simply return EOF, with optind set to i;
			ctx->optind = i;
			return EOF;
		}

		ctx->optind += 1;
	}

	if(ctx->optind >= argc) {
		ctx->optind = 0;
	}

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

/**
 *	@public
 *	@brief	FullFAT Path Processor
 *
 *			Processes a path, and determines if it is relative or absolute.
 *			An absolute path is copied to dest as-is.
 *			A relative path is converted into an absolute path based on the working directory
 *			that is provided via the pEnv (FF_ENVIRONMENT object) pointer.
 *
 *	@param	dest	Where to write the processed string.
 *	@param	src		Path to be processed.
 *	@param	pEnv	Pointer to an FF_ENVIRONMENT object.
 *
 **/
void ProcessPath(char *dest, const char *src, FF_ENVIRONMENT *pEnv) {
	if(src[0] != '\\' && src[0] != '/') {
		if(strlen(pEnv->WorkingDir) == 1) {
			sprintf(dest, "\\%s", src);
		} else {
			sprintf(dest, "%s\\%s", pEnv->WorkingDir, src);
		}

	} else {
		sprintf(dest, "%s", src);
	}
}

/**
 *	@public
 *	@brief	A Standardised Dirent Print for FullFAT
 *
 *	Prints a Directory Entry, using a mix of styles found in Windows and Linux.
 *
 *	@param	pDirent		A pointer to an FF_DIRENT object as populated by the FF_FindFirst() or FF_FindNext() functions.
 **/
void FF_PrintDir(FF_DIRENT *pDirent) {
	unsigned char attr[5] = { '-','-','-','-', '\0' };	// String of Attribute Flags.
	if(pDirent->Attrib & FF_FAT_ATTR_READONLY)
		attr[0] = 'R';
	if(pDirent->Attrib & FF_FAT_ATTR_HIDDEN)
		attr[1] = 'H';
	if(pDirent->Attrib & FF_FAT_ATTR_SYSTEM)
		attr[2] = 'S';
	if(pDirent->Attrib & FF_FAT_ATTR_DIR)
		attr[3] = 'D';
#ifdef FF_TIME_SUPPORT	// Different Print formats dependent on if Time support is built-in.
	printf("%02d.%02d.%02d  %02d:%02d  %s  %12lu  %s\n", pDirent->CreateTime.Day, pDirent->CreateTime.Month, pDirent->CreateTime.Year, pDirent->CreateTime.Hour, pDirent->CreateTime.Minute, attr, pDirent->Filesize, pDirent->FileName);
#else
	printf(" %s %12lu    %s\n", attr, pDirent->Filesize, pDirent->FileName);
#endif
}

void SD_PrintDirent(SD_DIRENT *pDirent) {
	unsigned char attr[5] = { '-','-','-','-', '\0' };	// String of Attribute Flags.
	if(pDirent->ulAttributes & SD_ATTRIBUTE_RDONLY)
		attr[0] = 'R';
	if(pDirent->ulAttributes & SD_ATTRIBUTE_HIDDEN)
		attr[1] = 'H';
	if(pDirent->ulAttributes & SD_ATTRIBUTE_SYSTEM)
		attr[2] = 'S';
	if(pDirent->ulAttributes & SD_ATTRIBUTE_DIR) {
		attr[3] = 'D';
	}

	printf("%02d.%02d.%02d  %02d:%02d  %s  %12lu", pDirent->tmCreated.cDay, pDirent->tmCreated.cMonth, pDirent->tmCreated.iYear, pDirent->tmCreated.cHour, pDirent->tmCreated.cMinute, attr, pDirent->ulFileSize);
	
	if(pDirent->ulAttributes & SD_ATTRIBUTE_DIR) {
		FFTerm_SetConsoleColour(DIR_COLOUR | FFT_FOREGROUND_INTENSITY);
	}
	
	printf("  %s\n",  pDirent->szFileName);
	
	FFTerm_SetConsoleColour(FFT_FOREGROUND_GREY);
}

/*
	This routine removes all relative ..\ from a path.
	It's probably not the best, but it works. 

	Its based on some old code I wrote a long time ago.
*/
void ExpandPath(char *acPath) {

	char 	*pRel 		= 0;
	char	*pRelStart 	= 0;
	char 	*pRelEnd 	= 0;
	int		charRef 	= 0;
	int 	lenPath 	= 0;
	int		lenRel		= 0;
	int 	i 			= 0;
	int 	remain 		= 0;
	

	lenPath = strlen(acPath);
	pRel = strstr(acPath, "..");
	while(pRel) {	// Loop removal of Relativity
		charRef = pRel - acPath;

		/*
			We have found some relativity in the Path, 
		*/

		// Where it ends:
		
		if(pRel[2] == '\\' || pRel[2] == '/') {
			pRelEnd = pRel + 3;
		} else {
			pRelEnd = pRel + 2;	
		}
		
		// Where it Starts:
		
		if(charRef == 1) {	// Relative Path comes after the root /
			return;	// Fixed, returns false appropriately, as in the TODO: above!
		} else {
			for(i = (charRef - 2); i >= 0; i--) {
				if(acPath[i] == '\\' || acPath[i] == '/') {
					pRelStart = (acPath + (i + 1));
					break;
				}
			}
		}
		
		// The length of the relativity
		lenRel = pRelEnd - pRelStart;
		
		remain = lenPath - (pRelEnd - acPath);	// Remaining Chars on the end of the path
		
		if(lenRel) {
			strncpy(pRelStart, pRelEnd, remain);
			pRelStart[remain] = '\0';
		}
		
		lenPath -= lenRel;
		pRel = strstr(acPath, "..");
	}
}
