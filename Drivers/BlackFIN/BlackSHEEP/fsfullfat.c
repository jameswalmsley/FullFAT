#include "fsfullfat.h"
#include "../../../src/fullfat.h"


// local includes
//#include "fat/fat.h"

#ifdef _USE_VDK_
#include <services/services.h>
#endif
#pragma align 32
#pragma section("L1_data")
unsigned char g_Cache[4096];



unsigned long FF_blackfin_blk_read(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam) {

	return	msd_blockRead((T_MSD_HANDLE) pParam, sector, sectors, buffer) ;
}


unsigned long FF_blackfin_blk_write(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam) {

	return	msd_blockWrite((T_MSD_HANDLE) pParam, sector, sectors, buffer) ;
}



bool fs_fullfat_formatDevice( void *hMsd ) {
	return (fat_format( (unsigned long)hMsd ) >= 0);
}

void* fs_fullfat_mountPartition(void *hMsd) {
	FF_IOMAN *pIoman;
	int result;	
	
	//unsigned char *cache = /*(unsigned char *) &g_Cache;*/malloc(4096);
	
	pIoman = FF_CreateIOMAN(NULL, 4096, 512, NULL);
	if(pIoman) {
		
		FF_RegisterBlkDevice(pIoman, 512, (FF_WRITE_BLOCKS) FF_blackfin_blk_write, (FF_READ_BLOCKS) &FF_blackfin_blk_read, hMsd);
		if(FF_MountPartition(pIoman, 0)) {
			FF_DestroyIOMAN(pIoman);
			pIoman = NULL;
		}
	}
		
	return pIoman;
}

bool fs_fullfat_unmountPartition(void *partition) {
	return 0;
}

unsigned long fs_fullfat_partitionSize(void *pPartition) {
	return (FF_T_UINT32) FF_GetVolumeSize((FF_IOMAN*) pPartition);
} 

unsigned long fs_fullfat_partitionFree(void *pPartition) {
	return (FF_T_UINT32) FF_GetFreeSize((FF_IOMAN*) pPartition);
}

bool fs_fullfat_fdirExists( void *partition, const char *dir ) {
	
	if(FF_FindDir((FF_IOMAN *) partition, (FF_T_INT8 *) dir, strlen(dir))  > 0) {
		return true; 
	}
	
	return false;
}

bool fs_fullfat_fdirEmpty( void* partition, const char *dir ) {
	return true;
}

int fs_fullfat_fmkdir( void* partition, const char *dir ) {
	return FF_MkDir((FF_IOMAN *) partition, (char *) dir);
}

int fs_fullfat_frmdir( void* partition, const char *dir ) {
	return FF_RmDir((FF_IOMAN *) partition, (char *) dir);
}

int fs_fullfat_fdelete( void *pPartition, const char *filename ) {
	return 0;
}

void *fs_fullfat_fopen( void *pPartition, const char *fname, unsigned short modebits ) {
	
	FF_FILE *file;
	int i,x;
	
	char Filename[260];
	char Path[260];
	
	unsigned char nOpenMode = 0;
	// evaluate the mode bits	
	if (modebits & FS_O_BIT_RD) {
		if (modebits & FS_O_BIT_WR) {
			nOpenMode |= FF_MODE_READ;
			nOpenMode |= FF_MODE_WRITE;
		} else {
			nOpenMode |= FF_MODE_READ;
		}
	} else {
		if (modebits & FS_O_BIT_WR) {
			nOpenMode |= FF_MODE_WRITE;
		}
	}
	if (modebits & FS_O_BIT_APP) {
		nOpenMode |= FF_MODE_APPEND;
		if (nOpenMode & FS_O_BIT_RD) {
			nOpenMode |= FF_MODE_WRITE;
			nOpenMode |= FF_MODE_READ;			
		} else {
			nOpenMode |= FF_MODE_WRITE;
		}
	}
	
	sprintf(Filename, "%s", fname);
	
	file = FF_Open((FF_IOMAN*)pPartition, Filename, nOpenMode, NULL);
	
	return file;
}

int fs_fullfat_fclose( void *stream ) {

	return FF_Close((FF_FILE *) stream);
	
}

