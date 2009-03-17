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
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winbase.h>
#include <conio.h>
#include "../../../src/fullfat.h"

#define PARTITION_NUMBER	0		///< Change this to the primary partition to be mounted (0 to 3)
#define COPY_BUFFER_SIZE	8192	// Increase This for Faster File Copies

void fnRead_512		(char *buffer, unsigned long sector, unsigned short sectors, void *pParam);
void fnWrite_512	(char *buffer, unsigned long sector, unsigned short sectors, void *pParam);
signed int fnNewRead_512(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam);
signed int fnNewWrite_512(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam);
void test_2048		(char *buffer, unsigned long sector, unsigned short sectors, void *pParam);

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
	LARGE_INTEGER ticksPerSecond;
	LARGE_INTEGER start_ticks, end_ticks, cputime; 
	FILE *f,*fDest;
	int f1;
	FF_FILE *fSource, *ff1, *ff2, *ff3, *ff4;
	FF_IOMAN *pIoman = FF_CreateIOMAN(NULL, 8192, 512);
	char buffer[COPY_BUFFER_SIZE];
	char commandLine[1024];
	char commandShadow[2600];
	char source[260], destination[260];
	char workingDir[2600] = "\\";
	char tester;
	unsigned long BytesRead;
	FF_T_UINT32 i;
	FF_DIRENT mydir;
	FF_BUFFER *mybuffer;
	FF_T_SINT8 Error;
	float time, transferRate;
	//f = fopen("c:\\ramdisk.dat", "ab+");
	f = fopen("\\\\.\\PHYSICALDRIVE1", "rb");
	//f = fopen("c:\\ramdisk.dat", "rb");
	//f1 = open("\\\\.\\PHYSICALDRIVE1",  O_RDWR | O_BINARY);
	//f1 = open("c:\\ramdisk.dat",  O_RDWR | O_BINARY);
	QueryPerformanceFrequency(&ticksPerSecond);

	printf("FullFAT by James Walmsley - Windows Demonstration\n");
	printf("Use the command help for more information\n\n");

	if(f) {
		FF_RegisterBlkDevice(pIoman, 512, (FF_WRITE_BLOCKS) fnWrite_512, (FF_READ_BLOCKS) fnRead_512, f);
		if(FF_MountPartition(pIoman, PARTITION_NUMBER)) {
			fclose(f);
			printf("FullFAT Couldn't mount the specified parition!\n");
			getchar();
			return -1;
		}

		ff1 = FF_Open(pIoman, "\\talk.mp3", FF_MODE_READ, &Error);
		ff2 = FF_Open(pIoman, "\\1FILE", FF_MODE_READ, &Error);
		ff3 = FF_Open(pIoman, "\\HELLO.txt", FF_MODE_READ, &Error);
		ff4 = FF_Open(pIoman, "\\3.txt", FF_MODE_READ, &Error);
		FF_Close(ff1);

		while(1) {
			printf("FullFAT:%s>",workingDir);
			for(i = 0; i < 1024; i++) {
				commandLine[i] = (char) _getch();
				_putch(commandLine[i]);
				if(commandLine[i] == '\r') {
					_putch('\n');
					commandLine[i] = '\0';
					break;
				}
			}
			if(strstr(commandLine, "cd")) {
				
				if(commandLine[3] != '\\' && commandLine[3] != '/') {
					if(strlen(workingDir) == 1) {
						sprintf(commandShadow, "\\%s", (commandLine + 3));
					} else {
						sprintf(commandShadow, "%s\\%s", workingDir, (commandLine + 3));
					}
				} else {
					sprintf(commandShadow, "%s", (commandLine + 3));
				}

				if(FF_FindDir(pIoman, commandShadow, strlen(commandShadow))) {
					sprintf(workingDir, "%s", commandShadow);
				} else {
					printf("Path %s Not Found\n", commandShadow);
				}
			}

			if(strstr(commandLine, "ls") || strstr(commandLine, "dir")) {
				i = 0;
				tester = 0;
				tester = FF_FindFirst(pIoman, &mydir, workingDir);
				while(tester == 0) {
					FF_PrintDir(&mydir);
					i++;
					tester = FF_FindNext(pIoman, &mydir);
				}
				printf("\n%d Items\n", i);
				putchar('\n');
			}

			if(strstr(commandLine, "view")) {
				if(strlen(workingDir) == 1) {
					sprintf(buffer, "\\%s", (commandLine+5)); 
				} else {
					sprintf(buffer, "%s\\%s", workingDir, (commandLine+5));
				}
				
				fSource = FF_Open(pIoman, buffer, FF_MODE_READ, &Error);
				if(fSource) {
					for(i = 0; i < fSource->Filesize; i++) {
						printf("%c", FF_GetC(fSource));
					}

					FF_Close(fSource);
				}else {
					if(Error == FF_ERR_FILE_NOT_FOUND) {
						printf("File Not Found!\n");
					} else if (Error == FF_ERR_FILE_ALREADY_OPEN) {
						printf("File In Use!\n");
					} else {
						printf("Couldn't Open file! Unknown Error \n");
					}
				}
			}

			if(strstr(commandLine, "pwd")) {
				printf("%s\n", workingDir);
			}

			if(strstr(commandLine, "help")) {
				printf("The following commands are available:\n\n");
				printf("pwd \t\t- Print the working directory\n");
				printf("ls or dir \t- List the contents of the working directory\n");
				printf("cd \t\t- Change working directory e.g. cd path_to_dir\n");
				printf("cp \t\t- Copy a file to the hard disk.\n");
				printf("\t\t  e.g. cp filename c:\\filename\n");
				printf("exit \t\t- Quits the FullFAT test suite.\n");
				printf("\nFullFAT is developed and maintained by James Walmsley\n");
				printf("\nVisit www.worm.me.uk/fullfat for more information, and contact details\n\n");
			}

			if(strstr(commandLine, "info")) {
				switch(pIoman->pPartition->Type) {
					case FF_T_FAT32:
						printf("FAT32 Formatted Drive\n"); break;
					case FF_T_FAT16:
						printf("FAT16 Formatted Drive\n"); break;
					case FF_T_FAT12:
						printf("FAT12 Formatted Drive\n"); break;
				}

				printf("Block Size: %d\n", pIoman->pPartition->BlkSize);
				printf("Cluster Size: %dKb\n", (pIoman->pPartition->BlkSize * pIoman->pPartition->SectorsPerCluster) / 1024);
#ifdef FF_64_NUM_SUPPORT
				printf("Volume Size: %llu (%d MB)\n", FF_GetVolumeSize(pIoman), (unsigned int) (FF_GetVolumeSize(pIoman) / 1048576));
#else
				printf("Volume Size: %d (%d MB)\n", FF_GetVolumeSize(pIoman), (unsigned int) (FF_GetVolumeSize(pIoman) / 1048576));
#endif
			}

			if(strstr(commandLine, "cp")) {
				sscanf((commandLine + 3), "%s %s", source, destination);
				
				if(strlen(workingDir) == 1) {
					sprintf(buffer, "\\%s", (source)); 
				} else {
					sprintf(buffer, "%s\\%s", workingDir, (source));
				}
				
				fDest = fopen(destination, "wb");
				if(fDest) {
					fSource = FF_Open(pIoman, buffer, FF_MODE_READ, &Error);
					if(fSource) {
						QueryPerformanceCounter(&start_ticks);  
						do{
							BytesRead = FF_Read(fSource, COPY_BUFFER_SIZE, 1, (FF_T_UINT8 *)buffer);
							fwrite(buffer, BytesRead, 1, fDest);
							QueryPerformanceCounter(&end_ticks); 
							cputime.QuadPart = end_ticks.QuadPart - start_ticks.QuadPart;
							time = ((float)cputime.QuadPart/(float)ticksPerSecond.QuadPart);
							transferRate = (fSource->FilePointer / time) / 1024;
							printf("%3.0f%% - %10d Bytes Copied, %7.2f Kb/S\r", ((float)((float)fSource->FilePointer/(float)fSource->Filesize) * 100), fSource->FilePointer, transferRate);
						}while(BytesRead > 0);
						printf("%3.0f%% - %10d Bytes Copied, %7.2f Kb/S\n", ((float)((float)fSource->FilePointer/(float)fSource->Filesize) * 100), fSource->FilePointer, transferRate);
						fclose(fDest);
						FF_Close(fSource);
					} else {
						fclose(fDest);
						if(Error == FF_ERR_FILE_NOT_FOUND) {
							printf("File Not Found!\n");
						} else if (Error == FF_ERR_FILE_ALREADY_OPEN) {
							printf("File In Use!\n");
						} else {
							printf("Couldn't Open file! Unknown Error \n");
						}
					}
				} else {
					printf("Error Opening Destination\n");
				}
				strcpy(source, "");
				strcpy(destination, "");
			}

			if(strstr(commandLine, "exit") || strstr(commandLine, "quit")) {
				fclose(f);
				return 0;
			}
		}
		
	} else {
		
		printf("Couldn't Open Block Device\n");
		printf("Run Visual Studio as an Administrator!\n");
		getchar();
	}

}