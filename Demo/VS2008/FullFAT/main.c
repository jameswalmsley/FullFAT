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

/*
	NOTE THIS DEMO IS HIGHLY SUBJECT TO CHANGE:
	AND MOST UNIT TESTS HAVE NOT BEEN INCLUDED AT THIS POINT:

	SIMPLY USE A BASIC DEMO
*/

#include <stdio.h>
#include <string.h>
#include "../../../src/ff_ioman.h"
#include "../../../src/ff_fat.h"

void test(char *buffer, unsigned long sector, unsigned short sectors, void *pParam);
void test_ipod(char *buffer, unsigned long sector, unsigned short sectors, void *pParam);

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

	printf("%s %12lu %s\n", attr, pDirent->Filesize, pDirent->FileName);
}

int main(void) {
	/*printf("FullFAT by James Walmsley - Test Suite\n");
	FF_ioman_test();*/
	FILE *f, *jim;
	FF_FILE *MyFile;
	FF_IOMAN *pIoman = FF_CreateIOMAN(NULL, 4096);
	char buffer[4096];
	char cmd[10];
	char arg[512];
	char tester;
	unsigned long BytesRead;
	FF_T_UINT32 i;
	char string[] = "\\\\.\\PHYSICALDRIVE1";
	FF_DIRENT mydir;
	f = fopen("\\\\.\\PHYSICALDRIVE2", "rb");

	
	if(f) {
		FF_RegisterBlkDevice(pIoman, (FF_WRITE_BLOCKS) test_ipod, (FF_READ_BLOCKS) test, f);
		FF_MountPartition(pIoman);
		/*jim = fopen("c:\\talktest.mp3", "wb");

		MyFile = FF_Open(pIoman, "\\", "TALK", FF_MODE_READ);

		if(MyFile) {
			fputc(FF_GetC(MyFile), jim);
			 do{
				BytesRead = FF_Read(MyFile, 4096, 1, buffer);
				if(BytesRead == 0)
					break;
				fwrite(buffer, BytesRead, 1, jim);
			}while(BytesRead > 0);
			fclose(jim);
			FF_Close(MyFile);
		}

		MyFile = FF_Open(pIoman, "\\", "1FILE", FF_MODE_READ);
		if(MyFile) {
			for(i = 0; i < MyFile->Filesize; i++) {
				printf("%c", FF_GetC(MyFile));
			}
			FF_Close(MyFile);
		}*/

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
		
	} else {
		
		printf("Couldn't Open Block Device\n");
		printf("Run Visual Studio as an Administrator!\n");
		getchar();
	}

}