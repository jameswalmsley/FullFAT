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

/**
 *	This file defines some portable types.
 *	You should change these as appropriate for your platform, as necessary.
 *
 **/

#ifndef _FF_TYPES_H_
#define _FF_TYPES_H_

//---------------- BOOLEAN TYPES
typedef	char			FF_T_BOOL;		///< This can be a char if your compiler isn't C99

#define FF_TRUE		1	///< 1 if bool not supported.
#define FF_FALSE	0	///< 0 if bool not supported.

//---------------- 8 BIT INTEGERS
typedef	char			FF_T_INT8;		///< 8 bit default integer
typedef	unsigned char	FF_T_UINT8;		///< 8 bit unsigned integer
typedef signed char		FF_T_SINT8;		///< 8 bit signed integer

//---------------- 16 BIT INTEGERS
typedef	short			FF_T_INT16;
typedef	unsigned short	FF_T_UINT16;
typedef	signed short	FF_T_SINT16;

//---------------- 32 BIT INTEGERS
typedef	int				FF_T_INT32;
typedef	unsigned int	FF_T_UINT32;
typedef	signed int		FF_T_SINT32;

//---------------- 64 BIT INTEGERS			// If you cannot define these, then make sure you see ff_config.h
typedef long long			FF_T_INT64;		// about 64-bit number support.
typedef unsigned long long	FF_T_UINT64;	// It means that FF_GetVolumeSize() cannot return a size
typedef signed long long	FF_T_SINT64;	// > 4GB in bytes if you cannot support 64-bits integers.
											// No other function makes use of 64-bit numbers.
#endif
