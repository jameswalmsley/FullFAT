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
 *	@file		ff_format.c
 *	@author		James Walmsley
 *	@ingroup	FORMAT
 *
 *	@defgroup	FORMAT Independent FAT Formatter
 *	@brief		Provides an interface to format a partition with FAT.
 *
 *	
 *	
 **/


#include "ff_format.h"
#include "ff_types.h"
#include "ff_ioman.h"
#include "ff_fatdef.h"

static FF_T_SINT8 FF_PartitionCount (FF_T_UINT8 *pBuffer)
{
	FF_T_SINT8 count = 0;
	FF_T_SINT8 part;
	// Check PBR or MBR signature
	if (FF_getChar(pBuffer, FF_FAT_MBR_SIGNATURE) != 0x55 &&
		FF_getChar(pBuffer, FF_FAT_MBR_SIGNATURE) != 0xAA ) {
		// No MBR, but is it a PBR ?
		if (FF_getChar(pBuffer, 0) == 0xEB &&          // PBR Byte 0
		    FF_getChar(pBuffer, 2) == 0x90 &&          // PBR Byte 2
		    (FF_getChar(pBuffer, 21) & 0xF0) == 0xF0) {// PBR Byte 21 : Media byte
			return 1;	// No MBR but PBR exist then only one partition
		}
		return 0;   // No MBR and no PBR then no partition found
	}
	for (part = 0; part < 4; part++)  {
		FF_T_UINT8 active = FF_getChar(pBuffer, FF_FAT_PTBL + FF_FAT_PTBL_ACTIVE + (16 * part));
		FF_T_UINT8 part_id = FF_getChar(pBuffer, FF_FAT_PTBL + FF_FAT_PTBL_ID + (16 * part));
		// The first sector must be a MBR, then check the partition entry in the MBR
		if (active != 0x80 && (active != 0 || part_id == 0)) {
			break;
		}
		count++;
	}
	return count;
}


FF_ERROR FF_CreatePartitionTable(FF_IOMAN *pIoman, FF_T_UINT32 ulTotalDeviceBlocks, FF_PARTITION_TABLE *pPTable) {
	FF_BUFFER *pBuffer = FF_GetBuffer(pIoman, 0, FF_MODE_WRITE);
	{
		if(!pBuffer) {
			return FF_ERR_DEVICE_DRIVER_FAILED;
		}


	}
	FF_ReleaseBuffer(pIoman, pBuffer);

	return FF_ERR_NONE;
}


