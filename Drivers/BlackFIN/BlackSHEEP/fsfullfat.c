/********************************************************************
 			FullFAT File-System Integration for BlackSHEEP
						by James Walmsley

	This file handles the seamless integration of FullFAT in to the
	file-system manager provided by BlackSHEEP. This file serves as
	a good example of a complete and seamless FullFAT integration.

 ********************************************************************/


#include "fsfullfat.h"
#include "../../../src/fullfat.h"			// FullFAT types, objects and prototypes.



/**
 *	FullFAT Integration starts with Block-device driver integration.
 *
 *	This is simple with BlackSHEEP simply wrap the msd manager interfaces.
 **/
static unsigned long FF_blackfin_blk_read(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam) {

	return	msd_blockRead((T_MSD_HANDLE) pParam, sector, sectors, buffer) ;
}


static unsigned long FF_blackfin_blk_write(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam) {

	return	msd_blockWrite((T_MSD_HANDLE) pParam, sector, sectors, buffer) ;
}


/**
 *	Everything in this file is defined as a static. None of these functions
 *	should ever expose an interface.
 *
 **/

 
/**
 *	This simply wraps the BlackSHEEP Fat formatting tool.
 *
 *	FullFAT also includes a device format interface, but it isn't complete
 *	as of 0.99.
 *
 **/
static bool fs_fullfat_formatDevice( void *hMsd ) {
	return (fat_format( (unsigned long)hMsd ) >= 0);
}



/**
 *	This is a good example of an integrated Mounting function.
 *
 *
 *	Here FullFAT malloc()'s its own working memory.
 *
 **/
static void* fs_fullfat_mountPartition(void *hMsd) {
	FF_IOMAN 		*pIoman;				// FF_IOMAN Object to be created.
	FF_ERROR		Error = FF_ERR_NONE;	// FullFAT Error Code
	T_DEVICE_INFO	DevInfo;				// We need to know the Blocksize of the underlying device.
	
	msd_info((T_MSD_HANDLE) hMsd, &DevInfo);
	
	pIoman = FF_CreateIOMAN(NULL, 4096, DevInfo.nBlockSize, &Error);	// Create an IOMAN with 4KB of working buffers.
	if(pIoman) {
		// Register BlackSHEEP's Block Device read and write functions.
		Error = FF_RegisterBlkDevice(pIoman, DevInfo.nBlockSize, (FF_WRITE_BLOCKS) FF_blackfin_blk_write, (FF_READ_BLOCKS) &FF_blackfin_blk_read, hMsd);
		
		if(Error) {
			FF_DestroyIOMAN(pIoman);
			pIoman = NULL;	
		}
		
		if(FF_MountPartition(pIoman, 0)) {
			FF_DestroyIOMAN(pIoman);
			pIoman = NULL;
		}
	}
		
	return pIoman;	// No error reporting, just return pIoman. Will be Null if there was an error!
}

static bool fs_fullfat_unmountPartition(void *pPartition) {
	if(FF_UnmountPartition((FF_IOMAN*) pPartition)) {	// Try to Unmount
		return 0;										// Mount failed.
	}
	
	// Success, destroy the IOMAN object.
	FF_DestroyIOMAN((FF_IOMAN*) pPartition);
	
	return 1;
}

static unsigned long fs_fullfat_partitionSize(void *pPartition) {
	return (FF_T_UINT32) FF_GetVolumeSize((FF_IOMAN*) pPartition);
} 

static unsigned long fs_fullfat_partitionFree(void *pPartition) {
	return (FF_T_UINT32) FF_GetFreeSize((FF_IOMAN*) pPartition);
}

static bool fs_fullfat_fdirExists( void *pPartition, const char *dir ) {
	
	if(FF_FindDir((FF_IOMAN *) pPartition, (FF_T_INT8 *) dir, strlen(dir))  > 0) {
		return true; 
	}
	
	return false;
}

