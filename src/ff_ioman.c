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
	pIoman->MemAllocation = (FF_T_UINT8) 0x00;	// Unset all allocation identifiers.

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
	FF_T_UINT16 i;
	for(i = 0; i < pIoman->CacheSize; i++) {
		pIoman->pBuffers->ID 			= i;
		pIoman->pBuffers->ContextID		= 0;
		pIoman->pBuffers->Mode			= 0;
		pIoman->pBuffers->NumHandles 	= 0;
		pIoman->pBuffers->Persistance 	= 0;
		pIoman->pBuffers->Sector 		= 0;
		pIoman->pBuffers->pBuffer 		= ((pIoman->pCacheMem) + 512 * i);
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
FF_T_BOOL FF_IOMAN_ModeValid(FF_T_INT8 Mode) {
	if(Mode != FF_MODE_READ || Mode != FF_MODE_WRITE) {
		return FF_FALSE;
	}
	return FF_TRUE;
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
void FF_IOMAN_FillBuffer(FF_IOMAN *pIoman, FF_T_UINT32 Sector, FF_T_INT8 *pBuffer) {
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