int fs_fullfat_feof( void *stream ) {
	return FF_isEOF((FF_FILE*)stream);
}

int fs_fullfat_fread( void *buffer, size_t size, size_t count, void *stream ) {	 
	return FF_Read((FF_FILE *) stream, size, count, buffer);
	/*int i;
	unsigned char *value = (unsigned char *) buffer;
	
	for(i = 0; i < (size * count); i++) {
		if(FF_isEOF((FF_FILE *) stream)) {
			break;	
		}
		*(value++) = (unsigned char) FF_GetC((FF_FILE *) stream);	
	}
	
	return i;*/
}

int fs_fullfat_fwrite( const void *buffer, size_t size, size_t count, void *stream ) {
	return FF_Write((FF_FILE *) stream, size, count, (FF_T_UINT8 *) buffer);
	/*int i;
	unsigned char *value = (unsigned char *) buffer;
	
	for(i = 0; i < (size * count); i++) {
		FF_PutC((FF_FILE *) stream, *(value++));	
	}
	
	return i;*/
}

int fs_fullfat_fputc( int ch, void *stream ) {
		FF_PutC((FF_FILE *) stream, (FF_T_UINT8) ch);
		return ch;
}

int fs_fullfat_fgetc( void *stream ) {	
	int mychar = FF_GetC((FF_FILE*) stream);
		
	if(mychar < 0) {
		return EOF;	
	}
	return mychar;	
}

int fs_fullfat_fseek( void *stream, long offset, int origin ) {
	int nFatOrigin;
	switch(origin) {
		case SEEK_SET:
			nFatOrigin = FF_SEEK_SET;
			break;
		case SEEK_CUR:
			nFatOrigin = FF_SEEK_CUR;
			break;
		case SEEK_END:
			nFatOrigin = FF_SEEK_END;
			break;
		default:
			nFatOrigin = origin;
	}
	return FF_Seek((FF_FILE *) stream, offset, nFatOrigin);
}


int fs_fullfat_findFirst(void *partition, const char *pathname, T_FF_INFO *ffblk, int attrib) {
		FF_DIRENT *mydir = malloc(sizeof(FF_DIRENT));
		FF_IOMAN *pIoman = (FF_IOMAN *)partition;
		int Result = 0;
		char mypath[1024];
		char *nPos;
	
		strcpy(mypath, pathname);
	
		nPos = strstr(mypath, "*.*");
		
		if(nPos) {
			nPos[0] = '\0';
		}
		
		if(strlen(mypath) > 1) {
			if(mypath[strlen(mypath) -1] == '\\') {
				mypath[strlen(mypath) - 1] = '\0';	
			}
		}
		
		Result = FF_FindFirst(pIoman, mydir, (FF_T_INT8*) mypath);
		
		if(Result == 0) {
			ffblk->pParam = (void *) mydir;
			
			ffblk->stCrDate.nHour 	= mydir->CreateTime.Hour;
			ffblk->stCrDate.nMinute = mydir->CreateTime.Minute;
			ffblk->stCrDate.nSecond = mydir->CreateTime.Second;
			ffblk->stCrDate.nDay 	= mydir->CreateTime.Day;
			ffblk->stCrDate.nMonth 	= mydir->CreateTime.Month;
			ffblk->stCrDate.nYear 	= mydir->CreateTime.Year;
			
			ffblk->stMoDate.nHour 	= mydir->ModifiedTime.Hour;
			ffblk->stMoDate.nMinute = mydir->ModifiedTime.Minute;
			ffblk->stMoDate.nSecond = mydir->ModifiedTime.Second;
			ffblk->stMoDate.nDay 	= mydir->ModifiedTime.Day;
			ffblk->stMoDate.nMonth 	= mydir->ModifiedTime.Month;
			ffblk->stMoDate.nYear 	= mydir->ModifiedTime.Year;
			
			ffblk->stAcDate.nHour 	= mydir->AccessedTime.Hour;
			ffblk->stAcDate.nMinute = mydir->AccessedTime.Minute;
			ffblk->stAcDate.nSecond = mydir->AccessedTime.Second;
			ffblk->stAcDate.nDay 	= mydir->AccessedTime.Day;
			ffblk->stAcDate.nMonth 	= mydir->AccessedTime.Month;
			ffblk->stAcDate.nYear 	= mydir->AccessedTime.Year;
			
			strcpy(ffblk->acName, mydir->FileName);
			ffblk->nSize = mydir->Filesize;
			
			if(mydir->Attrib == 0x10) {
				ffblk->nAttr = FS_A_DIR;
			}
			else 
				ffblk->nAttr = 0;
			return 0;
		} else {
			//free(mydir);	
		}
		
		return -1;
}