static bool fs_fullfat_fdirEmpty( void* partition, const char *dir ) {
	if(FF_isDirEmpty((FF_IOMAN *) partition, dir)) {
		return true;
	}
	
	return false;
}

static int fs_fullfat_fmkdir( void* partition, const char *dir ) {
	return FF_MkDir((FF_IOMAN *) partition, (char *) dir);
}

static int fs_fullfat_frmdir( void* partition, const char *dir ) {
	return FF_RmDir((FF_IOMAN *) partition, (char *) dir);
}

static int fs_fullfat_fdelete( void *pPartition, const char *filename ) {
	return FF_RmFile((FF_IOMAN *) pPartition, (char *) filename);
}

static void *fs_fullfat_fopen( void *pPartition, const char *fname, unsigned short modebits ) {
	
	FF_FILE *file;
	
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
	
	if(modebits & FS_O_BIT_CR) {
		nOpenMode |= FF_MODE_CREATE;	
	}
	
	file = FF_Open((FF_IOMAN*)pPartition, fname, nOpenMode, NULL);
	
	return file;
}

static int fs_fullfat_fclose( void *stream ) {

	return FF_Close((FF_FILE *) stream);
	
}

static int fs_fullfat_feof( void *stream ) {
	return FF_isEOF((FF_FILE*)stream);
}

static int fs_fullfat_fread( void *buffer, size_t size, size_t count, void *stream ) {	 
	return FF_Read((FF_FILE *) stream, size, count, buffer);
}

static int fs_fullfat_fwrite( const void *buffer, size_t size, size_t count, void *stream ) {
	return FF_Write((FF_FILE *) stream, size, count, (FF_T_UINT8 *) buffer);
}

static int fs_fullfat_fputc( int ch, void *stream ) {
	return FF_PutC((FF_FILE *) stream, (FF_T_UINT8) ch);
}

static int fs_fullfat_fgetc( void *stream ) {	
	int mychar = FF_GetC((FF_FILE*) stream);
		
	if(mychar < 0) {
		return EOF;	
	}
	return mychar;	
}

static int fs_fullfat_fseek( void *stream, long offset, int origin ) {
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


static int fs_fullfat_findFirst(void *pPartition, const char *pathname, T_FF_INFO *ffblk, int attrib) {
		FF_DIRENT *mydir = malloc(sizeof(FF_DIRENT));	// Free'd in findclose
		FF_IOMAN *pIoman = (FF_IOMAN *)pPartition;
		int Result = 0;
		int i;
		int pathLen;
		char mypath[2600];
		char *nPos;
		unsigned long DirCluster;
	
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
		
		pathLen = strlen(pathname);
		
		if(!nPos) {
			for(i = pathLen; i > 0; i--) {
				if(pathname[i] == '\\' || pathname[i] == '/') {
					break;	
				}
			}
			
			DirCluster = FF_FindDir(pIoman, pathname, i);
				
			if(DirCluster) {
				//FF_T_UINT32 FF_FindEntryInDir(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_T_INT8 *name, FF_T_UINT8 pa_Attrib, FF_DIRENT *pDirent)
				if(FF_FindEntryInDir(pIoman, DirCluster, (FF_T_INT8 *)(pathname + i + 1), 0, mydir)) {
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
					return -1;	
				}
										
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
		}
		
		return -1;
}

static int fs_fullfat_findNext(void *pPartition, T_FF_INFO *ffblk) {
		
		FF_DIRENT *mydir = (FF_DIRENT *) ffblk->pParam;
		FF_IOMAN *pIoman = (FF_IOMAN *)pPartition;
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
			
			if(mydir->Attrib == FF_FAT_ATTR_DIR) {
				ffblk->nAttr = FS_A_DIR;
			}
			else 
				ffblk->nAttr = 0;
			return 0;
		}
		
		return result;		
	
}

static long fs_fullfat_ftell(FF_FILE *pFile) {
	return (long) FF_Tell(pFile);	
}

static int fs_fullfat_findClose(T_FF_INFO *ffblk) {
	free(ffblk->pParam);
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
