/*
	Provides a library to help command lines!
*/

#ifndef _CMD_HELPERS_
#define _CMD_HELPERS_
#include <stdio.h>
#include "../../src/fullfat.h"
#include "../../../ffterm/src/ffterm.h"
#include "dir.h"
#include "cmd_config.h"

#ifdef FF_UNICODE_SUPPORT
#include <wchar.h>
#endif

void PrintError(FF_ERROR Error);

#define DIR_COLOUR 			FFT_FOREGROUND_BLUE | FFT_FOREGROUND_GREEN | FFT_FOREGROUND_INTENSITY
#define COPY_BUFFER_SIZE 	8192

typedef struct {
	char		cDriveLetter;
	FF_IOMAN	*pIoman;		// Mounted partition pointer.
	FF_ERROR	Error;			// Error code for transactions on this mountpoint.
} FF_MOUNTPOINT;

typedef struct {							// Provides an environment for the FullFAT commands.
	 FF_IOMAN	*pIoman;
#ifdef FF_UNICODE_SUPPORT	
	 FF_T_WCHAR	WorkingDir[FF_MAX_PATH];	// A working directory Environment variable.
#else
	 FF_T_INT8	WorkingDir[FF_MAX_PATH];	// A working directory Environment variable.
#endif
	 FFT_CONSOLE	*pConsole;
} FF_ENVIRONMENT;

#ifdef FF_UNICODE_SUPPORT
int	wcsAppendFilename(wchar_t *path, wchar_t *filename);
void ProcessPath(wchar_t *dest, const wchar_t *src, FF_ENVIRONMENT *pEnv);
void wcsExpandPath(wchar_t *acPath);
const wchar_t *wcsGetWildcard(const wchar_t *String);
#else
void ProcessPath(char *dest, const char *src, FF_ENVIRONMENT *pEnv);
#endif

int	AppendFilename(char *path, char *filename);
void ExpandPath(char *acPath);
const char *GetWildcard(const char *String);


typedef enum {
	SD_BYTES,
	SD_KILOBYTES,
	SD_MEGABYTES,
	SD_GIGABYTES,
	SD_TERABYTES,
} SD_SIZEUNIT;

void SD_PrintDirent(SD_DIRENT *pDirent, SD_SIZEUNIT eUnit, char cForcedBytes, FF_ENVIRONMENT *pEnv);
void FF_PrintDir(FF_DIRENT *pDirent);



#ifndef WIN32
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

typedef struct {
	struct dirent 	dir;
	struct stat		itemInfo;
	char szWildCard[FF_MAX_PATH];
	char szItemPath[FF_MAX_PATH];
} DIRENT;

void ProcessLinuxPath(char *dest, const char *src);
DIR	*FindFirstFile(const char *szpPath, DIRENT *pFindData);
int	FindNextFile(DIR *pDir, DIRENT *pFindData);
int FindClose(DIR *pDir);

#endif

#endif
