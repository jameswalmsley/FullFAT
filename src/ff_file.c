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
 *  See EXCEPTIONS.TXT for extra restrictions on the use of FullFAT.         *
 *                                                                           *
 *  Removing the above notice is illegal and will invalidate this license.   *
 *****************************************************************************
 *  See http://worm.me.uk/fullfat for more information.                      *
 *  Or  http://fullfat.googlecode.com/ for latest releases and the wiki.     *
 *****************************************************************************/

/**
 *	@file		ff_file.c
 *	@author		James Walmsley
 *	@ingroup	FILEIO
 *
 *	@defgroup	FILEIO FILE I/O Access
 *	@brief		Provides an interface to allow File I/O on a mounted IOMAN.
 *
 *	Provides file-system interfaces for the FAT file-system.
 **/

#include "ff_file.h"

/**
 *	@public
 *	@brief	Opens a File for Access
 *
 *	@param	pIoman		FF_IOMAN object that was created by FF_CreateIOMAN().
 *	@param	path		Path to the File or object.
 *	@param	Mode		Access Mode required.
 *
 **/
FF_FILE *FF_Open(FF_IOMAN *pIoman, FF_T_INT8 *path, FF_T_UINT8 Mode) {
	FF_FILE		*pFile;
	FF_DIRENT	Object;
	FF_T_UINT32 DirCluster, FileCluster;
	FF_T_INT8	filename[FF_MAX_FILENAME];

	FF_T_UINT16	i;

	if(!pIoman) {
		return (FF_FILE *)NULL;
	}
	pFile = malloc(sizeof(FF_FILE));
	if(!pFile) {
		return (FF_FILE *)NULL;
	}

	i = (FF_T_UINT16) strlen(path);

	while(i != 0) {
		if(path[i] == '\\' || path[i] == '/') {
			break;
		}
		i--;
	}

	strncpy(filename, (path + i + 1), FF_MAX_FILENAME);

	if(i == 0) {
		i = 1;
	}
	
	DirCluster = FF_FindDir(pIoman, path, i);
	

	if(DirCluster) {
		FileCluster = FF_FindEntry(pIoman, DirCluster, filename, 0x00, &Object);
		if(!FileCluster) {	// If 0 was returned, it might be because the file has no allocated cluster
			FF_tolower(Object.FileName, FF_MAX_FILENAME);
			FF_tolower(filename, FF_MAX_FILENAME);
			if(Object.Filesize == 0 && strcmp(filename, Object.FileName) == 0) {
				// The file really was found!
				FileCluster = 1;
			}
		}
		if(FileCluster) {
			pFile->pIoman = pIoman;
			pFile->FilePointer = 0;
			pFile->ObjectCluster = Object.ObjectCluster;
			pFile->Filesize = Object.Filesize;
			pFile->CurrentCluster = 0;
			pFile->AddrCurrentCluster = pFile->ObjectCluster;
			pFile->Mode = Mode;
			return pFile;
		}
	}

	return (FF_FILE *)NULL;
}


/**
 *	@public
 *	@brief	Get's the next Entry based on the data recorded in the FF_DIRENT object.
 *
 *	@param	pFile		FF_FILE object that was created by FF_Open().
 *
 *	@return FF_TRUE if End of File was reached. FF_FALSE if not.
 *	@return FF_FALSE if a null pointer was provided.
 *
 **/
FF_T_BOOL FF_isEOF(FF_FILE *pFile) {
	if(!pFile) {
		return FF_FALSE;
	}
	if(pFile->FilePointer >= pFile->Filesize) {
		return FF_TRUE;
	} else {
		return FF_FALSE;
	}
}


/**
 *	@public
 *	@brief	Equivalent to fread()
 *
 *	@param	pFile		FF_FILE object that was created by FF_Open().
 *	@param	ElementSize	The size of an element to read.
 *	@param	Count		The number of elements to read.
 *	@param	buffer		A pointer to a buffer of adequate size to be filled with the requested data.
 *
 *	@return Number of bytes read.
 *
 **/
