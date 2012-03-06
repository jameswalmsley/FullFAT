/**
 *	FSINFO Command for FullFAT.
 *
 **/

#include "fsinfo_cmd.h"

int fsinfo_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	FF_IOMAN 		*pIoman = pEnv->pIoman;
	FF_PARTITION	*pPart	= pIoman->pPartition;
	char *fatType;

	switch(pIoman->pPartition->Type) {
	case FF_T_FAT32:{
		fatType = "FAT32";
		break;
	}

	case FF_T_FAT16:{
		fatType = "FAT16";
		break;
	}

	case FF_T_FAT12: {
		fatType = "FAT12";
		break;
	}
	}

	printf("Media Format            : %s\n", fatType);
	printf("FullFAT Driver Blocksize: %d\n", pIoman->BlkSize);
	printf("Partition Blocksize     : %d\n", pPart->BlkSize);
	printf("Total Clusters			: %lu\n", pPart->NumClusters);
	printf("Cluster Size            : %fKb\n", (float)(pPart->BlkSize * pPart->SectorsPerCluster) / 1024.0);
#ifdef FF_64_NUM_SUPPORT
	printf("Volume Size             : %llu (%d MB)\n", FF_GetVolumeSize(pIoman), (unsigned int) (FF_GetVolumeSize(pIoman) / 1048576));
	printf("Volume Free             : %llu (%d MB)\n", FF_GetFreeSize(pIoman, NULL), (unsigned int) (FF_GetFreeSize(pIoman, NULL) / 1048576));
#else
	printf("Volume Size             : %lu (%d MB)\n", FF_GetVolumeSize(pIoman), (unsigned int) (FF_GetVolumeSize(pIoman) / 1048576));
	printf("Volume Free             : %lu (%d MB)\n", FF_GetFreeSize(pIoman, NULL), (unsigned int) (FF_GetFreeSize(pIoman, NULL) / 1048576));
#endif

	return 0;
}

const FFT_ERR_TABLE fsinfoInfo[] =
{
	{"Unknown or Generic Error",		-1},							// Generic Error (always the first entry).
	{"Information about the mounted file-system.",						FFT_COMMAND_DESCRIPTION},
	{ NULL }
};

