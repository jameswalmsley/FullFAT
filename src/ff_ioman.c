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
 *	@file		ff_ioman.c
 *	@author		James Walmsley
 *	@ingroup	IOMAN
 *
 *	@defgroup	IOMAN	I/O Manager
 *	@brief		Handles IO buffers for FullFAT safely.
 *
 *	Provides a simple static interface to the rest of FullFAT to manage
 *	buffers. It also defines the public interfaces for Creating and 
 *	Destroying a FullFAT IO object.
 **/

#include "ff_ioman.h"	// Includes ff_types.h, ff_safety.h, <stdio.h>
#include "fat.h"

/**
 *	@public
 *	@brief	Creates an FF_IOMAN object, to initialise FullFAT
 *
 *	@param	pCacheBuffer	Pointer to a buffer for the cache. (NULL if ok to Malloc).
 *	@param	Size			The size of the provided buffer, or size of the cache to be created.
 *
 *	@return	Returns a pointer to an FF_IOMAN type object.
 *
 **/
FF_IOMAN *FF_CreateIOMAN(FF_T_INT8 *pCacheMem, FF_T_UINT32 Size) {
	
	FF_IOMAN *pIoman = NULL;

	if((Size % 512) != 0 || Size == 0) {
		return NULL;	// Memory Size not a multiple of 512 > 0
	}

	pIoman = (FF_IOMAN *) malloc(sizeof(FF_IOMAN));
	
	if(!pIoman) {		// Ensure malloc() succeeded.
		return NULL;
	}

	// This is just a bit-mask, to use a byte to keep track of memory.
	pIoman->MemAllocation = 0x00;	// Unset all allocation identifiers.
	
	pIoman->pPartition	= (FF_PARTITION  *) malloc(sizeof(FF_PARTITION));
	if(pIoman->pPartition) {	// If succeeded, flag that allocation.
		pIoman->MemAllocation |= FF_IOMAN_ALLOC_PART;
	} else {
		FF_DestroyIOMAN(pIoman);
		return NULL;
	}

	pIoman->pBlkDevice	= (FF_BLK_DEVICE *) malloc(sizeof(FF_BLK_DEVICE));
	if(pIoman->pBlkDevice) {	// If succeeded, flag that allocation.
		pIoman->MemAllocation |= FF_IOMAN_ALLOC_BLKDEV;

		// Make sure all pointers are NULL
		pIoman->pBlkDevice->fnReadBlocks = NULL;
		pIoman->pBlkDevice->fnWriteBlocks = NULL;
		pIoman->pBlkDevice->pParam = NULL;

	} else {
		FF_DestroyIOMAN(pIoman);
		return NULL;
	}
	
	// Organise the memory provided, or create our own!
	if(pCacheMem) {
		pIoman->pCacheMem = pCacheMem;
		pIoman->CacheSize = Size / 512;
	}else {	// No-Cache buffer provided (malloc)
		pIoman->pCacheMem = (FF_T_INT8 *) malloc(Size);
		if(!pIoman->pCacheMem) {
			FF_DestroyIOMAN(pIoman);
			return NULL;
		}
		pIoman->MemAllocation |= FF_IOMAN_ALLOC_BUFFERS;
		pIoman->CacheSize = Size / 512;
	}
	
	/*	Malloc() memory for buffer objects. (FullFAT never refers to a buffer directly
		but uses buffer objects instead. Allows us to provide thread safety.	
	*/
	pIoman->pBuffers = (FF_BUFFER *) malloc(sizeof(FF_BUFFER) * pIoman->CacheSize);
	
	if(pIoman->pBuffers) {
		pIoman->MemAllocation |= FF_IOMAN_ALLOC_BUFDESCR;
		FF_IOMAN_InitBufferDescriptors(pIoman);
	} else {
		FF_DestroyIOMAN(pIoman);
	}

	// Finally create a Semaphore for Buffer Description modifications.
	pIoman->pSemaphore = FF_CreateSemaphore();

	return pIoman;	// Sucess, return the created object.
}

/**
 *	@public
 *	@brief	Destroys an FF_IOMAN object, and frees all assigned memory.
 *
 *	@param	pIoman	Pointer to an FF_IOMAN object, as returned from FF_CreateIOMAN.
 *
 *	@return	Zero on sucess, or a documented error code on failure. (FF_IOMAN_NULL_POINTER)
 *
 **/
