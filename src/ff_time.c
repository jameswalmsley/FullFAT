/*****************************************************************************
 *  FullFAT - High Performance, Thread-Safe Embedded FAT File-System         *
 *                                                                           *
 *  Copyright(C) 2009 James Walmsley  (james@fullfat-fs.co.uk)               *
 *	Copyright(C) 2010 Hein Tibosch    (hein_tibosch@yahoo.es)                *
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


#include "ff_time.h"


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
	
	pTime->Hour		= 0;
	pTime->Minute	= 0;
	pTime->Second	= 0;
	pTime->Day		= 0;
	pTime->Month	= 0;
	pTime->Year		= 0;

	return 0;
}

#endif
