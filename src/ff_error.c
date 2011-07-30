/*****************************************************************************
 *  FullFAT - High Performance, Thread-Safe Embedded FAT File-System         *
 *                                                                           *
 *  Copyright(C) 2009 James Walmsley  (james@fullfat-fs.co.uk)               *
 *  Copyright(C) 2010 Hein Tibosch    (hein_tibosch@yahoo.es)                *
 *                                                                           *
 *  See RESTRICTIONS.TXT for extra restrictions on the use of FullFAT.       *
 *                                                                           *
 *  Removing this notice is illegal and will invalidate this license.        *
 *****************************************************************************
 *  See http://www.fullfat-fs.co.uk/ for more information.                   *
 *  Or  http://fullfat.googlecode.com/ for latest releases and the wiki.     *
 *****************************************************************************
 * As of 19-July-2011 FullFAT has abandoned the GNU GPL License in favour of *
 * the more flexible Apache 2.0 license. See License.txt for full terms.     *
 *                                                                           *
 *            YOU ARE FREE TO USE FULLFAT IN COMMERCIAL PROJECTS             *
 *****************************************************************************/

/**
 *	@file		ff_error.c
 *	@author		James Walmsley
 *	@ingroup	ERROR
 *
 *	@defgroup	ERR Error Message
 *	@brief		Used to return pretty strings for FullFAT error codes.
 *
 **/
#include "ff_config.h"
#include "ff_types.h"
#include "ff_error.h"

//#include "logbuf.h"

#ifdef FF_DEBUG
const struct _FFMODULETAB
{
	const FF_T_INT8 * const strModuleName;
	const FF_T_UINT8		ucModuleID;
} gcpFullFATModuleTable[] =
{
	{"Unknown Module",		1},									// 1 here is ok, as the GetError functions start at the end of the table.
	{"ff_ioman.c",			FF_GETMODULE(FF_MODULE_IOMAN)},
	{"ff_dir.c",			FF_GETMODULE(FF_MODULE_DIR)},
	{"ff_file.c",			FF_GETMODULE(FF_MODULE_FILE)},
	{"ff_fat.c",			FF_GETMODULE(FF_MODULE_FAT)},
	{"ff_crc.c",			FF_GETMODULE(FF_MODULE_CRC)},
	{"ff_format.c",			FF_GETMODULE(FF_MODULE_FORMAT)},
	{"ff_hash.c",			FF_GETMODULE(FF_MODULE_HASH)},
	{"ff_memory.c",			FF_GETMODULE(FF_MODULE_MEMORY)},
	{"ff_string.c",			FF_GETMODULE(FF_MODULE_STRING)},
	{"ff_unicode.c",		FF_GETMODULE(FF_MODULE_UNICODE)},
	{"ff_safety.c",			FF_GETMODULE(FF_MODULE_SAFETY)},
	{"ff_time.c",			FF_GETMODULE(FF_MODULE_TIME)},
	{"Platform Driver",		FF_GETMODULE(FF_MODULE_DRIVER)},
};

const struct _FFFUNCTIONTAB
{
	const FF_T_INT8 * const strFunctionName;
	const FF_T_UINT8		ucFunctionID;
} gcpFullFATFunctionTable[] =
{
	{"Unknown Function",	1},
	// IO Man Functions
	{"FF_CreateIOMAN",		FF_GETFUNCTION(FF_CREATEIOMAN)},
	{"FF_DestroyIOMAN",		FF_GETFUNCTION(FF_DESTROYIOMAN)},
};

