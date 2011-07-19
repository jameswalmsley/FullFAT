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

/**
 *	@file		ff_time.h
 *	@author		James Walmsley
 *	@ingroup	TIME
 *
 *	Provides a means for receiving the time on any platform.
 **/

#ifndef _FF_TIME_H_
#define _FF_TIME_H_

#include "ff_config.h"
#include "ff_types.h"

/**
 *	@public
 *	@brief	A TIME and DATE object for FullFAT. A FullFAT time driver must populate these values.
 *
 **/
typedef struct {
	FF_T_UINT16	Year;		///< Year	(e.g. 2009).
	FF_T_UINT16 Month;		///< Month	(e.g. 1 = Jan, 12 = Dec).
	FF_T_UINT16	Day;		///< Day	(1 - 31).
	FF_T_UINT16 Hour;		///< Hour	(0 - 23).
	FF_T_UINT16 Minute;		///< Min	(0 - 59).
	FF_T_UINT16 Second;		///< Second	(0 - 59).
} FF_SYSTEMTIME;

//---------- PROTOTYPES

FF_T_SINT32	FF_GetSystemTime(FF_SYSTEMTIME *pTime);

#endif