FF_T_SINT8 FF_DestroyIOMAN(FF_IOMAN *pIoman) {

	// Ensure no NULL pointer was provided.
	if(!pIoman) {
		return FF_ERR_IOMAN_NULL_POINTER;
	}
	
	// Ensure pPartition pointer was allocated.
	if((pIoman->MemAllocation & FF_IOMAN_ALLOC_PART)) {
		free(pIoman->pPartition);
	}

	// Ensure pBlkDevice pointer was allocated.
	if((pIoman->MemAllocation & FF_IOMAN_ALLOC_BLKDEV)) {
		free(pIoman->pBlkDevice);
	}

	// Ensure pBuffers pointer was allocated.
	if((pIoman->MemAllocation & FF_IOMAN_ALLOC_BUFDESCR)) {
		free(pIoman->pBuffers);
	}

	// Ensure pCacheMem pointer was allocated.
	if((pIoman->MemAllocation & FF_IOMAN_ALLOC_BUFFERS)) {
		free(pIoman->pCacheMem);
	}
	
	// Destroy any Semaphore that was created.
	if(pIoman->pSemaphore) {
		FF_DestroySemaphore(pIoman->pSemaphore);
	}
	
	// Finally free the FF_IOMAN object.
	free(pIoman);

	return 0;
}

/**
 *	@private
 *	@brief	Initialises Buffer Descriptions as part of the FF_IOMAN object initialisation.
 *
 **/
void FF_IOMAN_InitBufferDescriptors(FF_IOMAN *pIoman) {
	int i;
	for(i = 0; i < pIoman->CacheSize; i++) {
		pIoman->pBuffers->ID 			= i;
		pIoman->pBuffers->ContextID		= 0;
		pIoman->pBuffers->Mode			= 0;
		pIoman->pBuffers->NumHandles 	= 0;
		pIoman->pBuffers->Persistance 	= 0;
		pIoman->pBuffers->Sector 		= 0;
		pIoman->pBuffers->pBuffer 		= (FF_T_UINT8 *)((pIoman->pCacheMem) + 512 * i);
	}
}

/**
 *	@private
 *	@brief	Tests the Mode for validity.
 *
 *	@param	Mode	Mode of buffer to check. 
 *
 *	@return	FF_TRUE when valid, else FF_FALSE.
 **/
FF_T_BOOL FF_IOMAN_ModeValid(FF_T_UINT8 Mode) {
	
	if(Mode == FF_MODE_READ || Mode == FF_MODE_WRITE) {
		return FF_TRUE;
	}
	
	return FF_FALSE;
}


/**
 *	@private
 *	@brief	Fills a buffer with the appropriate sector via the device driver.
 *
 *	@param	pIoman	FF_IOMAN object.
 *	@param	Sector	LBA address of the sector to fetch.
 *	@param	pBuffer	Pointer to a byte-wise buffer to store the fetched data.
 *
 *	@return	FF_TRUE when valid, else FF_FALSE.
 **/
void FF_IOMAN_FillBuffer(FF_IOMAN *pIoman, FF_T_UINT32 Sector, FF_T_UINT8 *pBuffer) {
	if(pIoman->pBlkDevice->fnReadBlocks) {	// Make sure we don't execute a NULL.
		pIoman->pBlkDevice->fnReadBlocks(pBuffer, Sector, 1, pIoman->pBlkDevice->pParam);
	}
}



/**
 *	@private
 *	@brief	Return's a pointer to a valid buffer resource
 *
 *	@param	pIoman	FF_IOMAN object.
 *	@param	Sector	LBA address of the sector to fetch.
 *	@param	Mode	FF_READ or FF_WRITE access modes.
 *
 *	@return	Pointer to the Buffer description.
 **/
