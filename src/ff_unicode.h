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
 *	@file		ff_unicode.c
 *	@author		James Walmsley
 *	@ingroup	UNICODE
 *
 **/

#ifndef _FF_UNICODE_H_
#define _FF_UNICODE_H_

#include "ff_config.h"
#include "ff_types.h"
#include "ff_error.h"

// UTF8 / UTF16 Transformation Functions

FF_T_UINT FF_GetUtf16SequenceLen	(FF_T_UINT16 usLeadChar);
FF_T_SINT32 FF_Utf8ctoUtf16c		(FF_T_UINT16 *utf16Dest, const FF_T_UINT8 *utf8Source, FF_T_UINT32 ulSize);
FF_T_SINT32 FF_Utf16ctoUtf8c		(FF_T_UINT8 *utf8Dest, const FF_T_UINT16 *utf16Source, FF_T_UINT32 ulSize);

// UTF16 / UTF32 Transformation Functions

FF_T_SINT32 FF_Utf16ctoUtf32c(FF_T_UINT32 *utf32Dest, const FF_T_UINT16 *utf16Source);
FF_T_SINT32 FF_Utf32ctoUtf16c(FF_T_UINT16 *utf16Dest, FF_T_UINT32 utf32char, FF_T_UINT32 ulSize);

// String transformations
FF_T_SINT32 FF_Utf32stoUtf8s(FF_T_UINT8 *Utf8String, FF_T_UINT32 *Utf32String);

#endif