FF_T_UINT32 FF_Read(FF_FILE *pFile, FF_T_UINT32 ElementSize, FF_T_UINT32 Count, FF_T_UINT8 *buffer) {
	FF_T_UINT32		Bytes = ElementSize * Count;
	FF_T_UINT32		BytesRead = 0;
	FF_PARTITION	*pPart = pFile->pIoman->pPartition;
	FF_T_UINT32		fileLBA, fatEntry;
	FF_BUFFER		*pBuffer;
	FF_T_UINT32		numClusters;
	FF_T_UINT32		relClusterPos	= pFile->FilePointer % ((pPart->SectorsPerCluster * pPart->BlkFactor) * pFile->pIoman->BlkSize);
	FF_T_UINT32		bytesPerCluster = pPart->BlkSize * pPart->SectorsPerCluster;
	FF_T_UINT32		majorBlockNum	 = relClusterPos / pPart->BlkSize;
	FF_T_UINT32		relMajorBlockPos = relClusterPos % pPart->BlkSize;
	
	FF_T_UINT32		minorBlockNum	 = relMajorBlockPos / pFile->pIoman->BlkSize;
	FF_T_UINT32		relMinorBlockPos = relMajorBlockPos % pFile->pIoman->BlkSize;

	FF_T_UINT32		Sectors = Bytes / pFile->pIoman->BlkSize;

	FF_T_INT32		retVal = 0;

	if(FF_isEOF(pFile)) {
		return 0;
	}

	if(FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1) > pFile->CurrentCluster) {
		fatEntry = FF_getFatEntry(pFile->pIoman, pFile->AddrCurrentCluster);
		if(fatEntry == FF_ERR_DEVICE_DRIVER_FAILED) {
			return 0;
		}
		if(FF_isEndOfChain(pFile->pIoman, fatEntry)) {
			pFile->AddrCurrentCluster = pFile->ObjectCluster;
			// ERROR THIS SHOULD NOT OCCUR!
			return 0;
		} else {
			pFile->AddrCurrentCluster = fatEntry;
			pFile->CurrentCluster += 1;
		}
	}

	fileLBA = FF_Cluster2LBA(pFile->pIoman, pFile->AddrCurrentCluster);
	fileLBA = FF_getRealLBA(pFile->pIoman, fileLBA + FF_getMajorBlockNumber(pFile->pIoman, pFile->FilePointer, 1)) + FF_getMinorBlockNumber(pFile->pIoman, pFile->FilePointer, 1);

	
	if(Bytes > (pFile->Filesize - pFile->FilePointer)) {
		Bytes = pFile->Filesize - pFile->FilePointer;
	}

	if((Bytes + relMinorBlockPos) < pFile->pIoman->BlkSize) { // We have to memcpy from a buffer!

		pBuffer = FF_GetBuffer(pFile->pIoman, fileLBA, FF_MODE_READ);
		{
			if(!pBuffer) {
				return 0;
			}
			memcpy(buffer, (pBuffer->pBuffer + relMinorBlockPos), Bytes);
		}
		FF_ReleaseBuffer(pFile->pIoman, pBuffer);

		pFile->FilePointer += Bytes;
		return Bytes;
	} else {

		if(relMinorBlockPos > 0) { // Not on a Sector Boundary. Memcpy First Lot
		// Read Across Multiple Clusters
			pBuffer = FF_GetBuffer(pFile->pIoman, fileLBA, FF_MODE_READ);
			{
				if(!pBuffer) {
					return 0;
				}
				memcpy(buffer, (pBuffer->pBuffer + relMinorBlockPos), pFile->pIoman->BlkSize - relMinorBlockPos);
			}
			FF_ReleaseBuffer(pFile->pIoman, pBuffer);

			buffer += (pFile->pIoman->BlkSize - relMinorBlockPos);
			fileLBA += 1;	// Next Copy just be from the Next LBA.
			// CARE ABOUT CLUSTER BOUNDARIES!
			Bytes -= pFile->pIoman->BlkSize - relMinorBlockPos;
			BytesRead += pFile->pIoman->BlkSize - relMinorBlockPos;
			pFile->FilePointer += pFile->pIoman->BlkSize - relMinorBlockPos;
		}
		
		// Direct Copy :D Remaining Bytes > IOMAN BlkSIze

		while(Bytes > bytesPerCluster) {
			// Direct Copy Size Remaining Cluster's!
			numClusters		 = Bytes / bytesPerCluster;
			relClusterPos	 = pFile->FilePointer % ((pPart->SectorsPerCluster * pPart->BlkFactor) * pFile->pIoman->BlkSize);
			majorBlockNum	 = relClusterPos / pPart->BlkSize;
			relMajorBlockPos = relClusterPos % pPart->BlkSize;
			minorBlockNum	 = relMajorBlockPos / pFile->pIoman->BlkSize;
			relMinorBlockPos = relMajorBlockPos % pFile->pIoman->BlkSize;

			if(FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1) > pFile->CurrentCluster) {
				fatEntry = FF_getFatEntry(pFile->pIoman, pFile->AddrCurrentCluster);
				if(fatEntry == FF_ERR_DEVICE_DRIVER_FAILED) {
					return BytesRead;
				}
				if(FF_isEndOfChain(pFile->pIoman, fatEntry)) {
					pFile->AddrCurrentCluster = pFile->ObjectCluster;
					// ERROR THIS SHOULD NOT OCCUR!
					return BytesRead;
				} else {
					pFile->AddrCurrentCluster = fatEntry;
					pFile->CurrentCluster += 1;
				}
			}
			
			fileLBA = FF_Cluster2LBA(pFile->pIoman, pFile->AddrCurrentCluster);
			fileLBA = FF_getRealLBA(pFile->pIoman, fileLBA + FF_getMajorBlockNumber(pFile->pIoman, pFile->FilePointer, 1)) + FF_getMinorBlockNumber(pFile->pIoman, pFile->FilePointer, 1);

			Sectors = (bytesPerCluster - relClusterPos) / pFile->pIoman->BlkSize;
	
			do{
				if(pFile->pIoman->pBlkDevice->fnReadBlocks) {
					retVal = pFile->pIoman->pBlkDevice->fnReadBlocks(buffer, fileLBA, Sectors, pFile->pIoman->pBlkDevice->pParam);
				}
				if(retVal == FF_ERR_DRIVER_BUSY) {
					FF_Yield();
					FF_Sleep(FF_DRIVER_BUSY_SLEEP);
				}
			} while(retVal == FF_ERR_DRIVER_BUSY);

			if(retVal < 0) {
				return BytesRead;
			}

			buffer	+= (bytesPerCluster - relClusterPos);
			Bytes	-= (bytesPerCluster - relClusterPos);
			pFile->FilePointer += (bytesPerCluster - relClusterPos);
			BytesRead += (retVal * pFile->pIoman->BlkSize);	// Return value of the driver function is sectors written.
		}

		relClusterPos	 = pFile->FilePointer % ((pPart->SectorsPerCluster * pPart->BlkFactor) * pFile->pIoman->BlkSize);
		majorBlockNum	 = relClusterPos / pPart->BlkSize;
		relMajorBlockPos = relClusterPos % pPart->BlkSize;
		minorBlockNum	 = relMajorBlockPos / pFile->pIoman->BlkSize;
		relMinorBlockPos = relMajorBlockPos % pFile->pIoman->BlkSize;

		if(FF_getClusterChainNumber(pFile->pIoman, pFile->FilePointer, 1) > pFile->CurrentCluster) {
			fatEntry = FF_getFatEntry(pFile->pIoman, pFile->AddrCurrentCluster);
			if(fatEntry == FF_ERR_DEVICE_DRIVER_FAILED) {
				return BytesRead;
			}
			if(FF_isEndOfChain(pFile->pIoman, fatEntry)) {
				pFile->AddrCurrentCluster = pFile->ObjectCluster;
				// ERROR THIS SHOULD NOT OCCUR!
				return BytesRead;
			} else {
				pFile->AddrCurrentCluster = fatEntry;
				pFile->CurrentCluster += 1;
			}
		}
		
		fileLBA = FF_Cluster2LBA(pFile->pIoman, pFile->AddrCurrentCluster);
		fileLBA = FF_getRealLBA(pFile->pIoman, fileLBA + FF_getMajorBlockNumber(pFile->pIoman, pFile->FilePointer, 1)) + FF_getMinorBlockNumber(pFile->pIoman, pFile->FilePointer, 1);


		if(Bytes >= pFile->pIoman->BlkSize) {
			Sectors = Bytes / pFile->pIoman->BlkSize;
			 do{
				if(pFile->pIoman->pBlkDevice->fnReadBlocks) {
					retVal = pFile->pIoman->pBlkDevice->fnReadBlocks(buffer, fileLBA, Sectors, pFile->pIoman->pBlkDevice->pParam);
				}
				if(retVal == FF_ERR_DRIVER_BUSY) {
					FF_Yield();
					FF_Sleep(FF_DRIVER_BUSY_SLEEP);
				}
			} while(retVal == FF_ERR_DRIVER_BUSY);
			
			if(retVal < 0) {
				return BytesRead;
			}

			Bytes -= Sectors * pFile->pIoman->BlkSize;
			buffer += Sectors * pFile->pIoman->BlkSize;
			BytesRead += Sectors * pFile->pIoman->BlkSize;
			pFile->FilePointer += Sectors * pFile->pIoman->BlkSize;
			fileLBA += Sectors;
		}

		if(Bytes > 0) {// Memcpy the remaining Bytes
			pBuffer = FF_GetBuffer(pFile->pIoman, fileLBA, FF_MODE_READ);
			{
				if(!pBuffer) {
					return BytesRead;
				}
				memcpy(buffer, (pBuffer->pBuffer), Bytes);
			}
			FF_ReleaseBuffer(pFile->pIoman, pBuffer);
		}

		buffer += Bytes;
		BytesRead += Bytes;
		pFile->FilePointer += Bytes;	
	
	}
	return BytesRead;
}


