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
 *	This file provides some commands allowing one to test multi-threaded I/O on FullFAT.
 **/

#include "../../../src/fullfat.h"
#include "../../../../FFTerm/src/FFTerm.h"
#include "cmd.h"
#include <windows.h>
#define FFTEST_MAX_THREADS	10
extern const FFT_ERR_TABLE mkthreadInfo[];
extern const FFT_ERR_TABLE killthreadInfo[];
extern const FFT_ERR_TABLE listthreadsInfo[];

int createthread_cmd	(int argc, char **argv, FF_ENVIRONMENT *pEnv);
int killthread_cmd		(int argc, char **argv, FF_ENVIRONMENT *pEnv);
int listthreads_cmd		(int argc, char **argv, FF_ENVIRONMENT *pEnv);

