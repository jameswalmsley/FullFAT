#ifndef _DIR_H_
#define _DIR_H_

typedef unsigned int	SD_ERROR;

#define SD_ATTRIBUTE_DIR		0x01
#define SD_ATTRIBUTE_RDONLY		0x02
#define SD_ATTRIBUTE_HIDDEN		0x04
#define SD_ATTRIBUTE_SYSTEM		0x08
#define SD_ATTRIBUTE_ARCHIVE	0x10

typedef struct {
	char	cDay;
	char	cMonth;
	int		iYear;
	char	cSecond;
	char	cMinute;
	char	cHour;
} SD_DATETIME;

typedef struct _SD_DIRENT {
	unsigned long		ulFileSize;
	unsigned long		ulAttributes;
	char 				*szFileName;
	struct _SD_DIRENT	*pNextEntry;

	SD_DATETIME			tmLastAccessed;
	SD_DATETIME			tmLastModified;
	SD_DATETIME			tmCreated;
} SD_DIRENT;

struct			_SD_DIR;
typedef struct 	_SD_DIR *SD_DIR;


SD_DIR SD_CreateDir		();
SD_ERROR SD_CleanupDir	(SD_DIR Dir);
SD_ERROR SD_AddDirent	(SD_DIR Dir, SD_DIRENT *pDirent);
SD_ERROR SD_FindNext	(SD_DIR Dir, SD_DIRENT *pFindData);
SD_ERROR SD_FindFirst	(SD_DIR Dir, SD_DIRENT *pFindData);
unsigned long SD_GetMaxFileName(SD_DIR Dir);
unsigned long SD_GetTotalItems(SD_DIR Dir);


#endif
