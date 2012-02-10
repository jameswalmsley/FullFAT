/*
	New versions of FF_Read() and FF_Write() that are optimised for unaligned accesses.
*/
#include "fullfat.h"


FF_T_SINT32 FF_Write2(FF_FILE *pFile, FF_T_UINT32 ElementSize, FF_T_UINT32 Count, FF_T_UINT8 *pBuffer) {
	FF_ERROR Error;

	if(!pFile) {
		return FF_ERR_NULL_POINTER | FF_WRITE;
	}

	Error = FF_CheckValid(pFile);
	if(Error) {
		return Error;
	}


}

