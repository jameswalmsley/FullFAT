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
 *	@file		ff_safety.h
 *	@author		James Walmsley
 *	@ingroup	SAFETY
 **/

#ifndef _FF_SAFETY_H_
#define	_FF_SAFETY_H_

#include <stdlib.h>
#include "ff_types.h"


//---------- PROTOTYPES (in order of appearance)

// PUBLIC:


// PRIVATE:
void		*FF_CreateSemaphore		(void);
void		FF_PendSemaphore		(void *pSemaphore);
void		FF_ReleaseSemaphore		(void *pSemaphore);
void		FF_DestroySemaphore		(void *pSemaphore);
void		FF_Yield				(void);
void		FF_Sleep				(FF_T_UINT32 TimeMs);

#endif

