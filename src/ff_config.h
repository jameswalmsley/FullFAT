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
#define FF_64_NUM_SUPPORT

//---------- Actively Determine if partition is FAT
#define FF_FAT_CHECK	// This is experimental, so if FullFAT won't mount your volume, comment this out
						// Also report the problem to james@worm.me.uk

//#endif
