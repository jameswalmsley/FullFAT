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
//#ifndef _FF_CONFIG_H_
//#define _FF_CONFIG_H_
/*
	Here you can change the configuration of FullFAT as appropriate to your
	platform.
*/

//---------- ENDIANESS
#define FF_LITTLE_ENDIAN
//#define FF_BIG_ENDIAN

//---------- LFN (Long File-name) SUPPORT
// Comment this out if you don't want to worry about Patent Issues.
#define FF_LFN_SUPPORT

//---------- 64-Bit Number Support
//#define FF_64_NUM_SUPPORT

//---------- Actively Determine if partition is FAT
#define FF_FAT_CHECK	// This is experimental, so if FullFAT won't mount your volume, comment this out
						// Also report the problem to james@worm.me.uk

//#endif

#ifdef FF_LFN_SUPPORT
#define FF_MAX_FILENAME		260
#else
#define	FF_MAX_FILENAME		13
#endif
