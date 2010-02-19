/*
	Provides a library to help command lines!
*/

#ifndef _CMD_HELPERS_
#define _CMD_HELPERS_

#include "../../src/fullfat.h"
#include "dir.h"

#define DIR_COLOUR 			FFT_FOREGROUND_BLUE | FFT_FOREGROUND_GREEN | FFT_FOREGROUND_INTENSITY
#define COPY_BUFFER_SIZE 	8192



typedef struct {							// Provides an environment for the FullFAT commands.
	FF_IOMAN	*pIoman;
	FF_T_INT8	WorkingDir[FF_MAX_PATH];	// A working directory Environment variable.
} FF_ENVIRONMENT;

int	append_filename(char *path, char *filename);
const char *getWildcard(const char *String);
void ProcessPath(char *dest, const char *src, FF_ENVIRONMENT *pEnv);

void SD_PrintDirent(SD_DIRENT *pDirent);
void FF_PrintDir(FF_DIRENT *pDirent);

void ExpandPath(char *acPath);

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

typedef struct {
	struct dirent 	dir;
	struct stat		itemInfo;
	char szWildCard[PATH_MAX];
	char szItemPath[PATH_MAX];
} DIRENT;

DIR	*FindFirstFile(const char *szpPath, DIRENT *pFindData);
int	FindNextFile(DIR *pDir, DIRENT *pFindData);
int FindClose(DIR *pDir);

#endif

#endif