int	fs_fullfat_findNext(void *partition, T_FF_INFO *ffblk) {
		
		FF_DIRENT *mydir = (FF_DIRENT *) ffblk->pParam;
		FF_IOMAN *pIoman = (FF_IOMAN *)partition;
		int result;
		
		result = FF_FindNext(pIoman, mydir);
		
		if(result == 0) {
			strcpy(ffblk->acName, mydir->FileName);
			ffblk->nSize = mydir->Filesize;
			
			ffblk->stCrDate.nHour 	= mydir->CreateTime.Hour;
			ffblk->stCrDate.nMinute = mydir->CreateTime.Minute;
			ffblk->stCrDate.nSecond = mydir->CreateTime.Second;
			ffblk->stCrDate.nDay 	= mydir->CreateTime.Day;
			ffblk->stCrDate.nMonth 	= mydir->CreateTime.Month;
			ffblk->stCrDate.nYear 	= mydir->CreateTime.Year;
			
			ffblk->stMoDate.nHour 	= mydir->ModifiedTime.Hour;
			ffblk->stMoDate.nMinute = mydir->ModifiedTime.Minute;
			ffblk->stMoDate.nSecond = mydir->ModifiedTime.Second;
			ffblk->stMoDate.nDay 	= mydir->ModifiedTime.Day;
			ffblk->stMoDate.nMonth 	= mydir->ModifiedTime.Month;
			ffblk->stMoDate.nYear 	= mydir->ModifiedTime.Year;
			
			ffblk->stAcDate.nHour 	= mydir->AccessedTime.Hour;
			ffblk->stAcDate.nMinute = mydir->AccessedTime.Minute;
			ffblk->stAcDate.nSecond = mydir->AccessedTime.Second;
			ffblk->stAcDate.nDay 	= mydir->AccessedTime.Day;
			ffblk->stAcDate.nMonth 	= mydir->AccessedTime.Month;
			ffblk->stAcDate.nYear 	= mydir->AccessedTime.Year;
			
			if(mydir->Attrib == 0x10) {
				ffblk->nAttr = FS_A_DIR;
			}
			else 
				ffblk->nAttr = 0;
			return 0;
		}
		
		return result;		
	
}

long fs_fullfat_ftell(FF_FILE *pFile) {
	return (long) FF_Tell(pFile);	
}

int fs_fullfat_findClose(T_FF_INFO *ffblk) {
//	free(ffblk->pParam);
	return 0;
}

// ----------------------------------------------------------------------------

static T_FS_FUNCTIONS g_fullfatFatFunctions = {
	fs_fullfat_formatDevice,
	fs_fullfat_mountPartition,
	fs_fullfat_unmountPartition,
	fs_fullfat_partitionSize,
	fs_fullfat_partitionFree,
	fs_fullfat_fdelete,
	fs_fullfat_fdirExists,
	fs_fullfat_fdirEmpty,
	fs_fullfat_frmdir,
	fs_fullfat_fmkdir,
	fs_fullfat_fopen,
	fs_fullfat_fclose,
	fs_fullfat_feof,
	fs_fullfat_fread,
	fs_fullfat_fwrite,
	fs_fullfat_fputc,
	fs_fullfat_fgetc,
	fs_fullfat_fseek,
	(FS_FN_TELL)fs_fullfat_ftell,		
	fs_fullfat_findFirst,
	fs_fullfat_findNext,		
	fs_fullfat_findClose
};

T_FS_FUNCTIONS *fs_fullfat_getFileFunctions(void) {
	return &g_fullfatFatFunctions;
}
