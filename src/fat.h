/******************************************************************************
 *   FullFAT - Embedded FAT File-System
 *
 *   Provides a full, thread-safe, implementation of the FAT file-system
 *   suitable for low-power embedded systems.
 *
 *   Written by James Walmsley, james@worm.me.uk, www.worm.me.uk/fullfat/
 *
 *   Copyright 2009 James Walmsley
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 *   Commercial support is available for FullFAT, for more information
 *   please contact the author, james@worm.me.uk
 *
 *   Removing the above notice is illegal and will invalidate this license.
 *****************************************************************************/
#ifndef _FAT_H_
#define _FAT_H_

/*
	This file defines offsets to various data for the FAT specification.
*/

// MBR / PBR Offsets

#define FF_FAT_BYTES_PER_SECTOR		0x00B
#define FF_FAT_SECTORS_PER_CLUS		0x00D
#define FF_FAT_RESERVED_SECTORS		0x00E
#define FF_FAT_NUMBER_OF_FATS		0x010
#define FF_FAT_ROOT_ENTRY_COUNT		0x011
#define FF_FAT_16_TOTAL_SECTORS		0x013
#define FF_FAT_32_TOTAL_SECTORS		0x020
#define FF_FAT_16_SECTORS_PER_FAT	0x016
#define FF_FAT_32_SECTORS_PER_FAT	0x024
#define FF_FAT_ROOT_DIR_CLUSTER		0x02C

#define FF_FAT_PTBL					0x1BE
#define FF_FAT_PTBL_LBA				0x008

// Directory Entry Offsets
#define FF_FAT_DIRENT_SHORTNAME		0x000
#define FF_FAT_DIRENT_ATTRIB		0x00B
#define FF_FAT_DIRENT_CLUS_HIGH		0x014
#define FF_FAT_DIRENT_CLUS_LOW		0x01A
#define FF_FAT_DIRENT_FILESIZE		0x01C

// Dirent Attributes
#define FF_FAT_ATTR_READONLY		0x01
#define FF_FAT_ATTR_HIDDEN			0x02
#define FF_FAT_ATTR_SYSTEM			0x04
#define FF_FAT_ATTR_VOLID			0x08
#define FF_FAT_ATTR_DIR				0x10
#define FF_FAT_ATTR_ARCHIVE			0x20
#define FF_FAT_ATTR_LFN				0x0F

#endif
