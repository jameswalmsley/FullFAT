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
 *	This file calls unit-tests for all modules, and displays the results.
 *	On sucessful completion it will enter a Demonstration Terminal allowing
 *	you to ls through a file-system on a connected device.
 *
 *	You should ensure all Unit-tests are passed on your platform.
 **/

#include <stdio.h>
#include <string.h>
#include "ff_ioman.h"
#include "ff_fat.h"

// FF_IOMAN UNIT TESTS
// iomanDestroy doesn't accept a NULL pointer!
char TEST_DestroyIOMAN_1() {
	char i = 0;
	i = FF_DestroyIOMAN(NULL);
	if(i != FF_ERR_IOMAN_NULL_POINTER) {
		return 0;
	}

	return 1;
}

// iomanCreate test only accepts multiples of 512 memory. 0 is invalid, and nothing else.
char TEST_CreateIOMAN_1() {
	int i;
	FF_IOMAN *test = 0;
	//  Test 0 unaccepted!
	test = FF_CreateIOMAN(NULL, 0);
	if(test) {
			FF_DestroyIOMAN(test);
			return 0;
		}
	FF_DestroyIOMAN(test);
	// Test all non multiples up to 1 megabyte
	for(i = 1; i < 1024768; i++) {
		if((i % 512) == 0)
			i++;
		test = FF_CreateIOMAN(NULL, i);
		if(test) {
			FF_DestroyIOMAN(test);
			return 0;
		}
		FF_DestroyIOMAN(test);
		// Destroy test on each loop to prevent over allocation of mem! 
	}

	// Test all multples up to 1 megabyte
	for(i = 1; i < 1024768; i++) {
		if((i % 512) == 0) {
			test = FF_CreateIOMAN(NULL, i);
			if(!test) {
				FF_DestroyIOMAN(test);
				return 0;
			}
			FF_DestroyIOMAN(test);
		}
	}

	return 1;
}

char TEST_CreateIOMAN_2() {
	FF_IOMAN *test = NULL;
	test = FF_CreateIOMAN(NULL, 512);
	if(test->pBlkDevice->fnReadBlocks) {
		FF_DestroyIOMAN(test);
		return 0;
	}
	
	if(test->pBlkDevice->fnWriteBlocks) {
		FF_DestroyIOMAN(test);
		return 0;
	}

	// Test pParam because we don't know what it is.
	// It's unlikely but it could be an function pointer.
	if(test->pBlkDevice->pParam) {
		FF_DestroyIOMAN(test);
		return 0;
	}

	return 1;
}

void FF_ioman_test(void) {

	// CreateIOMAN
	printf("IOMAN - CreateIOMAN() only accepts sizes of 512 Multiple...");
	if(TEST_CreateIOMAN_1())
		printf("Passed\n");
	else
		printf("Failed\n");
	printf("IOMAN - CreateIOMAN() provides no executable pointers...");
	if(TEST_CreateIOMAN_2())
		printf("Passed\n");
	else
		printf("Failed\n");

	// DestroyIOMAN
	printf("IOMAN - DestroyIOMAN() returns -1 if NULL pointer passed...");
	if(TEST_DestroyIOMAN_1())
		printf("Passed\n");
	else
		printf("Failed\n");
}

void test(char *buffer, unsigned long sector, unsigned short sectors, void *pParam);

void FF_PrintDir(FF_DIRENT *pDirent) {
	unsigned char attr[5] = { '-','-','-','-', '\0' };
	if(pDirent->Attrib & FF_FAT_ATTR_READONLY)
			attr[0] = 'R';
	if(pDirent->Attrib & FF_FAT_ATTR_HIDDEN)
			attr[1] = 'H';
	if(pDirent->Attrib & FF_FAT_ATTR_SYSTEM)
			attr[2] = 'S';
	if(pDirent->Attrib & FF_FAT_ATTR_DIR)
			attr[3] = 'D';

	printf("%s %12lu %s BeginCluster: %lu\n", attr, pDirent->Filesize, pDirent->ShortNAME, pDirent->ObjectCluster);
}

int main(void) {
	/*printf("FullFAT by James Walmsley - Test Suite\n");
	FF_ioman_test();*/
	FILE *f, *jim;
	FF_FILE *MyFile;
	FF_IOMAN *pIoman = FF_CreateIOMAN(NULL, 4096);
	FF_T_UINT32 clus,sect;
	char buffer[4096];
	char cmd[10];
	char arg[512];
	char tester;
	unsigned long BytesRead;
	int i;

	FF_DIRENT mydir;
	f = fopen("\\\\.\\PHYSICALDRIVE1", "rb");
	//char string[] = "\\\\.\\PHYSICALDRIVE1";
	
	FF_RegisterBlkDevice(pIoman, (FF_WRITE_BLOCKS) test, (FF_READ_BLOCKS) test, f);


	
	FF_MountPartition(pIoman);
	//test(buffer, 2065, 1, string);

	jim = fopen("c:\\talktest.mp3", "wb");
	MyFile = FF_Open(pIoman, "\\", "TALK", FF_MODE_READ);
	fputc(FF_GetC(MyFile), jim);
	 do{
		BytesRead = FF_Read(MyFile, 4096, 1, buffer);
		if(BytesRead == 0)
			break;
		fwrite(buffer, BytesRead, 1, jim);
	}while(BytesRead > 0);
	fclose(jim);
	
	/*for(i = 0; i < MyFile->Filesize; i++) {
		fputc(FF_GetC(MyFile), jim);
	}*/

	

	FF_Close(MyFile);

	MyFile = FF_Open(pIoman, "\\", "1FILE", FF_MODE_READ);

	for(i = 0; i < MyFile->Filesize; i++) {
		printf("%c", FF_GetC(MyFile));
	}

	FF_Close(MyFile);

	while(1) {
		scanf("%s %s", cmd, arg);
		if(strstr(cmd, "cd")) {
			i = 0;
			tester = 0;
			tester = FF_FindFirst(pIoman, &mydir, arg);
			while(tester == 0) {
				FF_PrintDir(&mydir);
				i++;
				tester = FF_FindNext(pIoman, &mydir);
			}
			printf("\n%d Items\n", i);
		}

		if(strstr(cmd, "view")) {
			MyFile = FF_Open(pIoman, "\\", arg, FF_MODE_READ);
			if(MyFile) {
				for(i = 0; i < MyFile->Filesize; i++) {
					printf("%c", FF_GetC(MyFile));
				}

				FF_Close(MyFile);
			}else {
				printf("File Not Found!\n");
			}
		}
		
	}
}