/**
 *	@public
 *	@brief	Equivalent to fgetc()
 *
 *	@param	pFile		FF_FILE object that was created by FF_Open().
 *
 *	@return The character that was read (cast as a 32-bit interger). -1 on EOF.
 *	@return -2 If a null file pointer was provided.
 *	@return -3 Device access failed.
 *
 **/
FF_T_INT32 FF_GetC(FF_FILE *pFile) {
	FF_T_UINT32 fileLBA;
	FF_BUFFER *pBuffer;
	FF_PARTITION *pPart = pFile->pIoman->pPartition;
	FF_T_UINT8 retChar;
	FF_T_UINT32 fatEntry;

	/*
		These can be replaced with the ff_blk.c functions! 
		Will be done for the 1.0 release.
	*/
	FF_T_UINT32 clusterNum		= pFile->FilePointer / ((pPart->SectorsPerCluster * pPart->BlkFactor) * pFile->pIoman->BlkSize);
	FF_T_UINT32 relClusterPos	= pFile->FilePointer % ((pPart->SectorsPerCluster * pPart->BlkFactor) * pFile->pIoman->BlkSize);
	
	FF_T_UINT32 majorBlockNum	= relClusterPos / pPart->BlkSize;
	FF_T_UINT32 relMajorBlockPos = relClusterPos % pPart->BlkSize;
	
	FF_T_UINT32 minorBlockNum = relMajorBlockPos / pFile->pIoman->BlkSize;
	FF_T_UINT32 relMinorBlockPos = relMajorBlockPos % pFile->pIoman->BlkSize;

	if(!pFile) {
		return -2;
	}
	
	if(pFile->FilePointer >= pFile->Filesize) {
		return -1; // EOF!	
	}
	
	if(clusterNum > pFile->CurrentCluster) {
		fatEntry = FF_getFatEntry(pFile->pIoman, pFile->AddrCurrentCluster);
		if(fatEntry == FF_ERR_DEVICE_DRIVER_FAILED) {
			return -3;
		}
		if(FF_isEndOfChain(pFile->pIoman, fatEntry)) {
			pFile->AddrCurrentCluster = pFile->ObjectCluster;
			// ERROR THIS SHOULD NOT OCCUR!
			return -2;
		} else {
			pFile->AddrCurrentCluster = fatEntry;
			pFile->CurrentCluster += 1;
		}
	}

	fileLBA = FF_Cluster2LBA(pFile->pIoman, pFile->AddrCurrentCluster);

	fileLBA = FF_getRealLBA(pFile->pIoman, fileLBA + majorBlockNum) + minorBlockNum;

	pBuffer = FF_GetBuffer(pFile->pIoman, fileLBA, FF_MODE_READ);
	{
		if(!pBuffer) {
			return -3;
		}
		retChar = pBuffer->pBuffer[relMinorBlockPos];
	}
	FF_ReleaseBuffer(pFile->pIoman, pBuffer);

	pFile->FilePointer += 1;

	return (FF_T_INT32) retChar;
}





