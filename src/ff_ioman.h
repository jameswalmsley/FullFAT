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

/**
 *	@file		ff_ioman.h
 *	@author		James Walmsley
 *	@ingroup	IOMAN
 **/

#ifndef _FF_IOMAN_H_
#define _FF_IOMAN_H_

#include <stdlib.h>		/* Use of malloc() */
#include "ff_config.h"
#include "ff_types.h"
#include "ff_safety.h"	// Provide critical regions
#include "ff_memory.h"

#define	FF_MAX_PARTITION_NAME				5	///< Partition name length.

#define FF_ERR_IOMAN_NULL_POINTER			-10	///< Null Pointer return error code.
#define FF_ERR_IOMAN_DEV_ALREADY_REGD		-11 ///< Device was already registered.
#define FF_ERR_IOMAN_NO_MOUNTABLE_PARTITION -12
#define FF_ERR_IOMAN_INVALID_FORMAT			-13
#define FF_ERR_IOMAN_INVALID_PARTITION_NUM	-14
#define FF_ERR_IOMAN_NOT_FAT_FORMATTED		-15
#define FF_ERR_IOMAN_DEV_INVALID_BLKSIZE	-16 ///< IOMAN object BlkSize is not compatible with the blocksize of this device driver.
#define FF_ERR_DEVICE_DRIVER_FAILED			-17

#define FF_T_FAT12				0x0A
#define FF_T_FAT16				0x0B
#define FF_T_FAT32				0x0C

#define FF_MODE_READ			0x01		///< Buffer / FILE Mode for Read Access.
#define	FF_MODE_WRITE			0x02		///< Buffer / FILE Mode for Write Access.
#define FF_MODE_DIR				0x80		///< Special Mode to open a Dir.

#define FF_BUF_MAX_HANDLES		65536		///< Maximum number handles sharing a buffer. (16 bit integer, we don't want to overflow it!)

/**
 *	I/O Driver Definitions
 *	Provide access to any Block Device via the following interfaces.
 *	Returns the number of blocks actually read or written.
 **/
typedef FF_T_SINT32 (*FF_WRITE_BLOCKS)	(FF_T_UINT8 *pBuffer, FF_T_UINT32 SectorAddress, FF_T_UINT32 Count, void *pParam);
typedef FF_T_SINT32 (*FF_READ_BLOCKS)	(FF_T_UINT8 *pBuffer, FF_T_UINT32 SectorAddress, FF_T_UINT32 Count, void *pParam);

#define FF_ERR_DRIVER_BUSY			-10
#define FF_ERR_DRIVER_FATAL_ERROR	-11
#define FF_DRIVER_BUSY_SLEEP		200	///< In Milliseconds. How long FullFAT should sleep the thread for in ms, if FF_ERR_DRIVER_BUSY is recieved.
/**
 *	@public
 *	@brief	Describes the block device driver interface to FullFAT.
 **/
typedef struct {
	FF_WRITE_BLOCKS	fnWriteBlocks;	///< Function Pointer, to write a block(s) from a block device.
	FF_READ_BLOCKS	fnReadBlocks;	///< Function Pointer, to read a block(s) from a block device.
	FF_T_UINT16		devBlkSize;		///< Block size that the driver deals with.
	void			*pParam;		///< Pointer to some parameters e.g. for a Low-Level Driver Handle
} FF_BLK_DEVICE;

/**
 *	@private
 *	@brief	FullFAT handles memory with buffers, described as below.
 *	@note	This may change throughout development.
 **/
typedef struct {
	FF_T_UINT16		ID;				///< Auto-Incremental Buffer ID.
	FF_T_UINT32		Sector;			///< The LBA of the Cached sector.
	FF_T_UINT8		Mode;			///< Read or Write mode.
	FF_T_UINT16		ContextID;		///< Context Identifier.
	FF_T_UINT16		NumHandles;		///< Number of objects using this buffer.
	FF_T_UINT16		Persistance;	///< For the persistance algorithm.
	FF_T_BOOL		Modified;		///< If the sector was modified since read.
	FF_T_BOOL		isIOMANediting;	///< FF_TRUE if the buffer manager is currently editing this.
	FF_T_UINT8		*pBuffer;		///< Pointer to the cache block.
} FF_BUFFER;

/**
 *	@private
 *	@brief	FullFAT identifies a partition with the following data.
 *	@note	This may shrink as development and optimisation goes on.
 **/
