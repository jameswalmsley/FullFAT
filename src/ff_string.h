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
 *	@file		ff_string.c
 *	@author		James Walmsley
 *	@ingroup	STRING
 *
 *	@defgroup	STRING	FullFAT String Library
 *	@brief		Portable String Library for FullFAT
 *
 *
 **/

#ifndef _FF_STRING_H_
#define _FF_STRING_H_

#include "ff_types.h"
#include "ff_config.h"
#include <string.h>

#ifdef WIN32
#define stricmp stricmp
#define FF_stricmp	stricmp
#else
#define strcasecmp strcasecmp
#define FF_stricmp strcasecmp
#endif

#ifdef FF_UNICODE_SUPPORT
void			FF_tolower		(FF_T_WCHAR *string, FF_T_UINT32 strLen);
void			FF_toupper		(FF_T_WCHAR *string, FF_T_UINT32 strLen);
FF_T_BOOL		FF_strmatch		(const FF_T_WCHAR *str1, const FF_T_WCHAR *str2, FF_T_UINT16 len);
FF_T_WCHAR		*FF_strtok		(const FF_T_WCHAR *string, FF_T_WCHAR *token, FF_T_UINT16 *tokenNumber, FF_T_BOOL *last, FF_T_UINT16 Length);
FF_T_BOOL		FF_wildcompare	(const FF_T_WCHAR *pszWildCard, const FF_T_WCHAR *pszString);

// ASCII to UTF16 and UTF16 to ASCII routines. -- These are lossy routines, and are only for converting ASCII to UTF-16
// and the equivalent back to ASCII. Do not use them for international text.
void FF_cstrtowcs(FF_T_WCHAR *wcsDest, const FF_T_INT8 *szpSource);
void FF_wcstocstr(FF_T_INT8 *szpDest, const FF_T_WCHAR *wcsSource);
void FF_cstrntowcs(FF_T_WCHAR *wcsDest, const FF_T_INT8 *szpSource, FF_T_UINT32 len);
void FF_wcsntocstr(FF_T_INT8 *szpDest, const FF_T_WCHAR *wcsSource, FF_T_UINT32 len);

#else
void			FF_tolower		(FF_T_INT8 *string, FF_T_UINT32 strLen);
void			FF_toupper		(FF_T_INT8 *string, FF_T_UINT32 strLen);
FF_T_BOOL		FF_strmatch		(const FF_T_INT8 *str1, const FF_T_INT8 *str2, FF_T_UINT16 len);
FF_T_INT8		*FF_strtok		(const FF_T_INT8 *string, FF_T_INT8 *token, FF_T_UINT16 *tokenNumber, FF_T_BOOL *last, FF_T_UINT16 Length);
FF_T_BOOL		FF_wildcompare	(const FF_T_INT8 *pszWildCard, const FF_T_INT8 *pszString);
#endif

#endif
