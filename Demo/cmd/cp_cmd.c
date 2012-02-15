/*****************************************************************************
 *  FullFAT - High Performance, Thread-Safe Embedded FAT File-System         *
 *                                                                           *
 *  Copyright(C) 2009  James Walmsley  (james@fullfat-fs.co.uk)              *
 *  Many Thanks to     Hein Tibosch    (hein_tibosch@yahoo.es)               *
 *                                                                           *
 *  See RESTRICTIONS.TXT for extra restrictions on the use of FullFAT.       *
 *                                                                           *
 *                FULLFAT IS NOT FREE FOR COMMERCIAL USE                     *
 *                                                                           *
 *  Removing this notice is illegal and will invalidate this license.        *
 *****************************************************************************
 *  See http://www.fullfat-fs.co.uk/ for more information.                   *
 *  Or  http://fullfat.googlecode.com/ for latest releases and the wiki.     *
 *****************************************************************************
 *  This program is free software: you can redistribute it and/or modify     *
 *  it under the terms of the GNU General Public License as published by     *
 *  the Free Software Foundation, either version 3 of the License, or        *
 *  (at your option) any later version.                                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public License        *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *                                                                           *
 *  IMPORTANT NOTICE:                                                        *
 *  =================                                                        *
 *  Alternative Licensing is available directly from the Copyright holder,   *
 *  (James Walmsley). For more information consult LICENSING.TXT to obtain   *
 *  a Commercial license.                                                    *
 *                                                                           *
 *****************************************************************************/
#include "cp_cmd.h"

/*
	This is a new implementation of a file copying command for FullFAT.
	
	It behaves similar to the GNU cp command.
*/
static FF_T_BOOL bExternal = FF_FALSE;

static int copy_dir	(const char *srcPath, const char *destPath, FF_T_BOOL bRecursive, FF_T_BOOL bVerbose, FF_ENVIRONMENT *pEnv);
static int copy_file(const char *szsrcPath, const char *szdestPath, FF_T_BOOL bVerbose, FF_ENVIRONMENT *pEnv);

int cp_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
#ifdef FF_UNICODE_SUPPORT
	const wchar_t			*szpSource, *szpDestination, *szpWildCard;
#else
	const char			*szpSource, *szpDestination, *szpWildCard;
#endif
	char				szsrcPath[FF_MAX_PATH], szdestPath[FF_MAX_PATH];
	FF_DIRENT			findData;
	
	FFT_GETOPT_CONTEXT	optionContext;	// CommandLine processing
	FF_T_BOOL			bRecursive = FF_FALSE, bVerbose = FF_FALSE;	// Option Flags.
	int 				option;

	memset(&optionContext, 0, sizeof(FFT_GETOPT_CONTEXT));			// Initialise the option context to zero.

	option = FFTerm_getopt(argc, (const char **) argv, "rRvx", &optionContext);		// Get the command line option charachters.

	while(option != EOF) {											// Process Commandline options
		switch(option) {
			case 'r':
			case 'R':
				bRecursive = FF_TRUE;								// Set recursive flag if -r or -R appears on the commandline.
				break;

			case 'v':
				bVerbose = FF_TRUE;									// Set verbose flag if -v appears on the commandline.
				break;

			case 'x':
				bExternal = FF_TRUE;
				break;

			default:
				break;
		}

		option = FFTerm_getopt(argc, (const char **) argv, "rRvx", &optionContext);	// Get the next option.
	}

	szpSource 		= FFTerm_getarg(argc, (const char **) argv, 0, &optionContext);	// The remaining options or non optional arguments.
	szpDestination 	= FFTerm_getarg(argc, (const char **) argv, 1, &optionContext);	// getarg() retrieves them intelligently.
	
	if(!szpSource) {
		printf("%s: No source file argument.\n", argv[0]);			// No source file provided.
		return 0;
	}

	if(!szpDestination) {
		printf("%s: No destination file argument.\n", argv[0]);		// No destination provided.
		return 0;
	}

	ProcessPath(szsrcPath, szpSource, pEnv);						// Process the paths into absolute paths.
	ProcessPath(szdestPath, szpDestination, pEnv);

	szpWildCard = GetWildcard(szpSource);							// Get the last token of the source path. (This may include a wildCard).

	if(strchr(szpWildCard, '*')) {									// If the 'WildCard' contains a * then its a wild card, otherwise its a file or directory.
		// WildCard Copying!
		//copy_wild();
		return 0;
	}

	if(FF_FindFirst(pEnv->pIoman, &findData, szsrcPath)) {			// Get the dirent for the file or directory, to detect if its a directory.
		// Not found!
		printf("%s: %s: no such file or directory.\n", argv[0], szpSource);
		return 0;
	}

	if(!strcmp(findData.FileName, szpWildCard) && (findData.Attrib & FF_FAT_ATTR_DIR)) {
		if(!bRecursive) {											// Its a dir!
			printf("%s: omitting directory '%s'\n", argv[0], szsrcPath);
			return 0;
		}
		copy_dir(szsrcPath, szdestPath, bRecursive, bVerbose, pEnv);// Start the copying!
		return 0;
	}

	copy_file(szsrcPath, szdestPath, bVerbose, pEnv);				// Final option, its simply a file to file copy
	
	return 0;
}
const FFT_ERR_TABLE cpInfo[] =
{
	{"Generic or Unknown Error",										-1},
	{"Copies the specified file to the specified location.",			FFT_COMMAND_DESCRIPTION},
	{ NULL }
};


