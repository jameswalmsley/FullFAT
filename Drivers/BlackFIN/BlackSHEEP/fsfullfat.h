#ifndef _FS_FULLFAT_H_
#define _FS_FULLFAT_H_

#include "S:\blackfin\VDK\BLACKSheep\common\fs\fsmgr.h"

T_FS_FUNCTIONS *fs_fullfat_getFileFunctions(void);

/*bool fs_fullfat_formatDevice( void *hMsd );
void* fs_fullfat_mountPartition(void *hMsd);
bool fs_fullfat_unmountPartition(void *partition);
unsigned long fs_fullfat_partitionSize(void *pPartition);
unsigned long fs_fullfat_partitionFree(void *pPartition);
void *fs_fullfat_fopen( void *pPartition, const char *fname, unsigned short modebits );
int fs_fullfat_fclose( void *stream );
int fs_fullfat_fdelete( void *pPartition, const char *filename );
bool fs_fullfat_fdirExists( void *partition, const char *dir );
bool fs_fullfat_fdirEmpty( void* partition, const char *dir );
int fs_fullfat_frmdir( void* partition, const char *dir );
int fs_fullfat_fmkdir( void* partition, const char *dir );
int fs_fullfat_feof( void *stream );
int fs_fullfat_fread( void *buffer, size_t size, size_t num, void *stream );
int fs_fullfat_fwrite( const void *buffer, size_t size, size_t count, void *stream );
int fs_fullfat_fputc( int ch, void *stream );
int fs_fullfat_fgetc( void *stream );
int fs_fullfat_fseek( void *stream, long offset, int origin );
int fs_fullfat_findFirst(void *partition, const char *pathname, T_FF_INFO *ffblk, int attrib);
int	fs_fullfat_findNext(void *partition, T_FF_INFO *ffblk);
int fs_fullfat_findClose(T_FF_INFO *ffblk);*/

#endif

