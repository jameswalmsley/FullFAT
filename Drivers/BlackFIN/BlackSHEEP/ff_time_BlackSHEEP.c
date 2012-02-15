/*****************************************************************************
 *  FullFAT - High Performance, Thread-Safe Embedded FAT File-System         *
 *  Copyright (C) 2009  James Walmsley (james@worm.me.uk)                    *
 *                                                                           *
 *  This program is free software: you can redistribute it and/or modify     *
 *  it under the terms of the GNU General Public License as published by     *
 *  the Free Software Foundation, either version 3 of the License, or        *
 *  (at your option) any later version.                                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public License        *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *                                                                           *
 *  IMPORTANT NOTICE:                                                        *
 *  =================                                                        *
 *  Alternative Licensing is available directly from the Copyright holder,   *
 *  (James Walmsley). For more information consult LICENSING.TXT to obtain   *
 *  a Commercial license.                                                    *
 *                                                                           *
 *  See RESTRICTIONS.TXT for extra restrictions on the use of FullFAT.       *
 *                                                                           *
 *  Removing the above notice is illegal and will invalidate this license.   *
 *****************************************************************************
 *  See http://worm.me.uk/fullfat for more information.                      *
 *  Or  http://fullfat.googlecode.com/ for latest releases and the wiki.     *
 *****************************************************************************/


#include "../../../src/ff_time.h"


/**
 *	@file		ff_time.c
 *	@author		James Walmsley
 *	@ingroup	TIME
 *
 *	@defgroup	TIME Real-Time Clock Interface
 *	@brief		Allows FullFAT to time-stamp files.
 *
 *	Provides a means for receiving the time on any platform.
 **/


#ifdef	FF_TIME_SUPPORT

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
	
	unsigned char nHour;
	unsigned char nMinute;
	unsigned char nSecond;	
	unsigned char nDay;
	unsigned char nMonth;
	unsigned short nYear;
	unsigned char nWeekDay;
	
	rtc_getDate(&nWeekDay, &nDay, &nMonth, &nYear);
	rtc_getTime(&nHour, &nMinute, &nSecond);

	pTime->Hour		= nHour;
	pTime->Minute	= nMinute;
	pTime->Second	= nSecond;
	pTime->Day		= nDay;
	pTime->Month	= nMonth;
	pTime->Year		= nYear;

	return 0;
}

#endif
