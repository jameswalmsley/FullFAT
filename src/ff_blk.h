/*****************************************************************************
 *  FullFAT - High Performance, Thread-Safe Embedded FAT File-System         *
 *                                                                           *
 *  Copyright(C) 2009  James Walmsley  (james@fullfat-fs.co.uk)              *
 *  Many Thanks to     Hein Tibosch    (hein_tibosch@yahoo.es)               *
 *                                                                           *
 *  See RESTRICTIONS.TXT for extra restrictions on the use of FullFAT.       *
 *                                                                           *
 *                FULLFAT IS NOT FREE FOR COMMERCIAL USE                     *
 *                                                                           *
 *  Removing this notice is illegal and will invalidate this license.        *
 *****************************************************************************
 *  See http://www.fullfat-fs.co.uk/ for more information.                   *
 *  Or  http://fullfat.googlecode.com/ for latest releases and the wiki.     *
 *****************************************************************************
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
 *****************************************************************************/

/**
 *	@file		ff_blk.h
 *	@author		James Walmsley
 *	@ingroup	BLK
 *
 *	Helps calculate block numbers.
 **/

#ifndef _FF_BLK_H_
#define _FF_BLK_H_

#include "ff_ioman.h"

// PROTOTYPES:
FF_T_UINT32 FF_getClusterPosition	(FF_IOMAN *pIoman, FF_T_UINT32 nEntry, FF_T_UINT16 nEntrySize);
FF_T_UINT32 FF_getClusterChainNumber(FF_IOMAN *pIoman, FF_T_UINT32 nEntry, FF_T_UINT16 nEntrySize);
FF_T_UINT32 FF_getMajorBlockNumber	(FF_IOMAN *pIoman, FF_T_UINT32 nEntry, FF_T_UINT16 nEntrySize);
FF_T_UINT8	FF_getMinorBlockNumber	(FF_IOMAN *pIoman, FF_T_UINT32 nEntry, FF_T_UINT16 nEntrySize);
FF_T_UINT32 FF_getMinorBlockEntry	(FF_IOMAN *pIoman, FF_T_UINT32 nEntry, FF_T_UINT16 nEntrySize);

#endif