FF_BUFFER *FF_GetBuffer(FF_IOMAN *pIoman, FF_T_UINT32 Sector, FF_T_INT8 Mode) {

	int i;

	if(!pIoman) {
		return NULL;	// No NULL pointer modification.
	}

	if(!FF_IOMAN_ModeValid(Mode)) {
		return NULL;	// Make sure mode is correctly set.
	}

	// Get a Semaphore now, and release on any return.
	// Improves speed of the searching. Prevents description changes
	// Mid-loop. This could be a performance issue later on!!

	FF_PendSemaphore(pIoman->pSemaphore);

	// Search for an appropriate buffer.
	if(Mode == FF_MODE_READ) {
		for(i = 0; i < pIoman->CacheSize; i++) {
			if((pIoman->pBuffers + i)->Sector == Sector && (pIoman->pBuffers + i)->Mode == FF_MODE_READ ) {
				// Buffer is suitable, ensure we don't overflow its handle count.
				if((pIoman->pBuffers + i)->NumHandles < FF_BUF_MAX_HANDLES) {				
					(pIoman->pBuffers + i)->NumHandles ++;
					//(pIoman->pBuffers + i)->Persistance ++;
					FF_ReleaseSemaphore(pIoman->pSemaphore);
					return (pIoman->pBuffers + i);
				}
			}
		}
	}

	for(i = 0; i < pIoman->CacheSize; i++) {
		if((pIoman->pBuffers + i)->NumHandles == 0) {
			(pIoman->pBuffers + i)->Mode = Mode;
			(pIoman->pBuffers + i)->NumHandles = 1;
			(pIoman->pBuffers + i)->Sector = Sector;
			// Fill the buffer with data from the device.
			FF_IOMAN_FillBuffer(pIoman, Sector,(pIoman->pBuffers + i)->pBuffer);
			FF_ReleaseSemaphore(pIoman->pSemaphore);
			return (pIoman->pBuffers + i);
		}
	}

	// TODO: Think of a better way to deal with this situation.
	FF_ReleaseSemaphore(pIoman->pSemaphore);	// Important that we free access!
	return NULL;	// No buffer available.
}


/**
 *	@private
 *	@brief	Releases a buffer resource.
 *
 *	@param	pIoman	Pointer to an FF_IOMAN object.
 *	@param	pBuffer	Pointer to an FF_BUFFER object.
 *
 **/
void FF_ReleaseBuffer(FF_IOMAN *pIoman, FF_BUFFER *pBuffer) {
	if(pIoman && pBuffer) {
		// Protect description changes with a semaphore.		
		FF_PendSemaphore(pIoman->pSemaphore);
			pBuffer->NumHandles--;
		FF_ReleaseSemaphore(pIoman->pSemaphore);
		
		// Maybe for later when we handle buffers more complex
		/*pBuffer->Persistance--;
		if(pBuffer->NumHandles == 0) {
			pBuffer->Persistance = 0;
		}*/
	}
}

/**
 *	@public
 *	@brief	Registers a device driver with FullFAT
 *
 *	The device drivers must adhere to the specification provided by
 *	FF_WRITE_BLOCKS and FF_READ_BLOCKS.
 *
 *	@param	pIoman			FF_IOMAN object.
 *	@param	fnWriteBlocks	Pointer to the Write Blocks to device function, as described by FF_WRITE_BLOCKS.
 *	@param	fnReadBlocks	Pointer to the Read Blocks from device function, as described by FF_READ_BLOCKS.
 *	@param	pParam			Pointer to a parameter for use in the functions. 
 *
 *	@return	0 on success, FF_ERR_IOMAN_DEV_ALREADY_REGD if a device was already hooked, FF_ERR_IOMAN_NULL_POINTER if a pIoman object wasn't provided.
 **/
FF_T_SINT8 FF_RegisterBlkDevice(FF_IOMAN *pIoman, FF_WRITE_BLOCKS fnWriteBlocks, FF_READ_BLOCKS fnReadBlocks, void *pParam) {
	if(!pIoman) {	// We can't do anything without an IOMAN object.
		return FF_ERR_IOMAN_NULL_POINTER;
	}
	
	// Ensure that a device cannot be re-registered "mid-flight"
	// Doing so would corrupt the context of FullFAT
	if(pIoman->pBlkDevice->fnReadBlocks) {
		return FF_ERR_IOMAN_DEV_ALREADY_REGD;
	}
	if(pIoman->pBlkDevice->fnWriteBlocks) {
		return FF_ERR_IOMAN_DEV_ALREADY_REGD;
	}
	if(pIoman->pBlkDevice->pParam) {
		return FF_ERR_IOMAN_DEV_ALREADY_REGD;
	}
	
	// Here we shall just set the values.
	// FullFAT checks before using any of these values.
	pIoman->pBlkDevice->fnReadBlocks	= fnReadBlocks;
	pIoman->pBlkDevice->fnWriteBlocks	= fnWriteBlocks;
	pIoman->pBlkDevice->pParam			= pParam;

	return 0;	// Success
}