typedef struct {
	FF_T_UINT8		ID;					///< Partition Incremental ID number.
	FF_T_UINT8		Type;				///< Partition Type Identifier.
	FF_T_UINT16		BlkSize;			///< Size of a Sector Block in bytes.
	FF_T_UINT8      BlkFactor;			///< Scale Factor for blocksizes above 512!
	FF_T_INT8		Name[FF_MAX_PARTITION_NAME];	///< Partition Identifier e.g. c: sd0: etc.
	FF_T_INT8		VolLabel[12];		///< Volume Label of the partition.
	FF_T_UINT32		BeginLBA;			///< LBA start address of the partition.
	FF_T_UINT32		PartSize;			///< Size of Partition in number of sectors.
	FF_T_UINT32		FatBeginLBA;		///< LBA of the FAT tables.
	FF_T_UINT8		NumFATS;			///< Number of FAT tables.
	FF_T_UINT32		SectorsPerFAT;		///< Number of sectors per Fat.
	FF_T_UINT8		SectorsPerCluster;	///< Number of sectors per Cluster.
	FF_T_UINT32		TotalSectors;
	FF_T_UINT32		DataSectors;
	FF_T_UINT32		RootDirSectors;
	FF_T_UINT32		FirstDataSector;
	FF_T_UINT16		ReservedSectors;
	FF_T_UINT32		ClusterBeginLBA;	///< LBA of first cluster.
	FF_T_UINT32		NumClusters;		///< Number of clusters.
	FF_T_UINT32		RootDirCluster;		///< Cluster number of the root directory entry.
	FF_T_UINT32		LastFreeCluster;
} FF_PARTITION;



/**
 *	@public
 *	@brief	FF_IOMAN Object description.
 *
 *	FullFAT functions around an object like this.
 **/
typedef struct {
	FF_BLK_DEVICE	*pBlkDevice;	///< Pointer to a Block device description.
	FF_PARTITION	*pPartition;	///< Pointer to a partition description.
	FF_BUFFER		*pBuffers;		///< Pointer to the first buffer description.
	FF_T_UINT16		BlkSize;		///< The Block size that IOMAN is configured to.
	FF_T_UINT8		*pCacheMem;		///< Pointer to a block of memory for the cache.
	FF_T_UINT8		CacheSize;		///< Size of the cache in number of Sectors.
	FF_T_UINT8		MemAllocation;	///< Bit-Mask identifying allocated pointers.
	FF_T_UINT8		FatLock;		///< Lock Flag for FAT (This must be accessed via a semaphore).
	FF_T_UINT8		DirLock;		///< Lock Flag for Dir Modification (This must be accessed via a semaphore).
	void			*pSemaphore;	///< Pointer to a Semaphore object. (For buffer description modifications only!).
	void			*FirstFile;		///< Pointer to the first File object.
} FF_IOMAN;

// Bit-Masks for Memory Allocation testing.
#define FF_IOMAN_ALLOC_BLKDEV	0x01	///< Flags the pBlkDevice pointer is allocated.
#define FF_IOMAN_ALLOC_PART		0x02	///< Flags the pPartition pointer is allocated.
#define	FF_IOMAN_ALLOC_BUFDESCR	0x04	///< Flags the pBuffers pointer is allocated.
#define	FF_IOMAN_ALLOC_BUFFERS	0x08	///< Flags the pCacheMem pointer is allocated.
#define FF_IOMAN_ALLOC_RESERVED	0xF0	///< Reserved Section.


//---------- PROTOTYPES (in order of appearance)

// PUBLIC (Interfaces):
FF_IOMAN	*FF_CreateIOMAN		(FF_T_UINT8 *pCacheMem, FF_T_UINT32 Size, FF_T_UINT16 BlkSize);
FF_T_SINT8	FF_DestroyIOMAN		(FF_IOMAN *pIoman);
FF_T_SINT8	FF_RegisterBlkDevice(FF_IOMAN *pIoman, FF_T_UINT16 BlkSize, FF_WRITE_BLOCKS fnWriteBlocks, FF_READ_BLOCKS fnReadBlocks, void *pParam);
FF_T_SINT8	FF_MountPartition	(FF_IOMAN *pIoman, FF_T_UINT8 PartitionNumber);
FF_T_SINT8	FF_FlushCache		(FF_IOMAN *pIoman);

#ifdef FF_64_NUM_SUPPORT
FF_T_UINT64 FF_GetVolumeSize(FF_IOMAN *pIoman);
#else
FF_T_UINT32 FF_GetVolumeSize(FF_IOMAN *pIoman);
#endif
// PUBLIC  (To FullFAT Only):
FF_BUFFER	*FF_GetBuffer		(FF_IOMAN *pIoman, FF_T_UINT32 Sector, FF_T_UINT8 Mode);
void		FF_ReleaseBuffer	(FF_IOMAN *pIoman, FF_BUFFER *pBuffer);

// PRIVATE (For this module only!):
static void		FF_IOMAN_InitBufferDescriptors	(FF_IOMAN *pIoman);

#endif


