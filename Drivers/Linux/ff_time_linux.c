#include "../../src/ff_time.h"
#include <time.h>

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
	
	time_t result;
	struct tm str_t;

	result = time(NULL);
	localtime_r(&result, &str_t);
	
	pTime->Hour		= str_t.tm_hour;
	pTime->Minute	= str_t.tm_min;
	pTime->Second	= str_t.tm_sec;
	pTime->Day		= str_t.tm_mday;
	pTime->Month	= str_t.tm_mon + 1;
	pTime->Year		= str_t.tm_year + 1900;

	return 0;
}