const struct _FFERRTAB
{
    const FF_T_INT8 * const strErrorString;
    const FF_T_UINT8		ucErrorCode;		// Currently there are less then 256 errors, so lets keep this table small.
} gcpFullFATErrorTable[] =
{
	{"Unknown or Generic Error! - Please contact james@fullfat-fs.co.uk",			1},
	{"No Error",																	FF_ERR_NONE},
	{"Null Pointer provided, (probably for IOMAN)",									FF_ERR_NULL_POINTER},
    {"Not enough memory (malloc() returned NULL)",									FF_ERR_NOT_ENOUGH_MEMORY},
    {"Device Driver returned a FATAL error!",										FF_ERR_DEVICE_DRIVER_FAILED},
    {"The blocksize is not 512 multiple",											FF_ERR_IOMAN_BAD_BLKSIZE},
    {"The memory size, is not a multiple of the blocksize. (Atleast 2 Blocks)",		FF_ERR_IOMAN_BAD_MEMSIZE},
    {"Device is already registered, use FF_UnregisterBlkDevice() first",			FF_ERR_IOMAN_DEV_ALREADY_REGD},
    {"No mountable partition was found on the specified device",					FF_ERR_IOMAN_NO_MOUNTABLE_PARTITION},
    {"The format of the MBR was unrecognised",										FF_ERR_IOMAN_INVALID_FORMAT},
    {"The provided partition number is out-of-range (0 - 3)",						FF_ERR_IOMAN_INVALID_PARTITION_NUM},
    {"The selected partition / volume doesn't appear to be FAT formatted",			FF_ERR_IOMAN_NOT_FAT_FORMATTED},
    {"Cannot register device. (BlkSize not a multiple of 512)",						FF_ERR_IOMAN_DEV_INVALID_BLKSIZE},
    {"Cannot unregister device, a partition is still mounted",						FF_ERR_IOMAN_PARTITION_MOUNTED},
    {"Cannot unmount the partition while there are active FILE handles",			FF_ERR_IOMAN_ACTIVE_HANDLES},
	{"The GPT partition header appears to be corrupt, refusing to mount",			FF_ERR_IOMAN_GPT_HEADER_CORRUPT},
	{"Not enough free disk space to complete the disk transaction",					FF_ERR_IOMAN_NOT_ENOUGH_FREE_SPACE},
	{"Attempted to Read a sector out of bounds",									FF_ERR_IOMAN_OUT_OF_BOUNDS_READ},
	{"Attempted to Write a sector out of bounds",									FF_ERR_IOMAN_OUT_OF_BOUNDS_WRITE},

    {"Cannot open the file, file already in use",									FF_ERR_FILE_ALREADY_OPEN},
    {"The specified file could not be found",										FF_ERR_FILE_NOT_FOUND},
    {"Cannot open a Directory",														FF_ERR_FILE_OBJECT_IS_A_DIR},
	{"Cannot open for writing: File is marked as Read-Only",						FF_ERR_FILE_IS_READ_ONLY},
    {"Path not found",																FF_ERR_FILE_INVALID_PATH},
	{"File operation failed - the file was not opened for writing",					FF_ERR_FILE_NOT_OPENED_IN_WRITE_MODE},
	{"File operation failed - the file was not opened for reading",					FF_ERR_FILE_NOT_OPENED_IN_READ_MODE},
	{"File operation failed - could not extend file",								FF_ERR_FILE_EXTEND_FAILED},
	{"Destination file already exists",												FF_ERR_FILE_DESTINATION_EXISTS},
	{"Source file was not found",													FF_ERR_FILE_SOURCE_NOT_FOUND},
	{"Destination path (dir) was not found",										FF_ERR_FILE_DIR_NOT_FOUND},
	{"Failed to create the directory Entry",										FF_ERR_FILE_COULD_NOT_CREATE_DIRENT},
	{"A file handle was invalid",													FF_ERR_FILE_BAD_HANDLE},
#ifdef FF_REMOVABLE_MEDIA
	{"File handle got invalid because media was removed",							FF_ERR_FILE_MEDIA_REMOVED},
#endif
    {"A file or folder of the same name already exists",							FF_ERR_DIR_OBJECT_EXISTS},
    {"FF_ERR_DIR_DIRECTORY_FULL",													FF_ERR_DIR_DIRECTORY_FULL},
    {"FF_ERR_DIR_END_OF_DIR",														FF_ERR_DIR_END_OF_DIR},
    {"The directory is not empty",													FF_ERR_DIR_NOT_EMPTY},
	{"Could not extend File or Folder - No Free Space!",							FF_ERR_FAT_NO_FREE_CLUSTERS},
	{"Could not find the directory specified by the path",							FF_ERR_DIR_INVALID_PATH},
	{"The Root Dir is full, and cannot be extended on Fat12 or 16 volumes",			FF_ERR_DIR_CANT_EXTEND_ROOT_DIR},
	{"Not enough space to extend the directory.",									FF_ERR_DIR_EXTEND_FAILED},
	{"Name exceeds the number of allowed charachters for a filename",				FF_ERR_DIR_NAME_TOO_LONG},

#ifdef FF_UNICODE_SUPPORT
	{"An invalid Unicode charachter was provided!",									FF_ERR_UNICODE_INVALID_CODE},
	{"Not enough space in the UTF-16 buffer to encode the entire sequence",			FF_ERR_UNICODE_DEST_TOO_SMALL},
	{"An invalid UTF-16 sequence was encountered",									FF_ERR_UNICODE_INVALID_SEQUENCE},
	{"Filename exceeds MAX long-filename length when converted to UTF-16",			FF_ERR_UNICODE_CONVERSION_EXCEEDED},
#endif
};

/**
 *	@public
 *	@brief	Returns a pointer to a string relating to a FullFAT error code.
 *	
 *	@param	iErrorCode	The error code.
 *
 *	@return	Pointer to a string describing the error.
 *
 **/
const FF_T_INT8 *FF_GetErrMessage(FF_ERROR iErrorCode) {
    FF_T_UINT32 stCount = sizeof (gcpFullFATErrorTable) / sizeof ( struct _FFERRTAB);
    while (stCount--){
        if (((FF_ERROR) gcpFullFATErrorTable[stCount].ucErrorCode) == FF_GETERROR(iErrorCode)) {
            return gcpFullFATErrorTable[stCount].strErrorString;
        }
    }
	return gcpFullFATErrorTable[0].strErrorString;
}

const FF_T_INT8 *FF_GetErrModule(FF_ERROR iErrorCode) {
	FF_T_UINT32 stCount = sizeof (gcpFullFATModuleTable) / sizeof (struct _FFMODULETAB);
	while (stCount--) {
		if(gcpFullFATModuleTable[stCount].ucModuleID == FF_GETMODULE(iErrorCode)) {
			return gcpFullFATModuleTable[stCount].strModuleName;
		}
	}
	return gcpFullFATModuleTable[0].strModuleName;
}

const FF_T_INT8 *FF_GetErrFunction(FF_ERROR iErrorCode) {
	FF_T_UINT32 stCount = sizeof (gcpFullFATFunctionTable) / sizeof (struct _FFFUNCTIONTAB);
	while (stCount--) {
		if(gcpFullFATFunctionTable[stCount].ucFunctionID == FF_GETFUNCTION(iErrorCode)) {
			return gcpFullFATFunctionTable[stCount].strFunctionName;
		}
	}
	return gcpFullFATFunctionTable[0].strFunctionName;
}


#endif