static int copy_dir(const char *srcPath, const char *destPath, FF_T_BOOL bRecursive, FF_T_BOOL bVerbose, FF_ENVIRONMENT *pEnv) {
	FF_DIRENT	findData;
	FF_ERROR	RetVal;
	FF_T_INT8	szsrcFile[FF_MAX_PATH], szdestFile[FF_MAX_PATH];

	//const char	*szpWildCard;
	int i;

	strcpy(szsrcFile, srcPath);
	strcpy(szdestFile, destPath);

	// Ensure the paths both end in a '/' charachter
	i = strlen(szsrcFile);
	if(szsrcFile[i - 1] != '\\' || szsrcFile[i - 1] != '/') {
		strcat(szsrcFile, "\\");
	}
	i = strlen(szdestFile);
	if(szdestFile[i - 1] != '\\' || szdestFile[i - 1] != '/') {
		strcat(szdestFile, "\\");
	}

	RetVal = FF_FindFirst(pEnv->pIoman, &findData, szsrcFile);

	if(RetVal) {
		printf("cp: %s: No such file or directory.\n", srcPath);
		return 0;
	}

	while(!RetVal) {
		AppendFilename(szsrcFile, findData.FileName);
		AppendFilename(szdestFile, findData.FileName);

		if(!strcmp(szsrcFile, szdestFile)) {
			printf("cp: Source and Destination files are identical: illegal operation.\n");
			return 0;
		}

		if((findData.Attrib & FF_FAT_ATTR_DIR)) {
			if(!(findData.FileName[0] == '.' && !findData.FileName[1]) && !(findData.FileName[0] == '.' && findData.FileName[1] == '.' && !findData.FileName[2])) {
				// Add the wild card onto the end!
				if(bRecursive) {
					strcat(szsrcFile, "\\");
					//szpWildCard = getWildcard(srcPath);
					//strcat(szsrcFile, szpWildCard);
					
					strcat(szdestFile, "\\");
					//szpWildCard = getWildcard(destPath);
					//strcat(szdestFile, szpWildCard);
					

					// Make the dir if it doesn't already exist!
					copy_dir(szsrcFile, szdestFile, bRecursive, bVerbose, pEnv);

					strcpy(szsrcFile, srcPath);		// Reset the path.
				}
			}
		} else {
			copy_file(szsrcFile, szdestFile, bVerbose, pEnv);
		}

		RetVal = FF_FindNext(pEnv->pIoman, &findData);
	}

	return 0;
}


static int copy_file(const char *szsrcPath, const char *szdestPath, FF_T_BOOL bVerbose, FF_ENVIRONMENT *pEnv) {

	FF_FILE *pfSource;
	FF_FILE	*pfDestination = NULL;
	FILE *pex;
	FF_ERROR ffError;
	FF_T_SINT32	slBytesRead, slBytesWritten;
	unsigned char	buffer[CP_BUFFER_SIZE];

	if(!strcmp(szsrcPath, szdestPath)) {							// Ensure that source and destination are not the same file.
		printf("cp: Source and Destination files are identical: illegal operation.\n");
		return 0;
	}

	pfSource = FF_Open(pEnv->pIoman, szsrcPath, FF_MODE_READ, &ffError);	// Attempt to open the source.

	if(!pfSource) {
		printf("cp: %s: open failed: %s\n", szsrcPath, FF_GetErrMessage(ffError));	// Display a meaningful error message.
		return 0;
	}

	if(!bExternal) {
		pfDestination = FF_Open(pEnv->pIoman, szdestPath, (FF_MODE_WRITE | FF_MODE_CREATE | FF_MODE_TRUNCATE), &ffError);
	} else {
		pex = fopen(szdestPath+1, "w");
	}
	if(!pfDestination && !pex) {
		printf("cp: %s: open failed: %s\n", szdestPath, FF_GetErrMessage(ffError));
		FF_Close(pfSource);													// Don't forget to close the Source file.
		return 0;
	}

	// Source and Destination files are open, copy the data from Source to Dest!
	do {
		slBytesRead 	= FF_Read(pfSource, 1, CP_BUFFER_SIZE, buffer);
		if(!bExternal) {
			slBytesWritten 	= FF_Write(pfDestination, 1, slBytesRead, buffer);
		} else {
			slBytesWritten = fwrite(buffer, 1, slBytesRead, pex);
		}
		
		if(slBytesWritten != slBytesRead) {
			printf("cp: write error: %s\n", FF_GetErrMessage(slBytesWritten));
			break;
		}

	} while(slBytesRead);

	FF_Close(pfSource);
	if(!bExternal) {
		FF_Close(pfDestination);
	} else {
		fclose(pex);
	}

	if(bVerbose) {
		printf("'%s' -> '%s'\n", szsrcPath, szdestPath);
	}

	return 0;
}