FF_ERROR FF_FormatPartition(FF_IOMAN *pIoman, FF_T_UINT32 ulPartitionNumber, FF_T_UINT32 ulClusterSize) {
	
	FF_BUFFER *pBuffer;
	FF_T_UINT8	ucPartitionType;
	FF_T_SINT8	scPartitionCount;
	FF_T_UINT32 maxClusters, f16MaxClusters, f32MaxClusters;

	FF_PARTITION_ENTRY partitionGeom;

	FF_T_UINT32 fat32Size, fat16Size, newFat32Size, newFat16Size;
	FF_T_UINT32 sectorsPerCluster = ulClusterSize / pIoman->BlkSize;

	FF_T_UINT32 ul32DataSectors, ul16DataSectors;
	FF_T_UINT32 i;

	FF_T_UINT32 ulClusterBeginLBA;

	FF_ERROR	Error = FF_ERR_NONE;

	// Get Partition Metrics, and pass on to FF_Format() function

	pBuffer = FF_GetBuffer(pIoman, 0, FF_MODE_READ);
	{
		if(!pBuffer) {
			return FF_ERR_DEVICE_DRIVER_FAILED | FF_FORMATPARTITION;
		}

		printf("Sig: %x\n", (unsigned long) (*((unsigned short *) (pBuffer->pBuffer + 510)) ));

		scPartitionCount = FF_PartitionCount(pBuffer->pBuffer);
		printf("Found %d partitions\n", scPartitionCount);

		if(!scPartitionCount) {
			// Get Partition Geom from volume boot record.
			partitionGeom.ulStartLBA = FF_getShort(pBuffer->pBuffer, FF_FAT_RESERVED_SECTORS); // Get offset to start of where we can actually put the FAT table.
			partitionGeom.ulLength = (FF_T_UINT32) FF_getShort(pBuffer->pBuffer, FF_FAT_16_TOTAL_SECTORS);

			if(partitionGeom.ulLength == 0) { // 32-bit entry was used.
				partitionGeom.ulLength = FF_getLong(pBuffer->pBuffer, FF_FAT_32_TOTAL_SECTORS);
			}

			partitionGeom.ulLength -= partitionGeom.ulStartLBA; // Remove the reserved sectors from the count.

			printf("Partition-format-start: %lu\n", partitionGeom.ulStartLBA);

		} else {
			// Get partition Geom from the partition table entry.
			
		}

		printf("pLEN: %lu, clusterSize %lu\n", partitionGeom.ulLength, ulClusterSize);

		// Calculate the max possiblenumber of clusters based on clustersize.
		maxClusters = partitionGeom.ulLength / sectorsPerCluster;

		printf("maxClusters: %lu\n", maxClusters);

		// Determine the size of a FAT table required to support this.
		fat32Size = (maxClusters * 32) / pIoman->BlkSize; // Potential size in sectors of a fat32 table.
		if((maxClusters *32) % pIoman->BlkSize) {
			fat32Size++;
		}
		fat32Size *= 2;	// Officially there are 2 copies of the FAT.

		fat16Size = (maxClusters * 16) / pIoman->BlkSize; // Potential size in bytes of a fat16 table.
		if((maxClusters * 32) % pIoman->BlkSize) {
			fat16Size++;
		}
		fat16Size *= 2;

		printf("fat32size: %lu, fat16size: %lu\n", fat32Size, fat16Size);

		// A real number of sectors to be available is therefore ~~
		ul16DataSectors = partitionGeom.ulLength - fat16Size;
		ul32DataSectors = partitionGeom.ulLength - fat32Size;

		printf("fat16-sects: %lu, fat32-sects: %lu\n", ul16DataSectors, ul32DataSectors);
		f16MaxClusters = ul16DataSectors / sectorsPerCluster;
		f32MaxClusters = ul32DataSectors / sectorsPerCluster;

		printf("f16-clusts: %lu, f32-clusts: %lu\n", f16MaxClusters, f32MaxClusters);
		newFat16Size = (f16MaxClusters * 16) / pIoman->BlkSize;
		if((f16MaxClusters * 16) % pIoman->BlkSize) {
			newFat16Size++;
		}

		newFat32Size = (f32MaxClusters * 32) / pIoman->BlkSize;
		if((f32MaxClusters * 32) % pIoman->BlkSize) {
			newFat32Size++;
		}

		// Now determine if this should be fat16/32 format?

		if(f16MaxClusters < 65525) {
			// Go-ahead with FAT16 format.
			FF_ReleaseBuffer(pIoman, pBuffer);
			for(i = 0; i < newFat16Size*2; i++) { // Ensure these values are clear!
				if(i == 0) {
					pBuffer = FF_GetBuffer(pIoman, partitionGeom.ulStartLBA, FF_MODE_WR_ONLY);
					if(!pBuffer) {
						return FF_ERR_DEVICE_DRIVER_FAILED;
					}

					memset(pBuffer->pBuffer, 0, pIoman->BlkSize);
				} else {
					FF_BlockWrite(pIoman, partitionGeom.ulStartLBA+i, 1, pBuffer->pBuffer, FF_FALSE);
				}
			}

			FF_putShort(pBuffer->pBuffer, 0, 0xFFF8); // First FAT entry.
			FF_putShort(pBuffer->pBuffer, 2, 0xFFFF); // Allocation of root dir.
			FF_ReleaseBuffer(pIoman, pBuffer);
				
			// Clear and initialise the root dir.
			ulClusterBeginLBA = partitionGeom.ulStartLBA + (newFat16Size*2);
				
			for(i = 0; i < sectorsPerCluster; i++) {
				if(i == 0) {
					pBuffer = FF_GetBuffer(pIoman, ulClusterBeginLBA, FF_MODE_WR_ONLY);
					memset(pBuffer->pBuffer, 0, pIoman->BlkSize);
				} else  {
					FF_BlockWrite(pIoman, ulClusterBeginLBA+i, 1, pBuffer->pBuffer, FF_FALSE);
				}
					
			}

			FF_ReleaseBuffer(pIoman, pBuffer);
				
			// Correctly modify the second FAT item again.


			// Modify the fields in the VBR/PBR for correct mounting.
			pBuffer = FF_GetBuffer(pIoman, 0, FF_MODE_WRITE); // Modify the FAT descriptions.
			{
				//printf("modify the br!\n");
				FF_putShort(pBuffer->pBuffer, FF_FAT_RESERVED_SECTORS, partitionGeom.ulStartLBA);
				FF_putShort(pBuffer->pBuffer, FF_FAT_NUMBER_OF_FATS, 2);
				FF_putShort(pBuffer->pBuffer, FF_FAT_16_SECTORS_PER_FAT, newFat16Size);
				FF_putChar(pBuffer->pBuffer, FF_FAT_SECTORS_PER_CLUS, sectorsPerCluster);
				FF_putLong(pBuffer->pBuffer, FF_FAT_ROOT_DIR_CLUSTER, 1);
						

			}
			FF_ReleaseBuffer(pIoman, pBuffer);
				

			FF_FlushCache(pIoman);
				
				// Done :D (Wasn't so hard!).


		} else {
			// Go-ahead with FAT32 format.

		}

		// FAT16 format:

		// Write out the initial FAT table.


	}
	//FF_ReleaseBuffer(pIoman, pBuffer);

	return Error;
	
}

FF_ERROR FF_Format(FF_IOMAN *pIoman, FF_T_UINT32 ulStartLBA, FF_T_UINT32 ulEndLBA, FF_T_UINT32 ulClusterSize) {
	 //FF_T_UINT32 ulTotalSectors;
	//FF_T_UINT32 ulTotalClusters;

	//ulTotalSectors	= ulEndLBA - ulStartLBA;
	//ulTotalClusters = ulTotalSectors / (ulClusterSize / pIoman->BlkSize);


	return -1;


}