void FF_DetermineFatType(FF_IOMAN *pIoman) {
	
	FF_PARTITION *pPart;
	if(pIoman) {
		pPart = pIoman->pPartition;
		if(pPart->NumClusters < 4085) {
			// FAT12
			pPart->Type = FF_T_FAT12;
		} else if(pPart->NumClusters < 65525) {
			// FAT 16
			pPart->Type = FF_T_FAT16;
		}
		else {
			// FAT 32!
			pPart->Type = FF_T_FAT32;
		}
	}
}
/*
	
*/
FF_T_SINT8 FF_MountPartition(FF_IOMAN *pIoman) {
	FF_PARTITION	*pPart	 = pIoman->pPartition;
	FF_BUFFER		*pBuffer = 0;

	pBuffer = FF_GetBuffer(pIoman, 0, FF_MODE_READ);
	
	pPart->BlkSize = FF_getShort(pBuffer->pBuffer, FF_FAT_BYTES_PER_SECTOR);

	if(pPart->BlkSize == 512 || pPart->BlkSize == 1024 || pPart->BlkSize == 2048 || pPart->BlkSize == 4096) {
		// Volume is not partitioned (MBR Found)
		pPart->BeginLBA = 0;
	} else {
		// Primary Partitions to deal with!
		pPart->BeginLBA = FF_getShort(pBuffer->pBuffer, FF_FAT_PTBL + FF_FAT_PTBL_LBA);
		FF_ReleaseBuffer(pIoman, pBuffer);
		
		if(!pPart->BeginLBA) {
			return FF_ERR_IOMAN_NO_MOUNTABLE_PARTITION;
		}
		// Now we get the Partition sector.
		pBuffer = FF_GetBuffer(pIoman, pPart->BeginLBA, FF_MODE_READ);
		
		pPart->BlkSize = FF_getShort(pBuffer->pBuffer, FF_FAT_BYTES_PER_SECTOR);
		if(pPart->BlkSize != 512 && pPart->BlkSize != 1024 && pPart->BlkSize != 2048 && pPart->BlkSize != 4096) {
			return FF_ERR_IOMAN_INVALID_FORMAT;
		}
	}
	// Assume FAT16, then we'll adjust if its FAT32
	pPart->ReservedSectors = FF_getShort(pBuffer->pBuffer, FF_FAT_RESERVED_SECTORS);
	pPart->FatBeginLBA = pPart->BeginLBA + pPart->ReservedSectors;

	pPart->NumFATS = FF_getShort(pBuffer->pBuffer, FF_FAT_NUMBER_OF_FATS);
	pPart->SectorsPerFAT = (FF_T_UINT32) FF_getShort(pBuffer->pBuffer, FF_FAT_16_SECTORS_PER_FAT);

	pPart->SectorsPerCluster = FF_getChar(pBuffer->pBuffer, FF_FAT_SECTORS_PER_CLUS);

	if(pPart->SectorsPerFAT == 0) {	// FAT32
		pPart->SectorsPerFAT = FF_getLong(pBuffer->pBuffer, FF_FAT_32_SECTORS_PER_FAT);
		pPart->RootDirCluster = FF_getLong(pBuffer->pBuffer, FF_FAT_ROOT_DIR_CLUSTER);
		pPart->ClusterBeginLBA = pPart->BeginLBA + pPart->ReservedSectors + (pPart->NumFATS * pPart->SectorsPerFAT);
		pPart->TotalSectors = (FF_T_UINT32) FF_getShort(pBuffer->pBuffer, FF_FAT_16_TOTAL_SECTORS);
		if(pPart->TotalSectors == 0) {
			pPart->TotalSectors = FF_getLong(pBuffer->pBuffer, FF_FAT_32_TOTAL_SECTORS);
		}
	} else {	// FAT16
		pPart->ClusterBeginLBA = pPart->BeginLBA + pPart->ReservedSectors + (pPart->NumFATS * pPart->SectorsPerFAT);
		pPart->TotalSectors = (FF_T_UINT32) FF_getShort(pBuffer->pBuffer, FF_FAT_16_TOTAL_SECTORS);
		pPart->RootDirCluster = 1; // 1st Cluster is RootDir!
		if(pPart->TotalSectors == 0) {
			pPart->TotalSectors = FF_getLong(pBuffer->pBuffer, FF_FAT_32_TOTAL_SECTORS);
		}
	}
	
	FF_ReleaseBuffer(pIoman, pBuffer);	// Release the buffer finally!
	pPart->RootDirSectors = ((FF_getShort(pBuffer->pBuffer, FF_FAT_ROOT_ENTRY_COUNT) * 32) + pPart->BlkSize - 1) / pPart->BlkSize;
	
	pPart->DataSectors = pPart->TotalSectors - (pPart->ReservedSectors + (pPart->NumFATS * pPart->SectorsPerFAT) + pPart->RootDirSectors);
	pPart->NumClusters = pPart->DataSectors / pPart->SectorsPerCluster;
	FF_DetermineFatType(pIoman);

	return 0;
}
