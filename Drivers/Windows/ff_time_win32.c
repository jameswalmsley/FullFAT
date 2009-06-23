#include "../../src/ff_time.h"
#include <windows.h>

/**
 *	@public
 *	@brief	Populates an FF_SYSTEMTIME object with the current time from the system.
 *
 *	The developer must modify this function so that it is suitable for their platform.
 *	The function must return with 0, and if the time is not available all elements of the
 *	FF_SYSTEMTIME object must be zero'd, as in the examples provided.
 *
 *	@param	pTime	Pointer to an FF_TIME object.
 *
 *	@return	Always returns 0.
 **/
FF_T_SINT32	FF_GetSystemTime(FF_SYSTEMTIME *pTime) {
	
	SYSTEMTIME	str_t;

	GetLocalTime(&str_t);	
	
	pTime->Hour		= str_t.wHour;
	pTime->Minute	= str_t.wMinute;
	pTime->Second	= str_t.wSecond;
	pTime->Day		= str_t.wDay;
	pTime->Month	= str_t.wMonth;
	pTime->Year		= str_t.wYear;

	return 0;
}