/**
 *	@public
 *	@brief	Equivalent to fseek()
 *
 *	@param	pFile		FF_FILE object that was created by FF_Open().
 *	@param	Offset		An integer (+/-) to seek to, from the specified origin.
 *	@param	Origin		Where to seek from. (FF_SEEK_SET seek from start, FF_SEEK_CUR seek from current position, or FF_SEEK_END seek from end of file).
 *
 *	@return 0 on Sucess, 
 *	@return -2 if offset results in an invalid position in the file. 
 *	@return -1 if a FF_FILE pointer was not recieved.
 *	@return -3 if an invalid origin was provided.
 *	
 **/
FF_T_SINT8 FF_Seek(FF_FILE *pFile, FF_T_SINT32 Offset, FF_T_INT8 Origin) {
	if(!pFile) {
		return -1;
	}

	switch(Origin) {
		case FF_SEEK_SET:
			if(Offset <= pFile->Filesize && Offset >= 0) {
				pFile->FilePointer = Offset;
			} else {
				return -2;
			}
			break;

		case FF_SEEK_CUR:
			if((Offset + pFile->FilePointer) <= pFile->Filesize && (Offset + (FF_T_SINT32) pFile->FilePointer) >= 0) {
				pFile->FilePointer = Offset + pFile->FilePointer;
			} else {
				return -2;
			}
			break;
	
		case FF_SEEK_END:
			if((Offset + (FF_T_SINT32) pFile->Filesize) >= 0 && (Offset + pFile->Filesize) <= pFile->Filesize) {
				pFile->FilePointer = Offset + pFile->Filesize;
			} else {
				return -2;
			}
			break;

		default:
			return -3;
		
	}

	return 0;
}


/**
 *	@public
 *	@brief	Equivalent to fclose()
 *
 *	@param	pFile		FF_FILE object that was created by FF_Open().
 *
 *	@return 0 on sucess.
 *	@return -1 if a null pointer was provided.
 *
 **/
FF_T_SINT8 FF_Close(FF_FILE *pFile) {

	if(!pFile) {
		return -1;
	}
	// If file written, flush to disk
	free(pFile);
	// Simply free the pointer!
	return 0;
}
